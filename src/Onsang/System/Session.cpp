
#include <Onsang/Log.hpp>
#include <Onsang/System/Session.hpp>

#include <Hord/Cmd/Defs.hpp>
#include <Hord/Cmd/Unit.hpp>

#include <iomanip>

#include <Onsang/detail/gr_core.hpp>

namespace Onsang {
namespace System {

#define ONSANG_SCOPE_CLASS System::Session

#define ONSANG_SCOPE_FUNC notify_exception_impl
void
Session::notify_exception_impl(
	Hord::Cmd::UnitBase const& /*command*/,
	Hord::Cmd::type_info const& type_info,
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
	Hord::Cmd::type_info const& type_info
) noexcept {
	// TODO: Dispatch to observers
	Log::acquire(Log::debug)
		<< "notify_complete: " << std::hex << type_info.id
		<< " '" << type_info.name
		<< "' ok? " << std::boolalpha << command.ok()
		<< " message: " << command.get_message()
		<< '\n'
	;
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC open
void
Session::open() {
	get_datastore().open(m_auto_create);
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC close
void
Session::close() {
	get_datastore().close();
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

