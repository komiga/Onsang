/**
@file System/SessionManager.hpp
@brief %Session manager.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/System/Session.hpp>

#include <Hord/Hive/Defs.hpp>
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
	using session_collection_type = aux::vector<System::Session>;
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

private:
	struct find_pred final {
		Hord::Hive::ID const id;

		bool
		operator()(
			Session const& session
		) const noexcept {
			return session.get_hive().get_id_bare() == id;
		}
	};

public:
	iterator
	find(
		Hord::Hive::ID const id
	) noexcept {
		return std::find_if(
			m_sessions.begin(),
			m_sessions.end(),
			find_pred{id}
		);
	}

	const_iterator
	find(
		Hord::Hive::ID const id
	) const noexcept {
		return std::find_if(
			m_sessions.cbegin(),
			m_sessions.cend(),
			find_pred{id}
		);
	}

// operations
	/**
		Add a session by string type.

		Throws Onsang::Error:
		- ErrorCode::session_type_unrecognized

		Throws Hord::Error:
		- see Hord::System::Driver::placehold_hive()
		- see Hord::System::Context::Context()
	*/
	Hord::Hive::ID
	add_session(
		Hord::Hive::Type const hive_type,
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
