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
	for (auto const& tab : m_container->get_tabs()) {
		auto const object_view = std::static_pointer_cast<UI::ObjectView>(tab.widget);
		if (object_view && object_view->get_object().get_id() == object->get_id()) {
			return;
		}
	}
	m_container->insert(
		object->get_slug(),
		UI::make_object_view(get_root_weak(), m_session, *object),
		index
	);
}

} // namespace UI
} // namespace Onsang
