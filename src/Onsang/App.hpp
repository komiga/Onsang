/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief App class.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/ConfigNode.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/System/SessionManager.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedContainer.hpp>

#include <Beard/ui/Defs.hpp>
#include <Beard/ui/Widget/Defs.hpp>
#include <Beard/ui/Context.hpp>
#include <Beard/ui/PropertyMap.hpp>
#include <Beard/ui/Container.hpp>
#include <Beard/ui/Label.hpp>
#include <Beard/ui/Field.hpp>

#include <Hord/IO/Datastore.hpp>
#include <Hord/System/Driver.hpp>

#include <duct/StateStore.hpp>

namespace Onsang {

class App final {
public:
	struct UIBucket {
		UI::Container::SPtr viewc{};
		UI::Label::SPtr sline{};
		UI::Field::SPtr cline{};
	};

private:
	enum class Flags : unsigned {
		no_auto_open = bit(0u),
		no_stdout    = bit(1u),
	};

	duct::StateStore<Flags> m_flags{};
	Hord::System::Driver m_driver{true};

	System::SessionManager m_session_manager;

	bool m_running{false};
	UI::Context m_ui_ctx{UI::PropertyMap{true}};
	UIBucket m_ui{};
	System::Session* m_session{nullptr};

	ConfigNode m_config;
	ConfigNode m_args;

	App(App const&) = delete;
	App& operator=(App const&) = delete;
	App& operator=(App&&) = delete;

	void
	toggle_stdout(
		bool const enable
	);

public:
// constructors, destructor, and operators
	~App() = default;
	App(App&&) = default;

	App()
		: m_session_manager(m_driver)
		, m_config(
			{},
			{
				{"log", {ConfigNode::Flags::optional, {
					{"path", {
						{duct::VarType::string},
						ConfigNode::Flags::optional
					}}
				}}},
				{"term", {
					{"info", {
						{duct::VarType::string}
					}}
				}},
				{"sessions", {
					{"builder", {
						new ConfigNode({
							{"type", {
								{duct::VarType::string}
							}},
							{"path", {
								{duct::VarType::string}
							}},
							{"auto-open", {
								{duct::VarType::boolean},
								ConfigNode::Flags::optional
							}},
							{"auto-create", {
								{duct::VarType::boolean},
								ConfigNode::Flags::optional
							}}
						}),
						ConfigNode::Flags::node_matcher_named
					}}
				}}
			}
		)
		, m_args({
			{"--config", {
				{duct::VarMask::value}
			}},
			{"--log", {
				{duct::VarMask::value},
				ConfigNode::Flags::optional
			}},
			{"--no-auto", {
				{duct::VarType::null},
				ConfigNode::Flags::optional
			}},
			{"--no-stdout", {
				{duct::VarType::null},
				ConfigNode::Flags::optional
			}}
		})
	{}

// properties
	Hord::System::Driver&
	get_driver() noexcept {
		return m_driver;
	}

	System::SessionManager&
	get_session_manager() noexcept {
		return m_session_manager;
	}

	UI::Context&
	get_ui_context() noexcept {
		return m_ui_ctx;
	}

	UIBucket&
	get_ui() noexcept {
		return m_ui;
	}

// operations
	bool
	add_session(
		String const& type,
		String const& name,
		String const& path,
		bool const auto_open,
		bool const auto_create
	);

public:
	bool
	init(
		signed const argc,
		char* argv[]
	);

	void
	set_session(
		System::Session* const session
	);

private:
	void
	init_session(
		System::Session&
	);

	void
	close_session(
		System::Session&
	);

	void
	start_ui();

	void
	ui_event_filter(
		UI::Event const&
	);

public:
	void
	start();
};

} // namespace Onsang
