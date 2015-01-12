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

void
Session::root_object_changed(
	Hord::Object::ID const object_id
) noexcept {
	auto const* const object = get_datastore().find_ptr(object_id);
	DUCT_ASSERTE(object != nullptr);
	auto const it = m_root_objects.find(object_id);
	if (it != m_root_objects.cend()) {
		if (object->get_parent() != Hord::Object::ID_NULL) {
			m_root_objects.erase(it);
		}
	} else if (object->get_parent() == Hord::Object::ID_NULL) {
		m_root_objects.emplace(object_id);
	}
}

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
		<< "notify_complete: "
		<< std::hex << type_info.id
		<< ' ' << type_info.name
		<< ", ok: " << (command.ok() ? "yes" : "no")
		<< ", message: \"" << command.get_message() << '\"'
		<< '\n'
	;

	if (!command.ok()) {
		return;
	}
	switch (type_info.id) {
	case Hord::Cmd::Datastore::Init::COMMAND_ID:
		// Update root objects
		m_root_objects.clear();
		for (auto const& pair : get_datastore().get_objects()) {
			if (pair.second->get_parent() == Hord::Object::ID_NULL) {
				m_root_objects.emplace(pair.second->get_id());
				Log::acquire(Log::debug)
					<< "root object: "
					<< *pair.second
					<< '\n'
				;
			}
		}
		break;

	case Hord::Cmd::Object::SetParent::COMMAND_ID:
		root_object_changed(
			static_cast<Hord::Cmd::Object::SetParent const&>(command).get_object_id()
		);
		break;

	case Hord::Cmd::Object::SetSlug::COMMAND_ID:
		if (m_view) {
			m_view->update_view_title(
				static_cast<Hord::Cmd::Object::SetSlug const&>(command).get_object_id()
			);
		}
		break;
	}
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC open
void
Session::open(
	UI::RootWPtr root
) try {
	get_datastore().open(m_auto_create);
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
	get_datastore().close();
	m_root_objects.clear();
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

