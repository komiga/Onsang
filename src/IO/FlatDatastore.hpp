/**
@file IO/FlatDatastore.hpp
@brief FlatDatastore class.

@author Tim Howard
@copyright 2013 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_IO_FLATDATASTORE_HPP_
#define ONSANG_IO_FLATDATASTORE_HPP_

#include "../config.hpp"
#include "../aux.hpp"
#include "../utility.hpp"
#include "../String.hpp"

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
	"index" set<Identity>;
	"data/$id/m" Metadata;
	"data/$id/p" <primary data>;
	"data/$id/a" <aux data>;
	"data/$id/s" <scratch space>;
	"trash/"
		[same layout]

*/

namespace Onsang {
namespace IO {

// Forward declarations
class FlatDatastore;

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
	// NB: Currently only Nodes can be created, so we don't track
	// the type dynamically
	struct Identity final {
		static constexpr Hord::Object::Type const
		type = Hord::Object::Type::Node;

		Hord::Object::ID id;
		bool is_trash;

		/*struct hash {
			std::size_t
			operator()(
				Identity const& identity
			) noexcept {
				return static_cast<std::size_t>(identity.id);
			}
		};*/
	};

	struct {
		String path;
		Hord::IO::PropInfo info;
		std::fstream stream;
		bool is_input;

		void
		reset() {
			path.clear();
			info.object_id = Hord::Object::NULL_ID;
		}
	} m_prop;

	Hord::LockFile m_lock;
	aux::unordered_map<
		Hord::Object::ID,
		Identity/*,
		Identity::hash*/
	> m_index; // contains trash objects

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
