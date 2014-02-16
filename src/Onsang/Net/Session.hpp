/**
@file Net/Session.hpp
@brief Session.

@author Tim Howard
@copyright 2013 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_NET_SESSION_HPP_
#define ONSANG_NET_SESSION_HPP_

#include <Onsang/config.hpp>
#include <Onsang/Net/Defs.hpp>
#include <Onsang/Net/CmdStreamer.hpp>

#include <Hord/System/Driver.hpp>
#include <Hord/System/Context.hpp>

#include <utility>

namespace Onsang {
namespace Net {

class Session final {
private:
	Hord::System::Context m_context;
	Net::Socket m_socket;
	Net::CmdStreamer m_streamer;

	Session() = delete;
	Session(Session const&) = delete;
	Session& operator=(Session const&) = delete;

public:
// constructors, destructor, and operators
	~Session() = default;
	Session(Session&&) = default;
	Session& operator=(Session&&) = default;

	Session(
		Hord::System::Context context,
		Net::Socket socket
	) noexcept
		: m_context(std::move(context))
		, m_socket(std::move(socket))
		, m_streamer(
			m_context.get_driver(),
			m_socket
		)
	{}

// properties
	Hord::System::Context&
	get_context() noexcept {
		return m_context;
	}

	Net::Socket&
	get_socket() noexcept {
		return m_socket;
	}

	Net::CmdStreamer&
	get_streamer() noexcept {
		return m_streamer;
	}

	bool
	is_connected() const noexcept {
		return m_socket.is_open();
	}
};

} // namespace Net
} // namespace Onsang

#endif // ONSANG_NET_SESSION_HPP_
