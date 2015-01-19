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
		get_root()->set_focus(prev_focus);
		m_prev_focus.reset();
	}
	m_field.input_control_changed(*this);
	queue_actions(
		ui::UpdateActions::render |
		ui::UpdateActions::flag_noclear
	);
}

void
CommandStatusLine::reflow_impl(
	Rect const& area,
	bool const cache
) noexcept {
	base::reflow_impl(area, cache);
	if (has_input_control()) {
		m_field.reflow(area);
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
				//String const string_value = m_field.m_cursor.get_node().to_string();
			}
			m_field.m_cursor.clear();
			set_input_control(false);
			return true;
		} else if (event.key_input.code == KeyCode::esc) {
			m_field.m_cursor.clear();
			set_input_control(false);
			return true;
		} else {
			bool view_modified = true;
			switch (event.key_input.code) {
			//case KeyCode::up   : /* TODO */ break;
			//case KeyCode::down : /* TODO */ break;
			case KeyCode::left : m_field.m_cursor.col_prev(); break;
			case KeyCode::right: m_field.m_cursor.col_next(); break;
			case KeyCode::home: m_field.m_cursor.col_extent(txt::Extent::head); break;
			case KeyCode::end : m_field.m_cursor.col_extent(txt::Extent::tail); break;
			case KeyCode::del      : m_field.m_cursor.erase(); break;
			case KeyCode::backspace: m_field.m_cursor.erase_before(); break;
			default:
				if (
					event.key_input.cp != codepoint_none &&
					event.key_input.cp != '\t'
				) {
					m_field.m_cursor.insert_step(event.key_input.cp);
				} else {
					view_modified = false;
				}
				break;
			}
			if (view_modified) {
				m_field.update_view();
				queue_actions(
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
	auto const& frame = get_geometry().get_frame();
	if (!has_input_control()) {
		// render message & location
		tty::attr_type const
			primary_fg = rd.get_attr(ui::property_primary_fg_inactive),
			primary_bg = rd.get_attr(ui::property_primary_bg_inactive),
			content_fg = rd.get_attr(ui::property_content_fg_inactive),
			content_bg = rd.get_attr(ui::property_content_bg_inactive)
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
	queue_actions(
		ui::UpdateActions::render |
		ui::UpdateActions::flag_noclear
	);
}

void
CommandStatusLine::set_location(
	String text
) {
	m_location = std::move(text);
	queue_actions(
		ui::UpdateActions::render |
		ui::UpdateActions::flag_noclear
	);
}

void
CommandStatusLine::prompt_command() {
	m_field.reflow(get_geometry().get_area());
	set_input_control(true);
	m_prev_focus = get_root()->get_focus();
	get_root()->set_focus(shared_from_this());
}

} // namespace UI
} // namespace Onsang
