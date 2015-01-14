/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/config.hpp>
#include <Onsang/String.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/FieldDescriber.hpp>
#include <Onsang/App.hpp>

#include <utility>

namespace Onsang {
namespace UI {

bool
FieldDescriber::operator()(
	UI::Widget::SPtr widget,
	UI::Event const& event
) const noexcept {
	if (event.type == UI::EventType::focus_changed && widget->is_focused()) {
		App::instance.m_ui.csline->set_description(text);
	}
	return false;
}

void
bind_field_describer(
	UI::Widget::SPtr const& widget,
	String name
) {
	widget->signal_event_filter.bind(UI::FieldDescriber{std::move(name)});
}

} // namespace UI
} // namespace Onsang
