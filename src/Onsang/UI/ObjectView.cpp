/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/ObjectView.hpp>
#include <Onsang/UI/PropView.hpp>

#include <Hord/Object/Defs.hpp>
#include <Hord/Object/Unit.hpp>

namespace Onsang {
namespace UI {

UI::ObjectView::SPtr
make_object_view(
	UI::RootWPtr&& root,
	System::Session& session,
	Hord::Object::Unit& object
) {
	auto object_view = UI::ObjectView::make(root, session, object);
	// TODO: Add props by object type
	UI::add_base_prop_view(object_view);
	return object_view;
}

} // namespace UI
} // namespace Onsang
