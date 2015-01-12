/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/utility.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedContainer.hpp>
#include <Onsang/UI/ObjectView.hpp>

#include <Hord/Object/Unit.hpp>
#include <Hord/Object/Ops.hpp>
#include <Hord/Cmd/Datastore.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

void
SessionView::add_object_view(
	Hord::Object::ID const object_id,
	unsigned const index
) {
	auto* const object = m_session.get_datastore().find_ptr(object_id);
	if (!object) {
		return;
	}
	for (auto const& tab : m_container->m_tabs) {
		auto const object_view = std::static_pointer_cast<UI::ObjectView>(tab.widget);
		if (object_view && object_view->m_object.get_id() == object->get_id()) {
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
	m_container->insert(
		object->get_slug(),
		UI::make_object_view(get_root_weak(), m_session, *object),
		index
	);
}

void
SessionView::update_view_title(
	Hord::Object::ID const object_id
) {
	auto const& tabs = m_container->m_tabs;
	for (unsigned index = 0; index < tabs.size(); ++index) {
		auto const& tab = tabs[index];
		auto const object_view = std::static_pointer_cast<UI::ObjectView>(tab.widget);
		if (object_view && object_view->m_object.get_id() == object_id) {
			m_container->set_title(index, object_view->m_object.get_slug());
			return;
		}
	}
}

} // namespace UI
} // namespace Onsang
