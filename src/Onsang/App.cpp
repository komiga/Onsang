/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/aux.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/init.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedContainer.hpp>
#include <Onsang/UI/SessionView.hpp>
#include <Onsang/UI/ObjectView.hpp>
#include <Onsang/App.hpp>

#include <Beard/keys.hpp>
#include <Beard/tty/Defs.hpp>
#include <Beard/tty/Ops.hpp>
#include <Beard/tty/Terminal.hpp>
#include <Beard/tty/TerminalInfo.hpp>
#include <Beard/txt/Defs.hpp>
#include <Beard/ui/Defs.hpp>
#include <Beard/ui/PropertyGroup.hpp>
#include <Beard/ui/PropertyMap.hpp>
#include <Beard/ui/Root.hpp>
#include <Beard/ui/Context.hpp>
#include <Beard/ui/Container.hpp>

#include <Hord/IO/Defs.hpp>
#include <Hord/Object/Ops.hpp>
#include <Hord/Table/Defs.hpp>
#include <Hord/Table/Unit.hpp>
#include <Hord/Cmd/Datastore.hpp>
#include <Hord/Cmd/Table.hpp>

#include <duct/debug.hpp>
#include <duct/Args.hpp>
#include <duct/ScriptParser.hpp>
#include <duct/ScriptWriter.hpp>

#include <exception>
#include <functional>

#include <Onsang/detail/gr_ceformat.hpp>

