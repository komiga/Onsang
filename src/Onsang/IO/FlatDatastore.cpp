
#include <Onsang/utility.hpp>
#include <Onsang/String.hpp>
#include <Onsang/serialization.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/IO/FlatDatastore.hpp>

#include <Hord/IO/StorageInfo.hpp>
#include <Hord/IO/PropStream.hpp>
#include <Hord/Object/Ops.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
#pragma GCC diagnostic pop

#include <cstdio>
#include <type_traits>
#include <utility>
#include <new>
#include <exception>

#include <Onsang/detail/Hord/gr_ceformat.hpp>

namespace Onsang {
namespace IO {

// class FlatDatastore implementation

#define HORD_SCOPE_CLASS IO::FlatDatastore

namespace {

HORD_DEF_FMT(
	s_err_object_not_found,
	"%s: object %s does not exist"
);

/*static constexpr ceformat::Format const
s_fmt_object_id{
	ONSANG_STR_LIT("%08x")
};*/

static char const* const
s_prop_type_abbr_rel[]{
	ONSANG_STR_LIT("/i"),
	ONSANG_STR_LIT("/m"),
	ONSANG_STR_LIT("/s"),
	ONSANG_STR_LIT("/p"),
	ONSANG_STR_LIT("/a"),
};
static_assert(
	static_cast<std::size_t>(Hord::IO::PropType::LAST)
	== std::extent<decltype(s_prop_type_abbr_rel)>::value,
	"PropType abbreviation list is incomplete"
);

}; // anonymous namespace

inline static constexpr bool
prop_info_equal(
	Hord::IO::PropInfo const& x,
	Hord::IO::PropInfo const& y
) noexcept {
	return
		x.object_id == y.object_id &&
		x.prop_type == y.prop_type
	;
}

Hord::IO::Datastore*
FlatDatastore::construct(
	Hord::String root_path
) noexcept {
	return new(std::nothrow) IO::FlatDatastore(std::move(root_path));
}

IO::FlatDatastore::base::type_info const
FlatDatastore::s_type_info{
	IO::FlatDatastore::construct
};

FlatDatastore::FlatDatastore(
	Hord::String root_path
)
	: base(
		IO::FlatDatastore::s_type_info,
		std::move(root_path)
	)
	, m_lock()
	, m_prop()
{}

void
FlatDatastore::read_index(
	std::istream& stream
) {
	auto ser = make_input_serializer(stream);
	auto& si_map = get_storage_info();
	si_map.clear();

	std::uint32_t size = 0u;
	ser(size);

	Hord::IO::StorageInfo sinfo{
		Hord::Object::ID_NULL,
		Hord::Object::TYPE_NULL,
		{true, true},
		Hord::IO::Linkage::resident
	};
	while (size--) {
		ser(sinfo);
		si_map.emplace(sinfo.object_id, sinfo);
	}
}

void
FlatDatastore::write_index(
	std::ostream& stream
) {
	auto ser = make_output_serializer(stream);
	auto& si_map = get_storage_info();
	ser(static_cast<uint32_t>(si_map.size()));
	for (auto const& si_pair : si_map) {
		ser(si_pair.second);
	}
}

static_assert(
	4u == sizeof(Hord::Object::IDValue),
	"Object::IDValue is not 4 bytes, which"
	" FlatDatastore implementation assumes"
);

void
FlatDatastore::assign_prop(
	Hord::IO::PropInfo const& prop_info,
	Hord::IO::StorageInfo const& storage_info,
	bool const is_input
) {
	bool const is_orphan = Hord::IO::Linkage::orphan == storage_info.linkage;
	m_prop.info = prop_info;
	m_prop.is_input = is_input;
	m_prop.path.reserve(
		get_root_path().size() +	// root
		(is_orphan ? 8u : 10u) +	// "/orphan/" or "/resident/"
		8u +						// ID
		2u							// "/" + prop abbr
	);

	// Taking runtime parsing cost over dynamic memory allocation
	// (which would be ceformat which could do this more simply with a
	// memstream which would be terrible... but actually safer... but
	// also evil... I should do it <- ceformat TODO)

	enum : unsigned {
		obj_id_str_len = 9
	};
	char obj_id_str[obj_id_str_len]; // uses 8 bytes (+1 for NUL)
	std::snprintf(
		obj_id_str,
		static_cast<signed>(obj_id_str_len),
		"%08x",
		prop_info.object_id.value()
	);
	m_prop.path
		.assign(get_root_path())
		.append(
			(is_orphan)
			? "/orphan/"
			: "/resident/"
		)
		.append(obj_id_str, obj_id_str_len - 1u)
		.append(s_prop_type_abbr_rel[
			enum_cast(prop_info.prop_type)
		])
	;
}

#define HORD_SCOPE_FUNC acquire_stream
namespace {
HORD_DEF_FMT_FQN(
	s_err_acquire_prop_unsupplied,
	"prop %s -> %s is not supplied for type %s"
);
HORD_DEF_FMT_FQN(
	s_err_acquire_prop_void,
	"prop %s -> %s is void"
);
HORD_DEF_FMT_FQN(
	s_err_acquire_prop_open_failed,
	"prop %s -> %s is void (or open otherwise failed)"
);
} // anonymous namespace

void
FlatDatastore::acquire_stream(
	Hord::IO::PropInfo const& prop_info,
	bool const is_input
) {
	auto const& sinfo_map = get_storage_info();
	auto const it = sinfo_map.find(prop_info.object_id);
	if (sinfo_map.cend() == it) {
		HORD_THROW_FMT(
			Hord::ErrorCode::datastore_object_not_found,
			s_err_object_not_found,
			HORD_SCOPE_FQN_STR_LIT,
			Hord::Object::IDPrinter{prop_info.object_id}
		);
	}

	auto const& sinfo = it->second;
	if (!sinfo.prop_storage.supplies(prop_info.prop_type)) {
		HORD_THROW_FMT(
			Hord::ErrorCode::datastore_prop_unsupplied,
			s_err_acquire_prop_unsupplied,
			Hord::Object::IDPrinter{prop_info.object_id},
			Hord::IO::get_prop_type_name(prop_info.prop_type),
			Hord::Object::get_base_type_name(sinfo.object_type.base())
		);
	}

	if (is_input && !sinfo.prop_storage.is_initialized(prop_info.prop_type)) {
		HORD_THROW_FMT(
			Hord::ErrorCode::datastore_prop_void,
			s_err_acquire_prop_void,
			Hord::Object::IDPrinter{prop_info.object_id},
			Hord::IO::get_prop_type_name(prop_info.prop_type)
		);
	}

	// TODO: stat() path, throw other custom/standard error if
	// non-existent
	assign_prop(prop_info, sinfo, is_input);
	m_prop.stream.open(
		m_prop.path,
		std::ios_base::binary
		| (is_input)
			? std::ios_base::in
			: std::ios_base::out
	);
	if (!m_prop.stream.is_open()) {
		// TODO: This should really not be datastore_prop_void (see above)
		m_prop.reset();
		HORD_THROW_FMT(
			Hord::ErrorCode::datastore_prop_void,
			s_err_acquire_prop_open_failed,
			Hord::Object::IDPrinter{prop_info.object_id},
			Hord::IO::get_prop_type_name(prop_info.prop_type)
		);
	}

	base::enable_state(State::locked);
}
#undef HORD_SCOPE_FUNC

#define HORD_SCOPE_FUNC release_stream
namespace {
HORD_DEF_FMT_FQN(
	s_err_release_prop_not_locked,
	"prop %s -> %s is not locked"
);
} // anonymous namespace

void
FlatDatastore::release_stream(
	Hord::IO::PropInfo const& prop_info,
	bool const is_input
) {
	// NB: Base checks is_locked() for us, which essentially ensures
	// true == m_prop.stream.is_open(), but we want to unlock if the
	// fstream somehow became closed in the interim.

	// This implementation knows which object the prop belongs to,
	// so there is no need to handle EC datastore_object_not_found
	// from the spec (because we don't need to lookup anything to
	// get at our representation of the prop).

	if (
		!prop_info_equal(m_prop.info, prop_info) ||
		is_input != m_prop.is_input
	) {
		HORD_THROW_FMT(
			Hord::ErrorCode::datastore_prop_not_locked,
			s_err_release_prop_not_locked,
			Hord::Object::IDPrinter{prop_info.object_id},
			Hord::IO::get_prop_type_name(prop_info.prop_type)
		);
	}

	try {
		// Ignore exceptions during close
		m_prop.stream.close();
	} catch (...) {}
	m_prop.reset();
	base::disable_state(State::locked);
}
#undef HORD_SCOPE_FUNC


// Hord::IO::Datastore implementation

#define HORD_SCOPE_FUNC open_impl
void
FlatDatastore::open_impl(
	bool const create_if_nonexistent
) try {
	// NB: open() protects us from the ill logic of trying to open
	// when the datastore is already open

	bool do_index = true;

	// wow
	namespace fs = boost::filesystem;
	boost::system::error_code ec;
	fs::path const path{get_root_path()};
	auto const stat = fs::status(path, ec);
	if (!fs::is_directory(stat)) {
		HORD_THROW_FQN(
			Hord::ErrorCode::datastore_open_failed,
			"root path does not exist or is not a directory"
		);
	}

	// Only create stubs if the directory exists and is empty
	// (Hord generalizes the parameter as "if datastore does not
	// exist")
	if (
		create_if_nonexistent &&
		fs::directory_iterator(path, ec) == fs::directory_iterator()
	) {
		// boost plz no
		fs::path path_resident{path};
		fs::path path_orphan{path};
		path_resident /= "resident";
		path_orphan /= "orphan";
		if (
			(fs::create_directory(path_resident, ec), ec) ||
			(fs::create_directory(path_orphan, ec), ec)
		) {
			HORD_THROW_FQN(
				Hord::ErrorCode::datastore_open_failed,
				"failed to create object directories"
			);
		}
		do_index = false;
	}

	// Path could've changed (and we don't assign it in the ctor)
	m_lock.set_path(get_root_path() + "/.lock");
	try {
		m_lock.acquire();
		base::enable_state(State::opened);
	} catch (...) {
		HORD_THROW_FQN(
			Hord::ErrorCode::datastore_open_failed,
			"failed to obtain hive lockfile"
		);
	}

	if (do_index) {
		auto const index_path = get_root_path() + "/index";
		std::ifstream index_stream{index_path};
		if (!index_stream.is_open()) {
			HORD_THROW_FQN(
				Hord::ErrorCode::datastore_open_failed,
				"failed to open index file for reading"
			);
		}
		std::exception_ptr eptr;
		try {
			read_index(index_stream);
		} catch (...) {
			eptr = std::current_exception();
		}
		index_stream.close();
		if (eptr) {
			Log::acquire(Log::error)
				<< DUCT_GR_MSG_FQN("failed to read index file:\n")
			;
			Log::report_error_ptr(eptr);
			HORD_THROW_FQN(
				Hord::ErrorCode::datastore_open_failed,
				"failed to read index file"
			);
		}
	}
} catch (...) {
	m_lock.release();
	throw;
}
#undef HORD_SCOPE_FUNC

#define HORD_SCOPE_FUNC close_impl
void
FlatDatastore::close_impl() {
	// NB: close() protects us from is_locked()

	auto const index_path = get_root_path() + "/index";
	std::ofstream index_stream{index_path};
	if (index_stream.is_open()) {
		try {
			write_index(index_stream);
			index_stream.close();
		} catch (...) {
			Log::acquire(Log::error)
				<< DUCT_GR_MSG_FQN("failed to write index file: '")
				<< index_path
				<< "':\n"
			;
			Log::report_error_ptr(std::current_exception());
		}
	} else {
		Log::acquire(Log::error)
			<< DUCT_GR_MSG_FQN("failed to open index file for writing: '")
			<< index_path
			<< "'\n"
		;
	}

	m_lock.release();
	base::disable_state(State::opened);
}
#undef HORD_SCOPE_FUNC

// acquire
std::istream&
FlatDatastore::acquire_input_stream_impl(
	Hord::IO::PropInfo const& prop_info
) {
	acquire_stream(prop_info, true);
	return m_prop.stream;
}

std::ostream&
FlatDatastore::acquire_output_stream_impl(
	Hord::IO::PropInfo const& prop_info
) {
	acquire_stream(prop_info, false);
	return m_prop.stream;
}

// release
void
FlatDatastore::release_input_stream_impl(
	Hord::IO::PropInfo const& prop_info
) {
	release_stream(prop_info, true);
}

void
FlatDatastore::release_output_stream_impl(
	Hord::IO::PropInfo const& prop_info
) {
	release_stream(prop_info, false);
}


// objects
Hord::Object::ID
FlatDatastore::generate_id_impl(
	Hord::System::IDGenerator& id_generator
) const noexcept {
	// TODO: seed()? Are we allowed to mutate id_generator?
	return id_generator.generate_unique(make_const(get_storage_info()));
}

#define HORD_SCOPE_FUNC create_object_impl
Hord::IO::Datastore::storage_info_map_type::const_iterator
FlatDatastore::create_object_impl(
	Hord::Object::ID const object_id,
	Hord::Object::type_info const& type_info,
	Hord::IO::Linkage const linkage
) {
	// NB: Base protects us from: closed state, locked state, and
	// IDs that already exist
	auto const emplace_pair = get_storage_info().emplace(
		object_id,
		Hord::IO::StorageInfo{
			object_id,
			type_info.type,
			{true, true},
			linkage
		}
	);
	// TODO: Throw if !emplace_pair.second
	return emplace_pair.first;
}
#undef HORD_SCOPE_FUNC

#define HORD_SCOPE_FUNC destroy_object_impl
void
FlatDatastore::destroy_object_impl(
	Hord::Object::ID const object_id
) {
	auto& sinfo_map = get_storage_info();
	auto const it = sinfo_map.find(object_id);
	if (sinfo_map.cend() == it) {
		HORD_THROW_FMT(
			Hord::ErrorCode::datastore_object_not_found,
			s_err_object_not_found,
			HORD_SCOPE_FQN_STR_LIT,
			Hord::Object::IDPrinter{object_id}
		);
	}
	// TODO: Destroy data on the filesystem
	sinfo_map.erase(it);
}
#undef HORD_SCOPE_FUNC

#undef HORD_SCOPE_CLASS // FlatDatastore

} // namespace IO
} // namespace Onsang
