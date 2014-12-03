
#include <Onsang/asio.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/init.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedContainer.hpp>
#include <Onsang/UI/ObjectView.hpp>
#include <Onsang/UI/BasicPropView.hpp>
#include <Onsang/Client/Unit.hpp>

#include <Beard/keys.hpp>
#include <Beard/tty/Defs.hpp>
#include <Beard/tty/Ops.hpp>
#include <Beard/tty/Terminal.hpp>
#include <Beard/tty/TerminalInfo.hpp>
#include <Beard/txt/Defs.hpp>
#include <Beard/ui/Defs.hpp>
#include <Beard/ui/Root.hpp>
#include <Beard/ui/Context.hpp>
#include <Beard/ui/Container.hpp>
#include <Beard/ui/Field.hpp>

#include <Hord/IO/Defs.hpp>
#include <Hord/Cmd/Datastore.hpp>

#include <duct/debug.hpp>
#include <duct/Args.hpp>
#include <duct/ScriptParser.hpp>
#include <duct/ScriptWriter.hpp>

#include <exception>
#include <functional>

#include <Onsang/detail/gr_ceformat.hpp>

namespace Onsang {
namespace Client {

// class Unit implementation

#define ONSANG_SCOPE_CLASS Client::Unit

namespace {
ONSANG_DEF_FMT_CLASS(
	s_err_command_failed,
	"command %s failed: %s"
);
} // anonymous namespace

void
Unit::toggle_stdout(
	bool const enable
) {
	if (m_flags.test(Flags::no_stdout)) {
		return;
	}

	auto& log_controller = Log::get_controller();
	if (enable == log_controller.stdout_enabled()) {
		return;
	}
	if (enable) {
		log_controller.stdout(true);
		Log::acquire()
			<< "Enabled stdout\n"
		;
	} else {
		Log::acquire()
			<< "Disabling stdout\n"
		;
		log_controller.stdout(false);
	}
}

bool
Unit::add_session(
	String const& type,
	String const& name,
	String const& path,
	bool const auto_open,
	bool const auto_create
) {
	Log::acquire(Log::debug)
		<< "Adding session '"
		<< name
		<< "': '"
		<< type
		<< "' @ '"
		<< path
		<< "'\n"
	;
	try {
		// TODO
		m_session_manager.add_session(
			type, name, path, auto_open, auto_create
		);
		return true;
	} catch (...) {
		Log::acquire(Log::error)
			<< "failed to add session: '"
			<< name
			<< "'\n"
		;
		Log::report_error_ptr(std::current_exception());
		return false;
	}
}

bool
Unit::init(
	signed const argc,
	char* argv[]
) {
	auto& log_controller = Log::get_controller();
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

	// Disable stdout before validation
	auto const& arg_no_stdout = m_args.entry("--no-stdout");
	if (arg_no_stdout.assigned()) {
		m_flags.enable(Flags::no_stdout);
		log_controller.stdout(false);
	}

	// Set the log file before validation
	auto const& arg_log = m_args.entry("--log");
	if (arg_log.assigned()) {
		if (arg_log.value.is_null()) {
			log_controller.file(false);
		} else {
			log_controller.set_file_path(
				arg_log.value.get_as_str()
			);
			log_controller.file(true);
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
		m_flags.enable(Flags::no_auto_open);
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
		} catch (...) {
			Log::acquire(Log::error)
				<< "Failed to parse config file '"
				<< cfg_path
				<< "':\n"
			;
			Log::report_error_ptr(std::current_exception());
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
				log_controller.file(false);
			} else {
				log_controller.set_file_path(
					cfg_log_path.value.get_string_ref()
				);
				log_controller.file(true);
				Log::acquire()
					<< "Log note: using config from '"
					<< cfg_path
					<< "'\n"
				;
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
			Log::report_error(err);
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

	// Update terminal cap cache
	m_ui_ctx.get_terminal().update_cache();

	// Initialize driver
	driver_init(m_driver);

	// Add sessions
	Log::acquire()
		<< "Creating sessions\n"
	;
	aux::vector<ConfigNode::node_pair_type const*>
	session_vec{cfg_sessions_builder.node_count};
	for (auto const& spair : cfg_sessions.node_proxy()) {
		session_vec[spair.second.get_index()] = &spair;
	}

	for (auto const& spair : session_vec) {
		auto const session = spair->second;
		if (!session.built()) {
			continue;
		}
		auto const& auto_open_entry = session.entry("auto-open");
		auto const& auto_create_entry = session.entry("auto-create");
		add_session(
			session.entry("type").value.get_string_ref(),
			spair->first,
			session.entry("path").value.get_string_ref(),
			!auto_open_entry.assigned() || auto_open_entry.value.get_bool(),
			!auto_create_entry.assigned() || auto_create_entry.value.get_bool()
		);
	}
	return true;
}

void
Unit::init_session(
	System::Session& session
) {
	if (session.is_open()) {
		return;
	}

	Log::acquire()
		<< "Initializing session: "
		<< session.get_name()
		<< '\n'
	;
	try {
		session.open();
		auto cmd = Hord::Cmd::Datastore::Init{session};
		if (!cmd(Hord::IO::PropTypeBit::base)) {
			// TODO: set status line
			ONSANG_THROW_FMT(
				ErrorCode::command_failed,
				s_err_command_failed,
				cmd.command_name(),
				cmd.get_message()
			);
		}
	} catch (...) {
		Log::acquire(Log::error)
			<< "Failed to initialize session '"
			<< session.get_name()
			<< "':\n"
		;
		Log::report_error_ptr(std::current_exception());
	}
}

void
Unit::close_session(
	System::Session& session
) {
	if (!session.is_open()) {
		return;
	}

	Log::acquire()
		<< "Closing session: "
		<< session.get_name()
		<< '\n'
	;
	try {
		auto cmd = Hord::Cmd::Datastore::StoreAll{session};
		if (!cmd()) {
			ONSANG_THROW_FMT(
				ErrorCode::command_failed,
				s_err_command_failed,
				cmd.command_name(),
				cmd.get_message()
			);
		}
		Log::acquire()
			<< "Stored "
			<< cmd.num_objects_stored() << " objects and "
			<< cmd.num_props_stored() << " props\n"
		;
		session.close();
	} catch (...) {
		Log::acquire(Log::error)
			<< "Failed to close session '"
			<< session.get_name()
			<< "':\n"
		;
		Log::report_error_ptr(std::current_exception());
	}
}

static Beard::KeyInputMatch const
s_kim_c{Beard::KeyMod::ctrl, Beard::KeyCode::none, 'c', false},
s_kim_cline{Beard::KeyMod::none, Beard::KeyCode::none, ':', false};

void
Unit::ui_event_filter(
	UI::Event const& event
) {
	if (UI::EventType::key_input != event.type) {
		return;
	}

	if (Beard::key_input_match(event.key_input, s_kim_c)) {
		m_running = false;
	} else if (Beard::key_input_match(event.key_input, s_kim_cline)) {
		m_ui.sline->set_visible(false);
		m_ui.cline->set_visible(true);
		m_ui.cline->set_input_control(true);
		m_ui_ctx.get_root()->set_focus(m_ui.cline);
	} else {
		switch (event.key_input.cp) {
		case '1': m_ui.viewc->prev_tab(); break;
		case '2': m_ui.viewc->next_tab(); break;
		case '!': m_ui.viewc->move_left(m_ui.viewc->get_tab()); break;
		case '@': m_ui.viewc->move_right(m_ui.viewc->get_tab()); break;
		case '3': m_ui.viewc->move_tab(m_ui.viewc->get_tab(), 0, true); break;

		case 'n': {
			if (m_session_manager.cbegin() != m_session_manager.cend()) {
				auto& session = *m_session_manager.begin();
				auto* const object = session.get_datastore().find_ptr(
					Hord::Object::ID{1}
				);
				DUCT_ASSERTE(object);
				auto view = UI::ObjectView::make(
					m_ui_ctx.get_root(),
					session,
					*object
				);
				UI::add_basic_prop_view(view);
				m_ui.viewc->push_back(object->get_slug(), std::move(view));
			}
		} break;

		case 'c':
			m_ui.viewc->remove_current();
			break;
		}
	}
}

void
Unit::start_ui() {
	using namespace std::placeholders;

	m_ui_ctx.open(Beard::tty::this_path(), true);
	auto& pmap = m_ui_ctx.get_property_map().find(UI::group_default)->second;
	pmap.find(UI::property_field_content_underline)->second.set_boolean(false);
	pmap.find(UI::property_frame_debug_enabled)->second.set_boolean(false);

	auto root = UI::Root::make(m_ui_ctx, UI::Axis::vertical);
	m_ui_ctx.set_root(root);

	m_ui.viewc = UI::TabbedContainer::make(root);
	m_ui.viewc->get_geometry().set_sizing(
		UI::Axis::both,
		UI::Axis::both
	);

	// TODO: Custom widget: sline + cline
	m_ui.sline = UI::Label::make(root, "(status)");
	m_ui.sline->get_geometry().set_sizing(
		UI::Axis::horizontal,
		UI::Axis::horizontal
	);

	m_ui.cline = UI::Field::make(root, "");
	m_ui.cline->get_geometry().set_sizing(
		UI::Axis::horizontal,
		UI::Axis::horizontal
	);
	m_ui.cline->set_visible(false);
	m_ui.cline->signal_control_changed.bind([this](
		aux::shared_ptr<UI::Field>,
		bool const has_control
	) {
		String command;
		if (!has_control) {
			m_ui.sline->set_visible(true);
			m_ui.cline->set_visible(false);
			m_ui_ctx.get_root()->clear_focus();
			command = m_ui.cline->get_text();
			m_ui.cline->set_text("");
		}
		if ("q" == command || "quit" == command) {
			m_running = false;
		}
	});

	root->push_back(m_ui.viewc);
	root->push_back(m_ui.sline);
	root->push_back(m_ui.cline);
}

void
Unit::start() try {
	// The terminal will get all screwy if we don't disable stdout
	toggle_stdout(false);


	Log::acquire()
		<< "Opening UI context\n"
	;
	start_ui();

	Log::acquire()
		<< "Initializing sessions\n"
	;
	for (auto& session : m_session_manager) {
		if (session.get_auto_open()) {
			init_session(session);
		}
	}

	// Event loop
	m_ui_ctx.render(true);
	m_running = true;
	while (m_running) {
		if (!m_ui_ctx.update(20u)) {
			ui_event_filter(m_ui_ctx.get_last_event());
		}
		// TODO: Process data from sessions, handle global hotkeys
		m_session_manager.process();
	}
	m_ui_ctx.close();

	for (auto& session : m_session_manager) {
		close_session(session);
	}

	toggle_stdout(true);
} catch (...) {
	// TODO: Terminate UI?
	toggle_stdout(true);
	throw;
}

#undef ONSANG_SCOPE_CLASS

} // namespace Client
} // namespace Onsang
