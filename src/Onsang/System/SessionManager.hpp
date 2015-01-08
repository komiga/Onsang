/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief %Session manager.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/System/Session.hpp>

#include <Hord/System/Driver.hpp>

namespace Onsang {
namespace System {

// Forward declarations
class SessionManager;

/**
	Session manager.
*/
class SessionManager {
public:
	using session_collection_type = aux::unordered_map<
		Hord::IO::Datastore::ID,
		System::Session::UPtr
	>;
	using iterator = session_collection_type::iterator;
	using const_iterator = session_collection_type::const_iterator;

private:
	Hord::System::Driver& m_driver;
	session_collection_type m_sessions{};

	SessionManager() = delete;
	SessionManager(SessionManager const&) = delete;
	SessionManager& operator=(SessionManager const&) = delete;
	SessionManager& operator=(SessionManager&&) = delete;

public:
// special member functions
	~SessionManager() = default;
	SessionManager(SessionManager&&) = default;

	SessionManager(
		Hord::System::Driver& driver
	) noexcept
		: m_driver(driver)
	{}

// properties
	Hord::System::Driver&
	get_driver() noexcept {
		return m_driver;
	}

	session_collection_type&
	get_sessions() noexcept {
		return m_sessions;
	}

	session_collection_type const&
	get_sessions() const noexcept {
		return m_sessions;
	}

// container interface
	iterator
	begin() noexcept {
		return m_sessions.begin();
	}

	iterator
	end() noexcept {
		return m_sessions.end();
	}

	const_iterator
	begin() const noexcept {
		return m_sessions.begin();
	}

	const_iterator
	end() const noexcept {
		return m_sessions.end();
	}

	const_iterator
	cbegin() const noexcept {
		return m_sessions.cbegin();
	}

	const_iterator
	cend() const noexcept {
		return m_sessions.cend();
	}

public:
	iterator
	find(
		Hord::IO::Datastore::ID const id
	) noexcept {
		return m_sessions.find(id);
	}

	const_iterator
	find(
		Hord::IO::Datastore::ID const id
	) const noexcept {
		return m_sessions.find(id);
	}

// operations
	/**
		Add a session by string type.

		Throws Onsang::Error:
		- ErrorCode::session_type_unrecognized

		Throws Hord::Error:
		- see Hord::System::Driver::placehold_datastore()
		- see Hord::System::Context::Context()
	*/
	Hord::IO::Datastore::ID
	add_session(
		String const& type,
		String const& name,
		String const& path,
		bool const auto_open,
		bool const auto_create
	);

	/**
		Process sessions.
	*/
	void
	process();
};

} // namespace System
} // namespace Onsang
