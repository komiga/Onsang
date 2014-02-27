
#include <Onsang/asio.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/Client/Unit.hpp>

#include <duct/debug.hpp>
#include <duct/Args.hpp>
#include <duct/ScriptParser.hpp>
#include <duct/ScriptWriter.hpp>

namespace Onsang {
namespace Client {

// class Unit implementation

void
Unit::add_session(
	String name,
	String type,
	String addr
) {
	Log::acquire(Log::debug)
		<< "Adding session '"
		<< name
		<< "': '"
		<< type
		<< "' @ '"
		<< addr
		<< "'\n"
	;
	/*auto const ds_pair = m_driver.placehold_hive(
		CacheDatastore::s_type_info,
		std::move(address)
	);

	// TODO: Read protocol from address string
	m_sessions.emplace_back(
		Hord::System::Context{
			Hord::System::Context::Type::client,
			m_driver,
			ds_pair.hive.get_id()
		},
		Net::Socket{
			m_io_service,
			Net::StreamProtocol{asio::tcp::v4()}
		}
	);*/
}

bool
Unit::init(
	signed const argc,
	char* argv[]
) {
	duct::Var opt;
	duct::Args::parse_raw(argc, argv, opt);

	// Import requires a node
	opt.morph(duct::VarType::node, false);
	{
		auto log = Log::acquire(Log::debug);
		duct::ScriptWriter writer{enum_combine(
			duct::ScriptWriter::Flags::defaults,
			duct::ScriptWriter::Flags::quote
		)};
		log << "Arguments:\n";
		DUCT_ASSERTE(writer.write(log, opt, true, 1u));
		log << "\n";
	}

	// Leak exceptions to caller
	m_args.import(opt);

	// Set the log file before validation
	auto const& arg_log = m_args.entry("--log");
	if (arg_log.assigned()) {
		if (arg_log.value.is_null()) {
			Log::get_controller().file(false);
		} else {
			Log::get_controller().set_file_path(
				arg_log.value.get_as_str()
			);
			Log::get_controller().file(true);
		}
	}

	// Check for missing arguments
	auto vinfo = m_args.validate();
	if (!vinfo.valid) {
		Log::acquire(Log::error)
			<< "Invalid arguments\n"
		;
		if (vinfo.has_iter) {
			Log::acquire(Log::error)
				<< "Missing option: "
				<< vinfo.iter->first
				<< '\n'
			;
		}
		return false;
	}

	if (m_args.entry("--no-auto").assigned()) {
		m_flags.enable(Flags::no_auto_connect);
	}

	// Load config
	auto const& arg_config = m_args.entry("--config");
	auto const cfg_path = arg_config.value.get_as_str();
	Log::acquire()
		<< "Loading config from '"
		<< cfg_path
		<< "'\n"
	;
	duct::Var cfg_root{duct::VarType::node};
	std::ifstream stream{cfg_path};
	if (stream.is_open()) {
		duct::ScriptParser parser{
			{duct::Encoding::UTF8, duct::Endian::system}
		};
		try {
			parser.process(cfg_root, stream);
			stream.close();
		} catch (std::exception& err) {
			Log::acquire(Log::error)
				<< "Failed to parse config file '"
				<< cfg_path
				<< "':\n"
				<< Log::Pre::current
				<< err.what()
				<< '\n'
			;
			return false;
		}
	} else {
		Log::acquire(Log::error)
			<< "Failed to open config file '"
			<< cfg_path
			<< "'\n"
		;
		return false;
	}

	// Import and validate config
	// Leak import exceptions to caller
	m_config.import(cfg_root);
	auto const& cfg_sessions = m_config.node("sessions");
	auto const& cfg_sessions_builder = cfg_sessions.entry("builder");
	vinfo = m_config.validate();
	if (!vinfo.valid) {
		Log::acquire(Log::error)
			<< "Invalid config\n"
		;
		if (vinfo.has_iter) {
			if (&vinfo.iter->second == &cfg_sessions_builder) {
				Log::acquire(Log::error)
					<< "No sessions supplied\n"
				;
			} else {
				Log::acquire(Log::error)
					<< "Missing required value '"
					<< vinfo.iter->first
					<< "' in node '"
					<< vinfo.node
					<< "'\n"
				;
			}
		} else {
			Log::acquire(Log::error)
				<< "Missing required node '"
				<< vinfo.node
				<< "'\n"
			;
		}
		return false;
	}

	// Use log path in config file unless --log was provided
	if (!arg_log.assigned()) {
		auto const& cfg_log_path = m_config.node("log").entry("path");
		if (cfg_log_path.assigned()) {
			if (cfg_log_path.value.is_null()) {
				Log::get_controller().file(false);
			} else {
				Log::get_controller().set_file_path(
					cfg_log_path.value.get_string_ref()
				);
				Log::get_controller().file(true);
			}
		}
	}

	// Load terminfo
	auto const& cfg_term_info = m_config.node("term").entry("info");
	auto const& terminfo_path = cfg_term_info.value.get_string_ref();
	Log::acquire()
		<< "Loading terminfo from '"
		<< terminfo_path
		<< "'\n"
	;
	stream.open(terminfo_path);
	if (stream.is_open()) {
		try {
			m_ui_ctx.get_terminal().get_info().deserialize(stream);
			stream.close();
		} catch (Beard::Error& err) {
			Log::acquire(Log::error)
				<< "Failed to parse terminfo file '"
				<< terminfo_path
				<< "':\n"
			;
			report_error(err);
			return false;
		}
	} else {
		Log::acquire(Log::error)
			<< "Failed to open terminfo file '"
			<< terminfo_path
			<< "'\n"
		;
		return false;
	}

	// Add sessions
	Log::acquire()
		<< "Creating sessions\n"
	;
	for (auto const& spair : cfg_sessions.node_proxy()) {
		auto const session = spair.second;
		if (!session.built()) {
			continue;
		}
		add_session(
			spair.first,
			session.entry("type").value.get_string_ref(),
			session.entry("addr").value.get_string_ref()
		);
	}

	return true;
}

void
Unit::start() {
}

} // namespace Client
} // namespace Onsang
