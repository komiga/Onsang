/**
@file Client/Unit.hpp
@brief Unit.

@author Tim Howard
@copyright 2013 Tim Howard under the MIT license;
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

namespace Onsang {
namespace Client {

class Unit final {
public:
	using session_vector_type = aux::vector<Net::Session>;

private:
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
		: m_driver()
		, m_io_service()
		, m_sessions()
		, m_ui_ctx(Beard::ui::PropertyMap{true})
		, m_config(
			{},
			{
				{"log", {
					{"path", {
						{duct::var_mask(
							duct::VarType::string,
							duct::VarType::null
						)},
						ConfigNode::Flags::optional
					}}
				}},
				{"sessions", {
					{"", {
						new ConfigNode({
							{"type", {
								{duct::var_mask(duct::VarType::string)}
							}},
							{"addr", {
								{duct::var_mask(duct::VarType::string)}
							}}
						})
					}}
				}}
			}
		)
		, m_args({
			{"--config", {
				{duct::VarMask::value},
				ConfigNode::Flags::optional
			}},
			{"--log", {
				{duct::VarMask::value},
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
		String address
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
