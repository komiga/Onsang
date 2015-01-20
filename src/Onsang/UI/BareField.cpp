/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/BareField.hpp>

#include <Beard/txt/Tree.hpp>
#include <Beard/txt/Cursor.hpp>
#include <Beard/tty/Defs.hpp>
#include <Beard/tty/Terminal.hpp>
#include <Beard/ui/packing.hpp>

#include <utility>

namespace Onsang {
namespace UI {

void
BareField::update_view() noexcept {
	auto const& frame = m_geom.get_frame();
	auto const inner_width = max_ce(0, frame.size.width - 2 - 1);
	if (
		m_view.col() > m_cursor.col() ||
		m_view.col() + inner_width < m_cursor.col()
	) {
		m_view.col_abs(m_cursor.col() - (inner_width / 2));
	}
}

void
BareField::input_control_changed(
	UI::Widget::Base& widget
) noexcept {
	widget.get_root()->get_context().get_terminal().set_caret_visible(
		widget.has_input_control()
	);
	if (widget.has_input_control()) {
		m_cursor.col_extent(txt::Extent::tail);
		update_view();
	} else {
		m_cursor.col_extent(txt::Extent::head);
		m_view.col_extent(txt::Extent::head);
	}
}

void
BareField::reflow(
	Rect const& area
) noexcept {
	UI::reflow(area, m_geom);
	update_view();
}

void
BareField::render(
	UI::Widget::RenderData& rd,
	bool const active
) noexcept {
	auto const& frame = m_geom.get_frame();
	auto const& node = m_cursor.get_node();

	tty::attr_type const
		primary_fg = rd.get_attr(
			active
			? ui::property_primary_fg_active
			: ui::property_primary_fg_inactive
		),
		primary_bg = rd.get_attr(
			active
			? ui::property_primary_bg_active
			: ui::property_primary_bg_inactive
		),
		content_fg = rd.get_attr(
			active
			? ui::property_content_fg_active
			: ui::property_content_fg_inactive
		),
		content_bg = rd.get_attr(
			active
			? ui::property_content_bg_active
			: ui::property_content_bg_inactive
		)
	;
	tty::Cell cell_side = tty::make_cell('[', primary_fg, primary_bg);
	rd.terminal.put_cell(frame.pos.x, frame.pos.y, cell_side);
	auto const inner_width = max_ce(geom_value_type{0}, frame.size.width - 2);
	auto const put_count = min_ce(
		static_cast<txt::Cursor::difference_type>(inner_width),
		max_ce(
			txt::Cursor::difference_type{0},
			signed_cast(node.points()) - m_view.col()
		)
	);
	rd.terminal.put_sequence(
		frame.pos.x + 1,
		frame.pos.y,
		txt::Sequence{
			&*node.cbegin() + m_view.index(),
			unsigned_cast(max_ce(
				txt::Cursor::difference_type{0},
				signed_cast(node.units()) - m_view.index()
			))
		},
		put_count, content_fg, content_bg
	);
	tty::Cell const cell_clear = tty::make_cell(' ', content_fg, content_bg);
	rd.terminal.put_line(
		{
			static_cast<geom_value_type>(frame.pos.x + 1 + put_count),
			frame.pos.y
		},
		inner_width - put_count, Axis::horizontal, cell_clear
	);
	cell_side.u8block.assign(']');
	rd.terminal.put_cell(
		frame.pos.x + frame.size.width - 1,
		frame.pos.y,
		cell_side
	);
	rd.terminal.set_caret_pos(
		frame.pos.x
		+ min_ce(
			inner_width,
			1 + static_cast<geom_value_type>(
				m_cursor.col() - m_view.col()
			)
		),
		frame.pos.y
	);
}

bool
BareField::input(
	UI::KeyInputData const& key_input
) {
	switch (key_input.code) {
	case KeyCode::up   : m_cursor.row_prev(); break;
	case KeyCode::down : m_cursor.row_next(); break;
	case KeyCode::left : m_cursor.col_prev(); break;
	case KeyCode::right: m_cursor.col_next(); break;
	case KeyCode::home : m_cursor.col_extent(txt::Extent::head); break;
	case KeyCode::end  : m_cursor.col_extent(txt::Extent::tail); break;
	case KeyCode::del      : m_cursor.erase(); break;
	case KeyCode::backspace: m_cursor.erase_before(); break;
	default:
		if (key_input.cp == codepoint_none || key_input.cp == '\t') {
			return false;
		} else {
			m_cursor.insert_step(key_input.cp);
		}
		break;
	}
	update_view();
	return true;
}

} // namespace UI
} // namespace Onsang
