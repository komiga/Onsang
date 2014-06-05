
#include <Onsang/utility.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/asio.hpp>
#include <Onsang/serialization.hpp>
#include <Onsang/Net/CmdStreamer.hpp>

#include <Hord/Error.hpp>
#include <Hord/ErrorCode.hpp>

#include <duct/IO/arithmetic.hpp>

#include <cassert>
#include <limits>
#include <mutex>
#include <iomanip>
#include <iostream>

#include <Onsang/detail/gr_ceformat.hpp>

namespace Onsang {
namespace Net {

// class CmdStreamer implementation

#define ONSANG_SCOPE_CLASS Net::CmdStreamer

namespace {

enum : unsigned {
	// uint32 size
	// uint32 type
	msg_header_size = 8u,

	// stage : 8
	// cmd   : 24
	mask_type_stage	= 0x000000FF,
	mask_type_cmd	= 0xFFFFFF00
};

static constexpr ceformat::Format const
	s_err_command_type_invalid{
		": command type %#08x is invalid\n"
	},
	s_err_stage_type_invalid{
		": stage type %#02x is invalid for command type %#08x\n"
	}
;

} // anonymous namespace

#define ONSANG_SCOPE_FUNC read_header
bool
CmdStreamer::read_header() {
	std::istream stream{&m_streambuf_in};
	std::size_t const size
	= duct::IO::read_arithmetic<std::uint32_t>(
		stream,
		duct::Endian::little
	);

	std::uint32_t const type
	= duct::IO::read_arithmetic<std::uint32_t>(
		stream,
		duct::Endian::little
	);

	if (stream.fail()) {
		return false;
	} else {
		m_incoming_size = size;
		m_incoming_type = type;
		return true;
	}
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC read_stage
bool
CmdStreamer::read_stage() {
	auto const command_type = static_cast<Hord::Cmd::Type>(
		m_incoming_type & mask_type_cmd
	);
	auto const stage_type = static_cast<Hord::Cmd::StageType>(
		m_incoming_type & mask_type_stage
	);

	Hord::Cmd::StageUPtr stage_uptr;
	Hord::Cmd::type_info const* const
	type_info = m_driver.get_command_type_info(
		command_type
	);
	if (!type_info) {
		Log::acquire(Log::error)
			<< ONSANG_SCOPE_FQN_STR_LIT
			<< ceformat::write_sentinel<
				s_err_command_type_invalid
			>(
				enum_cast(command_type)
			)
		;
		return false;
	}

	try {
		stage_uptr = type_info->make_stage(stage_type);
		std::istream stream{&m_streambuf_in};
		auto ser = make_input_serializer(stream);
		ser(*stage_uptr);
	} catch (Hord::Error& ex) {
		stage_uptr.reset();
		switch (ex.get_code()) {
		case Hord::ErrorCode::cmd_construct_stage_type_invalid:
			Log::acquire(Log::error)
				<< ONSANG_SCOPE_FQN_STR_LIT
				<< ceformat::write_sentinel<s_err_stage_type_invalid>(
					enum_cast(stage_type),
					enum_cast(command_type)
				)
			;
			return false;

		// IO error or malformed data
		default:
			return false;
		}
	} catch (...) {
		throw;
	}

	std::size_t const remaining = m_streambuf_in.get_remaining();
	if (0u < remaining) {
		Log::acquire(Log::debug)
			<< ONSANG_SCOPE_FQN_STR_LIT
			<< ": "
			<< remaining
			<< " bytes left in buffer after stage deserialization\n"
		;
	}

	{
		std::lock_guard<std::mutex> m_lock{m_stage_buffer_mutex};
		m_stage_buffer.emplace_back(std::move(stage_uptr));
		m_sig_have_stages.store(true);
	}

	return true;
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC chain_read_header
void
CmdStreamer::chain_read_header() {
	m_incoming_size = 0u;
	assert(m_streambuf_in.reset(msg_header_size));
	asio::async_read(
		m_socket,
		asio::buffer(
			m_streambuf_in.get_buffer().data(),
			msg_header_size
		),
		asio::transfer_exactly(msg_header_size),
		[this](
			boost::system::error_code const ec,
			std::size_t /*length*/
		) {
			if (!ec) {
				m_streambuf_in.commit_direct(msg_header_size, false);
				read_header();
				chain_read_stage();
			} else {
				halt();
			}
		}
	);
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC chain_read_stage
void
CmdStreamer::chain_read_stage() {
	assert(0u <  m_incoming_size);
	assert(0u != m_incoming_type);

	assert(m_streambuf_in.reset(m_incoming_size));
	asio::async_read(
		m_socket,
		asio::buffer(
			m_streambuf_in.get_buffer().data(),
			m_incoming_size
		),
		asio::transfer_exactly(m_incoming_size),
		[this](
			boost::system::error_code const ec,
			std::size_t /*length*/
		) {
			if (!ec) {
				m_streambuf_in.commit_direct(m_incoming_size, true);
				read_stage();
				chain_read_header();
			} else {
				halt();
			}
		}
	);
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC chain_flush_output
void
CmdStreamer::chain_flush_output() {
	assert(0u < m_outgoing_size);

	asio::async_write(
		m_socket,
		asio::buffer(
			make_const(m_streambuf_out).get_buffer().data(),
			m_outgoing_size
		),
		asio::transfer_exactly(m_incoming_size),
		[this](
			boost::system::error_code const ec,
			std::size_t /*length*/
		) {
			m_sig_pending_output.store(false);
			m_outgoing_size = 0u;
			if (ec) {
				halt();
			}
		}
	);
}
#undef ONSANG_SCOPE_FUNC


// operations

void
CmdStreamer::halt() noexcept {
	m_socket.get_io_service().stop();
}

#define ONSANG_SCOPE_FUNC run
void
CmdStreamer::run() {
	auto& io_service = m_socket.get_io_service();
	try {
		while (!io_service.stopped()) {
			chain_read_header();
			io_service.run();
		}
	} catch (std::exception& ex) {
		Log::acquire(Log::error)
			<< ONSANG_SCOPE_FQN_STR_LIT
			<< ": caught exception from io_service::run(): "
			<< ex.what()
			<< "\n"
		;
		throw;
	}
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC context_input
bool
CmdStreamer::context_input(
	Hord::System::Context& context
) {
	if (!m_sig_have_stages.load()) {
		return false;
	}

	{
		std::lock_guard<std::mutex> m_lock{m_stage_buffer_mutex};
		for (auto& stage_uptr : m_stage_buffer) {
			context.push_input(std::move(stage_uptr));
		}
		m_stage_buffer.clear();
		m_sig_have_stages.store(false);
	}

	return true;
}
#undef ONSANG_SCOPE_FUNC

// TODO: What should the exception specifications for this be?
// Does it leave it up to the callee? Seems to be the intention.
#define ONSANG_SCOPE_FUNC context_output
bool
CmdStreamer::context_output(
	Hord::System::Context& context
) {
	if (
		0 == context.get_output().size() ||
		m_sig_pending_output.load()
	) {
		return false;
	}

	m_streambuf_out.reset(
		msg_header_size + (m_streambuf_out.get_max_size() >> 2)
	);
	std::ostream stream{&m_streambuf_out};
	auto ser = make_output_serializer(stream);

	// Seek past header to reserve it; we need the size of the
	// stage payload
	stream.seekp(msg_header_size);

	// data
	auto& stage = *context.get_output().front().stage;
	ser(stage);

	// Can't use buffer.size() later because it might've grown
	// beyond our write amount
	m_outgoing_size = m_streambuf_out.get_sequence_size();

	// header
	stream.seekp(0);
	std::size_t const h_size = m_outgoing_size - msg_header_size;
	assert(std::numeric_limits<std::uint32_t>::max() >= h_size);
	ser(static_cast<std::uint32_t>(h_size));

	std::uint32_t const h_type
		= enum_cast(stage.get_stage_type())
		| enum_cast(stage.get_command_type())
	;
	ser(h_type);

	if (stream.fail()) {
		return false;
	}
	m_streambuf_out.commit();

	context.get_output().pop_front();
	m_sig_pending_output.store(true);

	m_socket.get_io_service().post(
		[this]() {
			chain_flush_output();
		}
	);

	return true;
}
#undef ONSANG_SCOPE_FUNC

#undef ONSANG_SCOPE_CLASS // Onsang::Net::CmdStreamer

} // namespace Net
} // namespace Onsang
