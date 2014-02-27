/**
@file IO/FlatDatastore.hpp
@brief FlatDatastore class.

@author Tim Howard
@copyright 2013-2014 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_IO_FLATDATASTORE_HPP_
#define ONSANG_IO_FLATDATASTORE_HPP_

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/String.hpp>

#include <Hord/LockFile.hpp>
#include <Hord/Object/Defs.hpp>
#include <Hord/Hive/Defs.hpp>
#include <Hord/IO/Defs.hpp>
#include <Hord/IO/Prop.hpp>
#include <Hord/IO/Datastore.hpp>

#include <fstream>

/*

Flat-file datastore.

Structure:

"hive/"
	".lock" LockFile;
	"index" Index;
	"data/$id/i" <identity>;
	"data/$id/m" Metadata;
	"data/$id/s" <scratch space>;
	"data/$id/p" <primary data>;
	"data/$id/a" <aux data>;
	"trash/"
		[same layout]

*/

namespace Onsang {
namespace IO {

// Forward declarations
class FlatDatastore;

namespace {

class Index final {
public:
	// NB: Currently only Nodes can be created, so we don't track
	// the type dynamically
	struct Entry final {
		static constexpr Hord::Object::Type const
		type = Hord::Object::Type::Node;

		Hord::Object::ID id;
		bool is_trash;

		// TODO: C++14: Also add operator== for Object::ID <-> Entry
		// comparison in unordered_set::find() so that we can
		// use unordered_set and Object::ID directly.

		struct hash final {
			constexpr std::size_t
			operator()(
				Entry const& entry
			) const noexcept {
				return static_cast<std::size_t>(entry.id);
			}
		};

		// For lookup purposes; only considering entries equal
		// if the IDs are equal
		constexpr bool
		operator==(
			Entry const& other
		) const noexcept {
			return id == other.id;
		}
	};

	using container_type
	= aux::unordered_set<
		Entry,
		Entry::hash
	>;

	using key_type = Hord::Object::ID;
	using iterator = container_type::iterator;
	using const_iterator = container_type::const_iterator;

private:
	container_type m_entries;

	Index(Index const&) = delete;
	Index(Index&&) = delete;
	Index& operator=(Index const&) = delete;
	Index& operator=(Index&&) = delete;

public:
	~Index() = default;
	Index() = default;

// properties
	iterator
	begin() noexcept { return m_entries.begin(); }
	iterator
	end() noexcept { return m_entries.end(); }

	const_iterator
	cbegin() const noexcept { return m_entries.cbegin(); }
	const_iterator
	cend() const noexcept { return m_entries.cend(); }

// lookup
	iterator
	find(
		key_type const& key
	) noexcept {
		return m_entries.find(Entry{key, false});
	}

	const_iterator
	find(
		key_type const& key
	) const noexcept {
		return m_entries.find(Entry{key, false});
	}

// operations
	std::pair<iterator, bool>
	insert(
		Hord::Object::ID const id
	) {
		return m_entries.insert(Entry{id, false});
	}

	iterator
	erase(
		const_iterator pos
	) noexcept {
		return m_entries.erase(pos);
	}
};

} // anonymous namespace

/**
	Flat-file datastore.
*/
class FlatDatastore final
	: public Hord::IO::Datastore
{
public:
	using base = Hord::IO::Datastore;
	using base::type_info;
	using base::State;

	static FlatDatastore::type_info const
	s_type_info;

private:
	Hord::LockFile m_lock;
	Index m_index;

	struct {
		String path;
		Hord::IO::PropInfo info;
		std::fstream stream;
		bool is_input;

		void
		reset() noexcept {
			path.clear();
			info.object_id = Hord::Object::NULL_ID;
		}
	} m_prop;

	static Hord::IO::Datastore*
	construct(
		Hord::String root_path
	) noexcept;

	FlatDatastore() = delete;
	FlatDatastore(FlatDatastore const&) = delete;
	FlatDatastore(FlatDatastore&&) = delete;
	FlatDatastore& operator=(FlatDatastore const&) = delete;
	FlatDatastore& operator=(FlatDatastore&&) = delete;

	FlatDatastore(
		Hord::String root_path
	);

public:
	~FlatDatastore() noexcept override = default;

private:
	void
	assign_prop(
		Hord::IO::PropInfo const&,
		bool const is_trash,
		bool const is_input
	);

	void
	acquire_stream(
		Hord::IO::PropInfo const&,
		bool const is_input
	);
	void
	release_stream(
		Hord::IO::PropInfo const&,
		bool const is_input
	);

// Hord::Datastore implementation
private:
	void
	open_impl() override;

	void
	close_impl() override;

// acquire
	std::istream&
	acquire_input_stream_impl(
		Hord::IO::PropInfo const&
	) override;

	std::ostream&
	acquire_output_stream_impl(
		Hord::IO::PropInfo const&
	) override;

// release
	void
	release_input_stream_impl(
		Hord::IO::PropInfo const&
	) override;

	void
	release_output_stream_impl(
		Hord::IO::PropInfo const&
	) override;

// objects
	Hord::Object::ID
	generate_id_impl(
		Hord::System::IDGenerator&
	) const noexcept override;

	void
	create_object_impl(
		Hord::Object::ID const,
		Hord::Object::Type const
	) override;

	void
	destroy_object_impl(
		Hord::Object::ID const
	) override;
};

} // namespace IO
} // namespace Onsang

template struct Hord::IO::Datastore::ensure_traits<
	Onsang::IO::FlatDatastore
>;

#endif // ONSANG_IO_FLATDATASTORE_HPP_
