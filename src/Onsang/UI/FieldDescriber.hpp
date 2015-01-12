/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Command line and status line widget.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/String.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/App.hpp>

#include <utility>

namespace Onsang {
namespace UI {

struct FieldDescriber {
	String text;

	FieldDescriber(
		String name
	)
		: text("field: ")
	{
		text.append(std::move(name));
	}

	bool
	operator()(
		UI::Widget::SPtr widget,
		UI::Event const& event
	) const noexcept {
		if (event.type == UI::EventType::focus_changed && widget->is_focused()) {
			App::instance.m_ui.csline->set_description(text);
		}
		return false;
	}
};

} // namespace UI
} // namespace Onsang
