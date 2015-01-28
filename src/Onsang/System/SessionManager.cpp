/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/Log.hpp>
#include <Onsang/System/Defs.hpp>
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

Hord::IO::Datastore::ID
SessionManager::add_session(
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
	Hord::IO::Datastore::TypeInfo const*
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
	auto const& datastore = m_driver.placehold_datastore(
		*datastore_tinfo,
		path
	);
	Log::acquire(Log::debug)
		<< "datastore root path: '"
		<< datastore.root_path()
		<< "'\n"
	;
	m_sessions.emplace(
		datastore.id(),
		System::Session::make(
			m_driver, datastore.id(), name, path, auto_open, auto_create
		)
	);
	return datastore.id();
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC process
void
SessionManager::process() {
	for (auto& pair : m_sessions) {
		pair.second->process();
	}
}
#undef ONSANG_SCOPE_FUNC

#undef ONSANG_SCOPE_CLASS

} // namespace System
} // namespace Onsang
