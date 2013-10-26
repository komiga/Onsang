
#include "../utility.hpp"
#include "../String.hpp"
#include "./FlatDatastore.hpp"

#include <Hord/Error.hpp>
#include <Hord/IO/PropStream.hpp>

#include <ceformat/print.hpp>

#include <cstdio>
#include <type_traits>
#include <utility>
#include <new>

namespace Onsang {
namespace IO {

// class FlatDatastore implementation

#define HORD_SCOPE_CLASS_IDENT__ Onsang::IO::FlatDatastore

namespace {

HORD_FMT_UNSCOPED(
	s_err_object_not_found,
	"%s: object %08x does not exist"
);

/*static constexpr ceformat::Format const
s_fmt_object_id{
	ONSANG_STR_LIT("%08x")
};*/

static char const* const
s_prop_type_abbr[]{
	ONSANG_STR_LIT("i"),
	ONSANG_STR_LIT("m"),
	ONSANG_STR_LIT("s"),
	ONSANG_STR_LIT("p"),
	ONSANG_STR_LIT("a"),
};
static_assert(
	static_cast<std::size_t>(Hord::IO::PropType::LAST)
	== std::extent<decltype(s_prop_type_abbr)>::value,
	"PropType abbreviation list is incomplete"
);

}; // anonymous namespace

static bool
type_supplies_prop(
	Hord::Object::Type const type
) noexcept {
	// FIXME: When datastores can contain other objects, this needs
	// to be implemented properly.

	// Currently containing only Node, which supplies all props
	return Hord::Object::Type::Node == type;
}

static bool
prop_info_equal(
	Hord::IO::PropInfo const& x,
	Hord::IO::PropInfo const& y
) {
	return
		x.object_id == y.object_id &&
		x.prop_type == y.prop_type;
}

Hord::IO::Datastore*
FlatDatastore::construct(
	Hord::String root_path
) noexcept {
	return new(std::nothrow) FlatDatastore(std::move(root_path));
}

FlatDatastore::type_info const
FlatDatastore::s_type_info{
	FlatDatastore::construct
};

FlatDatastore::FlatDatastore(
	Hord::String root_path
)
	: base(std::move(root_path))
	, m_lock()
	, m_index()
	, m_prop{
		{},
		{
			Hord::Object::NULL_ID,
			Hord::Object::Type::Hive,
			Hord::IO::PropType::identity
		},
		{},
		false
	}
{}

void
FlatDatastore::assign_prop(
	Hord::IO::PropInfo const& prop_info,
	bool const is_trash,
	bool const is_input
) {
	m_prop.info = prop_info;
	m_prop.is_input = is_input;

	String& path = m_prop.path;
	Hord::String const& root_path = get_root_path();

	path.reserve(
		root_path.size() +		// root
		8u +					// ID
		(is_trash ? 6u : 0u) +	// "/trash"
		6u +					// "/data/"
		1u						// prop name
	);

	// Taking runtime parsing cost over dynamic memory allocation
	// (which would be ceformat which could do this more simply with a
	// memstream which would be terrible... but actually safer... but
	// also evil... I should do it <- ceformat TODO)

	char obj_id_str[9u]; // uses 8 bytes (+1 for NUL)
	std::snprintf(
		obj_id_str,
		static_cast<signed>(std::extent<decltype(obj_id_str)>::value),
		"%08x",
		prop_info.object_id
	);
	path
		.assign(root_path)
		.append(
			(is_trash)
			? "/trash/data/"
			: "/data/"
		)
		.append(obj_id_str, std::extent<decltype(obj_id_str)>::value - 1u)
		.append(s_prop_type_abbr[
			enum_cast(prop_info.prop_type)
		])
	;
}

#define HORD_SCOPE_FUNC_IDENT__ acquire_stream
namespace {
HORD_FMT_SCOPED_FQN(
	s_err_acquire_prop_unsupplied,
	"prop %08x -> %s is not supplied for type %s"
);
HORD_FMT_SCOPED_FQN(
	s_err_acquire_prop_open_failed,
	"prop %08x -> %s is void (or open otherwise failed)"
);
} // anonymous namespace

void
FlatDatastore::acquire_stream(
	Hord::IO::PropInfo const& prop_info,
	bool const is_input
) {
	auto const it = make_const(m_index).find(
		prop_info.object_id
	);
	if (m_index.cend() == it) {
		HORD_THROW_ERROR_F(
			Hord::ErrorCode::datastore_object_not_found,
			s_err_object_not_found,
			HORD_SCOPE_FQN,
			prop_info.object_id
		);
	}

	Index::Entry const& entry = *it;
	if (!type_supplies_prop(entry.type)) {
		HORD_THROW_ERROR_F(
			Hord::ErrorCode::datastore_prop_unsupplied,
			s_err_acquire_prop_unsupplied,
			prop_info.object_id,
			Hord::IO::get_prop_type_name(prop_info.prop_type),
			Hord::Object::get_type_name(entry.type)
		);
	}

	// TODO: stat() path, throw other custom/standard error if
	// non-existent
	assign_prop(prop_info, entry.is_trash, is_input);
	m_prop.stream.open(
		m_prop.path,
		std::ios_base::binary
		| (is_input)
			? std::ios_base::in
			: std::ios_base::out
	);
	if (!m_prop.stream.is_open()) {
		HORD_THROW_ERROR_F(
			Hord::ErrorCode::datastore_prop_void,
			s_err_acquire_prop_open_failed,
			prop_info.object_id,
			Hord::IO::get_prop_type_name(prop_info.prop_type)
		);
	}

	base::enable_state(State::locked);
}
#undef HORD_SCOPE_FUNC_IDENT__

#define HORD_SCOPE_FUNC_IDENT__ release_stream
namespace {
HORD_FMT_SCOPED_FQN(
	s_err_release_prop_not_locked,
	"prop %08x -> %s is not locked"
);
} // anonymous namespace

void
FlatDatastore::release_stream(
	Hord::IO::PropInfo const& prop_info,
	bool const is_input
) {
	// NB: Base checks is_locked() for us, which essentially ensures
	// true==m_prop.stream.is_open(), but we want to unlock if the
	// fstream somehow became closed in the interim.

	// This implementation knows which object the prop belongs to,
	// so there is no need to handle EC datastore_object_not_found
	// from the spec (because we don't need to lookup anything to
	// get at our representation of the prop).

	if (
		!prop_info_equal(m_prop.info, prop_info) ||
		is_input != m_prop.is_input
	) {
		HORD_THROW_ERROR_F(
			Hord::ErrorCode::datastore_prop_not_locked,
			s_err_release_prop_not_locked,
			prop_info.object_id,
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
#undef HORD_SCOPE_FUNC_IDENT__


// Hord::IO::Datastore implementation


#define HORD_SCOPE_FUNC_IDENT__ open_impl
void
FlatDatastore::open_impl() {
	// NB: open() protects us from the ill logic of trying to open
	// when the datastore is already open

	// Path could've changed (and we don't assign it in the ctor)
	m_lock.set_path(get_root_path());
	try {
		m_lock.acquire();
		base::enable_state(State::opened);
	} catch (Hord::Error&) {
		HORD_THROW_ERROR_SCOPED_FQN(
			Hord::ErrorCode::datastore_open_failed,
			"failed to obtain hive lockfile"
		);
	} catch (...) {
		throw;
	}
}
#undef HORD_SCOPE_FUNC_IDENT__

void
FlatDatastore::close_impl() {
	// NB: close() protects us from is_locked()
	m_lock.release();
	base::disable_state(State::opened);
}


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
	return id_generator.generate_unique(m_index);
}

#define HORD_SCOPE_FUNC_IDENT__ create_object_impl
namespace {
HORD_FMT_SCOPED_FQN(
	s_err_create_object_already_exists,
	"object %08x (of type %s) already exists"
);
} // anonymous namespace

void
FlatDatastore::create_object_impl(
	Hord::Object::ID const object_id,
	Hord::Object::Type const /*object_type*/
) {
	// NB: Base protects us from prohibited types

	auto const it = make_const(m_index).find(object_id);
	if (m_index.cend() != it) {
		HORD_THROW_ERROR_F(
			Hord::ErrorCode::datastore_object_already_exists,
			s_err_create_object_already_exists,
			object_id,
			Hord::Object::get_type_name(it->type)
		);
	}

	// Currently only nodes are supported
	m_index.insert(object_id);
}
#undef HORD_SCOPE_FUNC_IDENT__

#define HORD_SCOPE_FUNC_IDENT__ destroy_object_impl
void
FlatDatastore::destroy_object_impl(
	Hord::Object::ID const object_id
) {
	auto const it = m_index.find(object_id);
	if (m_index.cend() == it) {
		HORD_THROW_ERROR_F(
			Hord::ErrorCode::datastore_object_not_found,
			s_err_object_not_found,
			HORD_SCOPE_FQN,
			object_id
		);
	}

	// Sending to trash; will purge later
	// NB: is_trash has no bearing on hash, so this should not
	// affect the container
	const_cast<Index::Entry&>(*it).is_trash = true;
}
#undef HORD_SCOPE_FUNC_IDENT__

#undef HORD_SCOPE_CLASS_IDENT__ // FlatDatastore

} // namespace IO
} // namespace Onsang
