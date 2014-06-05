/**
@file IO/FlatDatastore.hpp
@brief FlatDatastore class.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
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
#include <Hord/IO/StorageInfo.hpp>
#include <Hord/IO/Datastore.hpp>

#include <fstream>

/*

Flat-file datastore.

Structure:

"root/"
	".lock" LockFile;
	"index" <storage info>;
	"resident/$id/i" <identity>;
	"resident/$id/m" Metadata;
	"resident/$id/s" <scratch space>;
	"resident/$id/p" <primary data>;
	"resident/$id/a" <aux data>;
	"orphan/"
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
	Hord::LockFile m_lock;

	struct {
		String path{};
		Hord::IO::PropInfo info{
			Hord::Object::ID_NULL,
			Hord::Object::TYPE_NULL,
			Hord::IO::PropType::identity
		};
		std::fstream stream{};
		bool is_input{false};

		void
		reset() noexcept {
			path.clear();
			info.object_id = Hord::Object::ID_NULL;
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
		Hord::IO::StorageInfo const&,
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
		Hord::Object::type_info const&,
		Hord::IO::Linkage const
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
