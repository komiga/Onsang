/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief %Session.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>

#include <Hord/IO/Datastore.hpp>
#include <Hord/System/Driver.hpp>
#include <Hord/System/Context.hpp>

#include <utility>
#include <exception>

namespace Onsang {
namespace System {

// Forward declarations
class Session;

/**
	Session.
*/
class Session
	: public Hord::System::Context
{
private:
	using base = Hord::System::Context;

	String m_name;
	String m_path;
	bool m_auto_open;
	bool m_auto_create;
	// TODO: Views

	Session() = delete;
	Session(Session const&) = delete;
	Session& operator=(Session const&) = delete;

// Hord::System::Context implementation
private:
	void
	notify_exception_impl(
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::type_info const& type_info,
		std::exception_ptr eptr
	) noexcept override;

	void
	notify_complete_impl(
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::type_info const& type_info
	) noexcept override;

public:
// special member functions
	~Session() override = default;
	Session(Session&&) = default;
	Session& operator=(Session&&) = default;

	/**
		Throws Hord::Error:
		- see Hord::System::Context::Context()
	*/
	Session(
		Hord::System::Driver& driver,
		Hord::IO::Datastore::ID const datastore_id,
		String name,
		String path,
		bool const auto_open,
		bool const auto_create
	) noexcept
		: base(driver, datastore_id)
		, m_name(std::move(name))
		, m_path(std::move(path))
		, m_auto_open(auto_open)
		, m_auto_create(auto_create)
	{}

// properties
	String const&
	get_name() const noexcept {
		return m_name;
	}

	String const&
	get_path() const noexcept {
		return m_path;
	}

	bool
	get_auto_open() const noexcept {
		return m_auto_open;
	}

	bool
	get_auto_create() const noexcept {
		return m_auto_create;
	}

	bool
	is_open() const noexcept {
		return get_datastore().is_open();
	}

// operations
	/**
		Throws Hord::Error:
		- see Hord::IO::Datastore::open()
	*/
	void
	open();

	/**
		Throws Hord::Error:
		- see Hord::IO::Datastore::close()
	*/
	void
	close();

	/**
		Process views.
	*/
	void
	process();
};

} // namespace System
} // namespace Onsang
