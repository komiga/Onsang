/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/ObjectView.hpp>
#include <Onsang/UI/PropViews.hpp>

#include <Hord/Object/Defs.hpp>
#include <Hord/Object/Unit.hpp>
#include <Hord/Object/Ops.hpp>
#include <Hord/Cmd/Object.hpp>

#include <ceformat/print.hpp>

#include <Onsang/detail/gr_ceformat.hpp>

namespace Onsang {
namespace UI {

namespace {
	ONSANG_DEF_FMT(s_fmt_object, "%s");
} // anonymous namespace

String
ObjectView::view_title() noexcept {
	return "O:" + m_object.get_slug();
}

String
ObjectView::view_description() noexcept {
	return "object view: " + ceformat::print<s_fmt_object>(m_object);
}

void
ObjectView::notify_command(
	UI::View* const parent_view,
	Hord::Cmd::UnitBase const& command,
	Hord::Cmd::type_info const& type_info
) noexcept {
	base::notify_command(parent_view, command, type_info);
	switch (type_info.id) {
	case Hord::Cmd::Object::SetSlug::COMMAND_ID:
		auto const& c = static_cast<Hord::Cmd::Object::SetSlug const&>(command);
		if (c.ok_action() && c.get_object_id() == m_object.get_id() && parent_view) {
			parent_view->sub_view_title_changed(UI::Widget::Base::get_index());
		}
		break;
	}
}

void
ObjectView::add_prop_view(
	UI::PropView::SPtr prop_view,
	unsigned const index
) {
	insert(prop_view->view_title(), std::move(prop_view), index);
}

UI::ObjectView::SPtr
make_object_view(
	UI::RootWPtr&& root,
	System::Session& session,
	Hord::Object::Unit& object
) {
	auto object_view = UI::ObjectView::make(root, session, object);
	UI::add_base_prop_view(*object_view);
	UI::add_data_prop_view(*object_view);
	object_view->set_sub_view(object_view->sub_view_index_last());
	return object_view;
}

} // namespace UI
} // namespace Onsang
