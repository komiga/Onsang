/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief %Session class.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/System/Defs.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/SessionView.hpp>

#include <Hord/Object/Defs.hpp>
#include <Hord/IO/Datastore.hpp>
#include <Hord/System/Driver.hpp>
#include <Hord/System/Context.hpp>

#include <utility>
#include <exception>

namespace Onsang {
namespace System {

class Session
	: public Hord::System::Context
{
public:
	using UPtr = aux::unique_ptr<System::Session>;

private:
	using base = Hord::System::Context;
	enum class ctor_priv {};

	String m_name;
	String m_path;
	bool m_auto_open;
	bool m_auto_create;
	UI::SessionView::SPtr m_view;

	Session() = delete;
	Session(Session const&) = delete;
	Session& operator=(Session const&) = delete;

// Hord::System::Context implementation
private:
	void
	notify_exception_impl(
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::TypeInfo const& type_info,
		std::exception_ptr eptr
	) noexcept override;

	void
	notify_complete_impl(
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::TypeInfo const& type_info
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
		ctor_priv const,
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
		, m_view()
	{}

	static System::Session::UPtr
	make(
		Hord::System::Driver& driver,
		Hord::IO::Datastore::ID const datastore_id,
		String name,
		String path,
		bool const auto_open,
		bool const auto_create
	) {
		return System::Session::UPtr{new System::Session(
			ctor_priv{},
			driver,
			datastore_id,
			name,
			path,
			auto_open,
			auto_create
		)};
	}

// properties
	String const&
	name() const noexcept {
		return m_name;
	}

	String const&
	path() const noexcept {
		return m_path;
	}

	bool
	auto_open() const noexcept {
		return m_auto_open;
	}

	bool
	auto_create() const noexcept {
		return m_auto_create;
	}

	UI::SessionView::SPtr
	view() noexcept {
		return m_view;
	}

	bool
	is_open() const noexcept {
		return datastore().is_open();
	}

// operations
	/**
		Throws Hord::Error:
		- see Hord::IO::Datastore::open()
	*/
	void
	open(
		UI::RootWPtr root
	);

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
