/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/utility.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedContainer.hpp>
#include <Onsang/UI/ObjectView.hpp>
#include <Onsang/App.hpp>

#include <Hord/Object/Unit.hpp>
#include <Hord/Object/Ops.hpp>
#include <Hord/Cmd/Datastore.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

String
SessionView::view_title() noexcept {
	return "S:" + m_session.name();
}

String
SessionView::view_description() noexcept {
	return "session view: " + m_session.name();
}

void
SessionView::notify_command(
	UI::View* const parent_view,
	Hord::Cmd::UnitBase const& command,
	Hord::Cmd::TypeInfo const& type_info
) noexcept {
	base::notify_command(parent_view, command, type_info);
	if (
		command.bad() &&
		App::instance.m_session == &m_session
	) {
		App::instance.m_ui.csline->set_error(command.message());
	}
}

void
SessionView::add_object_view(
	Hord::Object::ID const object_id,
	unsigned index
) {
	auto* const object = m_session.datastore().find_ptr(object_id);
	if (!object) {
		return;
	}
	for (auto const& tab : m_tabs) {
		auto const object_view = std::static_pointer_cast<UI::ObjectView>(tab.widget);
		if (object_view && object_view->m_object.id() == object->id()) {
			return;
		}
	}
	{// Ensure data props are loaded
		Hord::Cmd::Datastore::Load cmd{m_session};
		if (!cmd(object_id, Hord::IO::PropTypeBit::data)) {
			Log::acquire(Log::error)
				<< "add_object_view: failed to load data props for "
				<< *object << '\n'
			;
			return;
		}
	}
	auto object_view = UI::make_object_view(get_root_weak(), m_session, *object);
	index = insert(object_view->view_title(), object_view, index);
	set_sub_view(index);
	// Show description if the sub view wasn't changed
	// (e.g., when the tabbed view is empty)
	App::instance.m_ui.csline->set_description(object_view->view_description());
}

} // namespace UI
} // namespace Onsang