namespace Onsang {

// class App implementation

namespace {

using AT = Beard::tty::attr_type;
namespace C = Beard::tty::Color;
namespace A = Beard::tty::Attr;

static UI::PropertyGroup const
s_ui_pgroup_default{{
// primary
	{UI::property_primary_fg_inactive, AT{C::term_default}},
	{UI::property_primary_bg_inactive, AT{C::term_default}},
	{UI::property_primary_fg_active, AT{C::term_default}},
	{UI::property_primary_bg_active, AT{C::term_default | A::inverted}},

// content
	{UI::property_content_fg_inactive, AT{C::term_default}},
	{UI::property_content_bg_inactive, AT{C::term_default}},
	{UI::property_content_fg_active, AT{C::term_default}},
	{UI::property_content_bg_active, AT{C::term_default}},
	{UI::property_content_fg_selected, AT{C::term_default | A::bold}},
	{UI::property_content_bg_selected, AT{C::magenta}},

// frame
	{UI::property_frame_enabled, false},
	{UI::property_frame_debug_enabled, false},
	{UI::property_frame_fg_inactive, AT{C::blue}},
	{UI::property_frame_bg_inactive, AT{C::term_default}},
	{UI::property_frame_fg_active, AT{C::blue}},
	{UI::property_frame_bg_active, AT{C::term_default | A::inverted}},

// field
	{UI::property_field_content_underline, false},
}},
s_ui_pgroup_csl{{
// primary
	{UI::property_primary_fg_inactive, AT{C::white}},
	{UI::property_primary_bg_inactive, AT{C::blue}},

// content
	{UI::property_content_fg_inactive, AT{C::white}},
	{UI::property_content_bg_inactive, AT{C::blue}},
}};

} // anonymous namespace

App App::instance{};

#define ONSANG_SCOPE_CLASS App

App::App()
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

namespace {
ONSANG_DEF_FMT_CLASS(
	s_err_command_failed,
	"command %s failed: %s"
);
} // anonymous namespace

void
App::toggle_stdout(
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
App::add_session(
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
App::init(
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
		duct::ScriptWriter writer{
			duct::ScriptWriter::Flags::defaults |
			duct::ScriptWriter::Flags::quote
		};
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
				arg_log.value.as_str()
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
	auto const cfg_path = arg_config.value.as_str();
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
					cfg_log_path.value.string_ref()
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
	auto const& terminfo_path = cfg_term_info.value.string_ref();
	Log::acquire()
		<< "Loading terminfo from '"
		<< terminfo_path
		<< "'\n"
	;
	stream.open(terminfo_path);
	if (stream.is_open()) {
		try {
			m_ui.ctx.terminal().info().deserialize(stream);
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
	m_ui.ctx.terminal().update_cache();

	// Initialize driver
	driver_init(m_driver);

	// Add sessions
	Log::acquire()
		<< "Creating sessions\n"
	;
	aux::vector<ConfigNode::node_pair_type const*>
	session_vec{cfg_sessions_builder.node_count};
	for (auto const& spair : cfg_sessions.node_proxy()) {
		session_vec[spair.second.index()] = &spair;
	}

	for (auto const& spair : session_vec) {
		auto const session = spair->second;
		if (!session.built()) {
			continue;
		}
		auto const& auto_open_entry = session.entry("auto-open");
		auto const& auto_create_entry = session.entry("auto-create");
		add_session(
			session.entry("type").value.string_ref(),
			spair->first,
			session.entry("path").value.string_ref(),
			!auto_open_entry.assigned() || auto_open_entry.value.boolean(),
			!auto_create_entry.assigned() || auto_create_entry.value.boolean()
		);
	}
	return true;
}

void
App::set_session(
	System::Session* const session
) {
	if (m_session == session) {
		return;
	}
	m_ui.viewc->clear();
	m_session = session;
	if (m_session && m_session->is_open()) {
		m_ui.viewc->push_back(m_session->view());
		m_ui.csline->set_description("switched session: " + m_session->name());
	} else {
		m_ui.csline->set_description("switched session: none");
	}
}

void
App::init_session(
	System::Session& session
) {
	if (session.is_open()) {
		return;
	}

	Log::acquire()
		<< "Initializing session: "
		<< session.name()
		<< '\n'
	;
	try {
		session.open(m_ui.ctx.root());
		auto cmd = Hord::Cmd::Datastore::Init{session};
		if (!cmd(Hord::IO::PropTypeBit::base)) {
			ONSANG_THROW_FMT(
				ErrorCode::command_failed,
				s_err_command_failed,
				cmd.command_name(),
				cmd.message()
			);
		}
		if (!m_session) {
			set_session(&session);
		}
	} catch (...) {
		m_ui.csline->set_error(
			"Failed to initialize session: " + session.name()
		);
		Log::acquire(Log::error)
			<< "Failed to initialize session '"
			<< session.name()
			<< "':\n"
		;
		Log::report_error_ptr(std::current_exception());
	}
}

void
App::close_session(
	System::Session& session
) {
	if (!session.is_open()) {
		return;
	}

	Log::acquire()
		<< "Closing session: "
		<< session.name()
		<< '\n'
	;
	try {
		auto cmd = Hord::Cmd::Datastore::Store{session};
		if (!cmd()) {
			ONSANG_THROW_FMT(
				ErrorCode::command_failed,
				s_err_command_failed,
				cmd.command_name(),
				cmd.message()
			);
		}
		Log::acquire()
			<< "Stored "
			<< cmd.num_objects_stored() << " objects and "
			<< cmd.num_props_stored() << " props\n"
		;
		if (&session == m_session) {
			set_session(nullptr);
		}
		session.close();
	} catch (...) {
		m_ui.csline->set_error(
			"Failed to close session: " + session.name()
		);
		Log::acquire(Log::error)
			<< "Failed to close session '"
			<< session.name()
			<< "':\n"
		;
		Log::report_error_ptr(std::current_exception());
	}
}

static Beard::KeyInputMatch const
s_kim_c{Beard::KeyMod::ctrl, Beard::KeyCode::none, 'c', false},
s_kim_cline{Beard::KeyMod::none, Beard::KeyCode::none, ':', false};

static char const* const
s_prop_view_switch_keys{"!@#$%%^&*()"};

void
App::ui_event_filter(
	UI::Event const& event
) {
	if (UI::EventType::key_input != event.type) {
		return;
	}

	if (Beard::key_input_match(event.key_input, s_kim_c)) {
		m_running = false;
	} else if (Beard::key_input_match(event.key_input, s_kim_cline)) {
		m_ui.csline->prompt_command();
	} else if (m_session) {
		auto& session_view = *m_session->view();
		auto const cp = event.key_input.cp;
		if (
			event.key_input.mod == Beard::KeyMod::esc
		) {
			auto const sub_view = session_view.sub_view();
			if ('0' <= cp && cp <= '9') {
				session_view.set_sub_view((cp - '0') - 1);
			} else if (sub_view) {
				auto const* it = s_prop_view_switch_keys;
				while (*it != '\0' && unsigned_cast(*it) != cp) {
					++it;
				}
				if (*it != '\0') {
					sub_view->set_sub_view(it - s_prop_view_switch_keys);
				}
			}
		} else if (cp == 'n') {
			Hord::Object::ID const id{0x2a4c479c};
			m_session->view()->add_object_view(id);
		} else if (cp == 'c') {
			session_view.close_sub_view();
		}
	}
}

void
App::start_ui() {
	m_ui.ctx.open(Beard::tty::this_path(), true);
	m_ui.ctx.set_property_map({
		{
			{UI::group_default, s_ui_pgroup_default},
			{UI::group_csl, s_ui_pgroup_csl},
		},
		false
	});

	auto const root = UI::Root::make(m_ui.ctx, UI::Axis::vertical);
	m_ui.ctx.set_root(root);
	m_ui.viewc = UI::Container::make(root, UI::Axis::vertical);
	root->push_back(m_ui.viewc);

	m_ui.csline = UI::CommandStatusLine::make(root);
	m_ui.csline->geometry().set_sizing(
		UI::Axis::horizontal,
		UI::Axis::horizontal
	);
	root->push_back(m_ui.csline);
}

void
App::start() try {
	// The terminal will get all screwy if we don't disable stdout
	toggle_stdout(false);

	Log::acquire()
		<< "Opening UI context\n"
	;
	start_ui();

	Log::acquire()
		<< "Initializing sessions\n"
	;
	for (auto& pair : m_session_manager) {
		auto& session = pair.second;
		if (session->auto_open()) {
			init_session(*session);
		}
	}

	// Event loop
	m_ui.ctx.render(true);
	m_running = true;
	while (m_running) {
		if (!m_ui.ctx.update(20u)) {
			ui_event_filter(m_ui.ctx.last_event());
		}
		// TODO: Process data from sessions, handle global hotkeys
		m_session_manager.process();
	}

	set_session(nullptr);
	for (auto& pair : m_session_manager) {
		auto& session = pair.second;
		close_session(*session);
	}

	m_ui.viewc->clear();
	m_ui.ctx.close();

	toggle_stdout(true);
} catch (...) {
	// TODO: Terminate UI?
	toggle_stdout(true);
	throw;
}

#undef ONSANG_SCOPE_CLASS

} // namespace Onsang
