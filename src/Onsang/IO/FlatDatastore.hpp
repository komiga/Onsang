/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Flat-file datastore.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/String.hpp>

#include <Hord/LockFile.hpp>
#include <Hord/Object/Defs.hpp>
#include <Hord/IO/Defs.hpp>
#include <Hord/IO/Prop.hpp>
#include <Hord/IO/StorageInfo.hpp>
#include <Hord/IO/Datastore.hpp>

#include <iostream>
#include <fstream>

/*

Flat-file datastore.

Structure:

"root/"
	".lock" LockFile;
	"index" <storage info>;
	"resident/"
		"$id/i" <identity>;
		"$id/m" Metadata;
		"$id/s" <scratch space>;
		"$id/p" <primary data>;
		"$id/a" <aux data>;
	"orphan/"
		[same layout]

*/

namespace Onsang {
namespace IO {

class FlatDatastore final
	: public Hord::IO::Datastore
{
public:
	using base = Hord::IO::Datastore;
	using base::State;

	static base::TypeInfo const
	s_type_info;

private:
	Hord::LockFile m_lock;

	struct {
		String directory{};
		Hord::IO::PropInfo info{
			Hord::Object::ID_NULL,
			Hord::Object::TYPE_NULL,
			Hord::IO::PropType::identity
		};
		Hord::IO::StorageInfo* sinfo;
		std::fstream stream{};
		bool is_input{false};

		void
		reset() noexcept {
			directory.clear();
			info.object_id = Hord::Object::ID_NULL;
			sinfo = nullptr;
		}
	} m_prop;

	static Hord::IO::Datastore::UPtr
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
	read_index(
		std::istream&
	);

	void
	write_index(
		std::ostream&
	);

	void
	assign_prop(
		Hord::IO::PropInfo const&,
		Hord::IO::StorageInfo&,
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

// Hord::IO::Datastore implementation
private:
	void
	open_impl(
		bool const create_if_nonexistent
	) override;

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

	Hord::IO::Datastore::storage_info_map_type::const_iterator
	create_object_impl(
		Hord::Object::ID const,
		Hord::Object::TypeInfo const&,
		Hord::IO::Linkage const
	) override;

	void
	destroy_object_impl(
		Hord::Object::ID const
	) override;

public:
// operations
	/**
		Creates the datastore if the root path is empty.

		Throws Hord::Error:
		- see Hord::IO::Datastore::open()
	*/
	void
	open(
		bool const create_if_empty
	);
};

} // namespace IO
} // namespace Onsang

template struct Hord::IO::Datastore::ensure_traits<
	Onsang::IO::FlatDatastore
>;
