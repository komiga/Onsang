/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/utility.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/CommandStatusLine.hpp>

#include <Beard/txt/Defs.hpp>
#include <Beard/tty/Terminal.hpp>
#include <Beard/ui/Defs.hpp>

namespace Onsang {
namespace UI {

void
CommandStatusLine::set_input_control_impl(
	bool const enabled
) noexcept {
	base::set_input_control_impl(enabled);
	if (!has_input_control()) {
		auto prev_focus = m_prev_focus.lock();
		root()->set_focus(prev_focus);
		m_prev_focus.reset();
	}
	m_field.input_control_changed(*this);
	enqueue_actions(
		ui::UpdateActions::render |
		ui::UpdateActions::flag_noclear
	);
}

void
CommandStatusLine::reflow_impl() noexcept {
	base::reflow_impl();
	if (has_input_control()) {
		m_field.reflow_into(geometry().area());
	}
}

bool
CommandStatusLine::handle_event_impl(
	UI::Event const& event
) noexcept {
	if (event.type != UI::EventType::key_input) {
		return false;
	}
	switch (event.type) {
	case UI::EventType::key_input:
		if (!has_input_control()) {
			break;
		}
		if (event.key_input.code == KeyCode::enter) {
			if (!m_field.m_text_tree.empty()) {
				// TODO
				//String const string_value = m_field.m_cursor.node().to_string();
			}
			m_field.m_cursor.clear();
			set_input_control(false);
			return true;
		} else if (event.key_input.code == KeyCode::esc) {
			m_field.m_cursor.clear();
			set_input_control(false);
			return true;
		} else {
			// TODO: Command history
			if (m_field.input(event.key_input)) {
				enqueue_actions(
					ui::UpdateActions::render |
					ui::UpdateActions::flag_noclear
				);
			}
			return true;
		}
		break;

	default:
		break;
	}
	return false;
}

void
CommandStatusLine::render_impl(
	UI::Widget::RenderData& rd
) noexcept {
	auto const& frame = geometry().frame();
	if (!has_input_control()) {
		// render message & location
		tty::attr_type const
			primary_fg = rd.attr(ui::property_primary_fg_inactive),
			primary_bg = rd.attr(ui::property_primary_bg_inactive),
			content_fg = rd.attr(ui::property_content_fg_inactive),
			content_bg = rd.attr(ui::property_content_bg_inactive)
		;
		auto const cell_clear = tty::make_cell(' ', primary_fg, primary_bg);
		rd.terminal.put_line(frame.pos, frame.size.width, Axis::horizontal, cell_clear);
		geom_value_type const location_pos = max_ce(
			0, frame.size.width - static_cast<signed>(m_location.size())
		);
		rd.terminal.put_sequence(
			frame.pos.x + location_pos,
			frame.pos.y,
			{m_location},
			frame.size.width - location_pos,
			content_fg,
			content_bg
		);
		rd.terminal.put_sequence(
			frame.pos.x,
			frame.pos.y,
			{m_message},
			frame.size.width,
			(m_message_type == MessageType::error)
			? (tty::Color::white | tty::Attr::bold)
			: content_fg,
			(m_message_type == MessageType::error)
			? tty::Color::red
			: content_bg
		);
	} else {
		// render field
		rd.update_group(UI::group_field);
		m_field.render(rd, true);
	}
}

void
CommandStatusLine::set_message(
	UI::CommandStatusLine::MessageType type,
	String&& text
) {
	m_message_type = type;
	m_message = std::move(text);
	enqueue_actions(
		ui::UpdateActions::render |
		ui::UpdateActions::flag_noclear
	);
}

void
CommandStatusLine::set_location(
	String text
) {
	m_location = std::move(text);
	enqueue_actions(
		ui::UpdateActions::render |
		ui::UpdateActions::flag_noclear
	);
}

void
CommandStatusLine::prompt_command() {
	m_field.reflow_into(geometry().area());
	set_input_control(true);
	m_prev_focus = root()->focused_widget();
	root()->set_focus(shared_from_this());
}

} // namespace UI
} // namespace Onsang
