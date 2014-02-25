
#include <Onsang/asio.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/Client/Unit.hpp>

#include <duct/Args.hpp>

namespace Onsang {
namespace Client {

// class Unit implementation

void
Unit::add_session(
	String /*address*/
) {
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
	duct::Var cmd;
	bool const has_cmd = duct::Args::parse_cmd(argc, argv, opt, cmd);

	// Import requires a node
	opt.morph(duct::VarType::node, false);

	// Leak exceptions to caller
	m_args.import(opt);

	// Set the log file before validation
	auto const& arg_log = m_args.entry("--log");
	if (arg_log.assigned()) {
		if (arg_log.value.is_null()) {
			Log::get_controller().file(false);
		} else {
			Log::get_controller().set_file_path(
				arg_log.value.get_string_ref()
			);
			Log::get_controller().file(true);
		}
	}

	// Check for missing arguments
	auto vinfo = m_args.validate();
	if (!vinfo.valid) {
		Log::acquire(Log::error)
			<< "invalid arguments\n"
		;
		if (vinfo.has_iter) {
			Log::acquire(Log::error)
				<< "missing option: '"
				<< vinfo.iter->first
				<< "'\n"
			;
		}
		return false;
	}

	// TODO: handle config

	if (has_cmd) {
		// TODO: handle command?
	}
	return true;
}

void
Unit::start() {
}

} // namespace Client
} // namespace Onsang
