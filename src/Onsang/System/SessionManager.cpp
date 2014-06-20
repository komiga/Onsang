
#include <Onsang/Log.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/System/SessionManager.hpp>
#include <Onsang/IO/FlatDatastore.hpp>

#include <Hord/System/Context.hpp>
#include <Hord/IO/Datastore.hpp>

#include <utility>

#include <Onsang/detail/gr_ceformat.hpp>

namespace Onsang {
namespace System {

#define ONSANG_SCOPE_CLASS System::SessionManager

#define ONSANG_SCOPE_FUNC add_session
namespace {
ONSANG_DEF_FMT_FQN(
	s_err_unrecognized_session_type,
	"unrecognized session type: '%s'"
);
} // anonymous namespace

Hord::Hive::ID
SessionManager::add_session(
	Hord::Hive::Type const hive_type,
	String const& type,
	String const& name,
	String const& path,
	bool const auto_open,
	bool const auto_create
) {
	/**
	Types:

	- flat: IO::FlatDatastore
	*/

	Hord::IO::Datastore::type_info const*
	datastore_tinfo = nullptr;
	if ("flat" == type) {
		datastore_tinfo = &IO::FlatDatastore::s_type_info;
	} else {
		ONSANG_THROW_FMT(
			ErrorCode::session_type_unrecognized,
			s_err_unrecognized_session_type,
			type
		);
	}
	Hord::Hive::ID const
	id{
		m_driver.placehold_hive(
			hive_type,
			*datastore_tinfo,
			path
		).hive->get_id_bare()
	};
	Log::acquire(Log::debug)
		<< "datastore root path: '"
		<< m_driver.find_hive(id)->second.datastore->get_root_path()
		<< "'\n"
	;
	Hord::System::Context context{m_driver, id};
	m_sessions.emplace_back(System::Session{
		std::move(context), name, path, auto_open, auto_create
	});
	return id;
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC process
void
SessionManager::process() {
	for (auto& session : m_sessions) {
		session.process();
	}
}
#undef ONSANG_SCOPE_FUNC

#undef ONSANG_SCOPE_CLASS

} // namespace System
} // namespace Onsang
