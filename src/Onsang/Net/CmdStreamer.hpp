/**
@file Net/CmdStreamer.hpp
@brief CmdStreamer.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_NET_CMDSTREAMER_HPP_
#define ONSANG_NET_CMDSTREAMER_HPP_

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/Net/Defs.hpp>

#include <Hord/System/Driver.hpp>
#include <Hord/System/Context.hpp>

#include <atomic>
#include <mutex>

namespace Onsang {
namespace Net {

class CmdStreamer final {
private:
	Hord::System::Driver const& m_driver;
	Net::Socket& m_socket;

	std::size_t m_incoming_size;
	uint32_t m_incoming_type;
	Net::DynamicStreamBuf m_streambuf_in;

	std::size_t m_outgoing_size;
	Net::DynamicStreamBuf m_streambuf_out;

	std::atomic_bool m_sig_have_stages;
	std::atomic_bool m_sig_pending_output;

	aux::vector<Hord::Cmd::StageUPtr> m_stage_buffer;
	std::mutex m_stage_buffer_mutex;

	CmdStreamer() = delete;
	CmdStreamer(CmdStreamer const&) = delete;
	CmdStreamer& operator=(CmdStreamer const&) = delete;

	bool
	read_header();
	bool
	read_stage();

	void
	chain_read_header();
	void
	chain_read_stage();

	void
	chain_flush_output();

public:
// constructors, destructor, and operators
	~CmdStreamer() = default;
	CmdStreamer(CmdStreamer&&) = default;
	CmdStreamer& operator=(CmdStreamer&&) = default;

	CmdStreamer(
		Hord::System::Driver const& driver,
		Net::Socket& socket,
		std::size_t const max_streambuf_capacity = 0x2000
	) noexcept
		: m_driver(driver)
		, m_socket(socket)
		, m_incoming_size(0u)
		, m_incoming_type(0u)
		, m_streambuf_in(
			max_streambuf_capacity >> 2,
			0u,
			max_streambuf_capacity
		)
		, m_outgoing_size(0u)
		, m_streambuf_out(
			max_streambuf_capacity >> 2,
			512u,
			max_streambuf_capacity
		)
		, m_sig_have_stages(false)
		, m_sig_pending_output(false)
		, m_stage_buffer()
		, m_stage_buffer_mutex()
	{}

// operations
	void
	halt() noexcept;

	// Can throw through io_service/IO operations
	void
	run();

	// Pushes input stages to the context
	bool
	context_input(
		Hord::System::Context& context
	);

	// Pulls output stages from the context and sends them to the
	// remote endpoint
	bool
	context_output(
		Hord::System::Context& context
	);
};

} // namespace Net
} // namespace Onsang

#endif // ONSANG_NET_CMDSTREAMER_HPP_
