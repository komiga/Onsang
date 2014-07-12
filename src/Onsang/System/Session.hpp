/**
@file System/Session.hpp
@brief %Session.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_SYSTEM_SESSION_HPP_
#define ONSANG_SYSTEM_SESSION_HPP_

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>

#include <Hord/IO/Datastore.hpp>
#include <Hord/System/Driver.hpp>
#include <Hord/System/Context.hpp>

#include <utility>

namespace Onsang {
namespace System {

// Forward declarations
class Session;

/**
	Hive session.
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

public:
// special member functions
	~Session() = default;
	Session(Session&&) = default;
	Session& operator=(Session&&) = default;

	/**
		Throws Hord::Error:
		- see Hord::System::Context::Context()
	*/
	Session(
		Hord::System::Context&& context,
		String name,
		String path,
		bool const auto_open,
		bool const auto_create
	) noexcept
		: base(std::move(context))
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

#endif // ONSANG_SYSTEM_SESSION_HPP_