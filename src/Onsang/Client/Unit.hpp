/**
@file Client/Unit.hpp
@brief Unit.

@author Tim Howard
@copyright 2013-2014 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_CLIENT_UNIT_HPP_
#define ONSANG_CLIENT_UNIT_HPP_

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/asio.hpp>
#include <Onsang/init.hpp>
#include <Onsang/ConfigNode.hpp>
#include <Onsang/Net/Defs.hpp>
#include <Onsang/Net/Session.hpp>

#include <Beard/ui/Context.hpp>
#include <Beard/ui/PropertyMap.hpp>

#include <Hord/System/Driver.hpp>

#include <duct/StateStore.hpp>

namespace Onsang {
namespace Client {

class Unit final {
public:
	using session_vector_type = aux::vector<Net::Session>;

private:
	enum class Flags : unsigned {
		no_auto_connect = 1u << 0,
	};

	duct::StateStore<Flags> m_flags;
	Hord::System::Driver m_driver;

	asio::io_service m_io_service;
	session_vector_type m_sessions;

	Beard::ui::Context m_ui_ctx;

	ConfigNode m_config;
	ConfigNode m_args;

	Unit(Unit const&) = delete;
	Unit& operator=(Unit const&) = delete;

public:
// constructors, destructor, and operators
	~Unit() = default;
	Unit(Unit&&) = default;
	Unit& operator=(Unit&&) = default;

	Unit()
		: m_flags()
		, m_driver()
		, m_io_service()
		, m_sessions()
		, m_ui_ctx(Beard::ui::PropertyMap{true})
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
							{"addr", {
								{duct::VarType::string}
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
			}}
		})
	{
		driver_init(m_driver);
	}

// properties
	Hord::System::Driver&
	get_driver() noexcept {
		return m_driver;
	}

	session_vector_type const&
	get_sessions() const noexcept {
		return m_sessions;
	}

// operations
	// May throw Hord::Error
	void
	add_session(
		String name,
		String type,
		String addr
	);

	bool
	init(
		signed const argc,
		char* argv[]
	);

	void
	start();
};

} // namespace Client
} // namespace Onsang

#endif // ONSANG_CLIENT_UNIT_HPP_
