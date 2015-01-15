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
	get_root()->get_context().get_terminal().set_caret_visible(
		has_input_control()
	);
	if (has_input_control()) {
		m_field_cursor.col_extent(txt::Extent::head);
		update_field_view();
	} else {
		auto prev_focus = m_prev_focus.lock();
		get_root()->set_focus(prev_focus);
		m_prev_focus.reset();
	}
	queue_actions(enum_combine(
		ui::UpdateActions::render,
		ui::UpdateActions::flag_noclear
	));
}

void
CommandStatusLine::reflow_impl(
	Rect const& area,
	bool const cache
) noexcept {
	base::reflow_impl(area, cache);
	if (has_input_control()) {
		update_field_view();
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
			if (!m_field_text_tree.empty()) {
				// TODO
				//String const string_value = m_field_text_tree.to_string();
			}
			m_field_cursor.clear();
			set_input_control(false);
			return true;
		} else if (event.key_input.code == KeyCode::esc) {
			m_field_cursor.clear();
			set_input_control(false);
			return true;
		} else {
			bool view_modified = true;
			switch (event.key_input.code) {
			//case KeyCode::up   : /* TODO */ break;
			//case KeyCode::down : /* TODO */ break;
			case KeyCode::left : m_field_cursor.col_prev(); break;
			case KeyCode::right: m_field_cursor.col_next(); break;
			case KeyCode::home: m_field_cursor.col_extent(txt::Extent::head); break;
			case KeyCode::end : m_field_cursor.col_extent(txt::Extent::tail); break;
			case KeyCode::del      : m_field_cursor.erase(); break;
			case KeyCode::backspace: m_field_cursor.erase_before(); break;
			default:
				if (
					event.key_input.cp != codepoint_none &&
					event.key_input.cp != '\t'
				) {
					m_field_cursor.insert_step(event.key_input.cp);
				} else {
					view_modified = false;
				}
				break;
			}
			if (view_modified) {
				update_field_view();
				queue_actions(enum_combine(
					ui::UpdateActions::render,
					ui::UpdateActions::flag_noclear
				));
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
		auto const& node = m_field_cursor.get_node();
		rd.update_group(UI::group_field);
		tty::attr_type
			primary_fg = rd.get_attr(ui::property_primary_fg_active),
			primary_bg = rd.get_attr(ui::property_primary_bg_active),
			content_fg = rd.get_attr(ui::property_content_fg_active),
			content_bg = rd.get_attr(ui::property_content_bg_active)
		;

		// primary_fg &= ~tty::Attr::bold;
		auto const cell_prompt = tty::make_cell(':', primary_fg, primary_bg);
		rd.terminal.put_cell(frame.pos.x, frame.pos.y, cell_prompt);
		auto const inner_width = max_ce(geom_value_type{0}, frame.size.width - 1);
		auto const put_count = min_ce(
			static_cast<txt::Cursor::difference_type>(inner_width),
			max_ce(
				txt::Cursor::difference_type{0},
				signed_cast(node.points()) - m_field_view.col()
			)
		);
		rd.terminal.put_sequence(
			frame.pos.x + 1,
			frame.pos.y,
			txt::Sequence{
				&*node.cbegin() + m_field_view.index(),
				unsigned_cast(max_ce(
					txt::Cursor::difference_type{0},
					signed_cast(node.units()) - m_field_view.index()
				))
			},
			put_count, content_fg, content_bg
		);
		auto const cell_clear = tty::make_cell(' ', content_fg, content_bg);
		rd.terminal.put_line(
			{
				static_cast<geom_value_type>(frame.pos.x + 1 + put_count),
				frame.pos.y
			},
			inner_width - put_count, Axis::horizontal, cell_clear
		);
		rd.terminal.set_caret_pos(
			frame.pos.x
			+ min_ce(
				inner_width,
				1 + static_cast<geom_value_type>(
					m_field_cursor.col() - m_field_view.col()
				)
			),
			frame.pos.y
		);
	}
}

void
CommandStatusLine::update_field_view() noexcept {
	auto const& frame = get_geometry().get_frame();
	auto const inner_width = max_ce(0, frame.size.width - 1 - 1);
	if (
		m_field_view.col() > m_field_cursor.col() ||
		m_field_view.col() + inner_width < m_field_cursor.col()
	) {
		m_field_view.col_abs(
			m_field_cursor.col() - (inner_width / 2)
		);
	}
}

void
CommandStatusLine::set_message(
	UI::CommandStatusLine::MessageType type,
	String&& text
) {
	m_message_type = type;
	m_message = std::move(text);
	queue_actions(enum_combine(
		ui::UpdateActions::render,
		ui::UpdateActions::flag_noclear
	));
}

void
CommandStatusLine::set_location(
	String text
) {
	m_location = std::move(text);
	queue_actions(enum_combine(
		ui::UpdateActions::render,
		ui::UpdateActions::flag_noclear
	));
}

void
CommandStatusLine::prompt_command() {
	set_input_control(true);
	m_prev_focus = get_root()->get_focus();
	get_root()->set_focus(shared_from_this());
}

} // namespace UI
} // namespace Onsang
