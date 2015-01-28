/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/Log.hpp>
#include <Onsang/System/Defs.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/SessionView.hpp>

#include <Hord/Object/Defs.hpp>
#include <Hord/Object/Ops.hpp>
#include <Hord/Cmd/Defs.hpp>
#include <Hord/Cmd/Unit.hpp>
#include <Hord/Cmd/Object.hpp>
#include <Hord/Cmd/Datastore.hpp>

#include <duct/debug.hpp>

#include <iomanip>

#include <Onsang/detail/gr_core.hpp>

namespace Onsang {
namespace System {

#define ONSANG_SCOPE_CLASS System::Session

#define ONSANG_SCOPE_FUNC notify_exception_impl
void
Session::notify_exception_impl(
	Hord::Cmd::UnitBase const& /*command*/,
	Hord::Cmd::TypeInfo const& type_info,
	std::exception_ptr eptr
) noexcept {
	Log::acquire(Log::error)
		<< "exception caught in " << type_info.name << ": \n"
	;
	Log::report_error_ptr(std::move(eptr));
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC notify_complete_impl
void
Session::notify_complete_impl(
	Hord::Cmd::UnitBase const& command,
	Hord::Cmd::TypeInfo const& type_info
) noexcept {
	static const char* const
	result_names[]{
		"success",
		"success_no_action",
		"error",
	};

	Log::acquire(command.bad() ? Log::error : Log::debug)
		<< "notify_complete: "
		<< std::hex << type_info.id
		<< ' ' << type_info.name
		<< ", result: " << result_names[enum_cast(command.result())]
		<< ", message: \"" << command.message() << '\"'
		<< '\n'
	;

	// Notify views of failed or mutative command execution
	if (m_view && (command.bad() || command.ok_action())) {
		m_view->notify_command(nullptr, command, type_info);
	}
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC open
void
Session::open(
	UI::RootWPtr root
) try {
	datastore().open(m_auto_create);
	m_view.reset();
	m_view = UI::SessionView::make(std::move(root), *this);
} catch (...) {
	throw;
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC close
void
Session::close() {
	m_view.reset();
	datastore().close();
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC process
void
Session::process() {
	// TODO: Update views
}
#undef ONSANG_SCOPE_FUNC

#undef ONSANG_SCOPE_CLASS

} // namespace System
} // namespace Onsang

