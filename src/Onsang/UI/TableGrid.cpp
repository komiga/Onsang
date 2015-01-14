/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TableGrid.hpp>
#include <Onsang/UI/ObjectView.hpp>

#include <Beard/keys.hpp>
#include <Beard/txt/Defs.hpp>
#include <Beard/ui/packing.hpp>

#include <Hord/Data/Defs.hpp>
#include <Hord/Data/ValueRef.hpp>
#include <Hord/Data/Table.hpp>
#include <Hord/Data/Ops.hpp>
#include <Hord/Object/Defs.hpp>
#include <Hord/Cmd/Object.hpp>

#include <duct/IO/memstream.hpp>

#include <cstdlib>
#include <algorithm>
#include <string>

namespace Onsang {
namespace UI {

#define GRID_TMP_COLUMN_WIDTH 25

void
TableGrid::set_input_control_impl(
	bool const enabled
) noexcept {
	base::set_input_control_impl(enabled);
	get_root()->get_context().get_terminal().set_caret_visible(
		has_input_control()
	);
	if (has_input_control()) {
		m_field_cursor.col_extent(txt::Extent::tail);
	} else {
		queue_cell_render(
			m_cursor.row, m_cursor.row + 1,
			m_cursor.col, m_cursor.col + 1
		);
	}
	update_field_view();
	queue_actions(enum_combine(
		ui::UpdateActions::render,
		ui::UpdateActions::flag_noclear
	));
}

void
TableGrid::reflow_impl(
	Rect const& area,
	bool const cache
) noexcept {
	base::reflow_impl(area, cache);
	if (has_input_control()) {
		reflow_field();
		queue_actions(enum_combine(
			UI::UpdateActions::render,
			UI::UpdateActions::flag_noclear
		));
	}
}

bool
TableGrid::handle_event_impl(
	UI::Event const& event
) noexcept {
	if (base::handle_event_impl(event)) {
		return true;
	} else if (event.type != UI::EventType::key_input) {
		return false;
	}
	if (!has_input_control()) {
		if (
			event.key_input.code != KeyCode::enter &&
			event.key_input.cp != ' ' &&
			event.key_input.cp != '*'
		) {
			return false;
		}
		auto const& column = m_table.get_schema().column(m_cursor.col);
		auto const value = m_table.iterator_at(m_cursor.row).get_field(m_cursor.col);
		// TODO: object_id handling (translate to/from path)
		// TODO: Size limit for string
		if (
			(
				value.type == Hord::Data::ValueType::null &&
				column.type != Hord::Data::ValueType::dynamic
			) ||
			value.type == Hord::Data::ValueType::object_id
		) {
			return true;
		}
		m_field_type = value.type;
		switch (m_field_type.type()) {
		case Hord::Data::ValueType::null:
			m_field_cursor.clear();
			m_field_type = column.type;
			break;

		case Hord::Data::ValueType::string:
			m_field_cursor.assign(value.data.string, value.size);
			break;

		default: {
			char value_buffer[48];
			duct::IO::omemstream format_stream{value_buffer, sizeof(value_buffer)};
			format_stream << value;
			m_field_cursor.assign(
				value_buffer,
				static_cast<unsigned>(format_stream.tellp())
			);
		}	break;
		}
		if (event.key_input.cp == '*') {
			m_field_type = Hord::Data::ValueType::dynamic;
		}
		reflow_field();
		set_input_control(true);
		return true;
	} else if (event.key_input.code == KeyCode::enter) {
		// TODO: Use callback instead
		String string_value;
		Hord::Data::ValueRef new_value{};
		if (!m_field_text_tree.empty()) {
			m_field_cursor.col_extent(txt::Extent::tail);
			auto const& node = m_field_cursor.get_node();
			switch (m_field_type.type()) {
			case Hord::Data::ValueType::dynamic:
			case Hord::Data::ValueType::integer:
			case Hord::Data::ValueType::decimal:
				m_field_cursor.insert('\0');
				new_value.read_from_string(node.units() - 1, &*node.cbegin());
				break;

			case Hord::Data::ValueType::string:
				string_value = m_field_text_tree.to_string();
				new_value = string_value;
				break;

			default: break;
			}
		}
		auto it = m_table.iterator_at(m_cursor.row);
		auto const old_value = it.get_field(m_cursor.col);
		if (old_value != new_value) {
			it.set_field(m_cursor.col, new_value);
			m_object.get_prop_states().assign(
				m_prop_type,
				Hord::IO::PropState::modified
			);
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
		case KeyCode::up   : m_field_cursor.row_prev(); break;
		case KeyCode::down : m_field_cursor.row_next(); break;
		case KeyCode::left : m_field_cursor.col_prev(); break;
		case KeyCode::right: m_field_cursor.col_next(); break;
		case KeyCode::home: m_field_cursor.col_extent(txt::Extent::head); break;
		case KeyCode::end : m_field_cursor.col_extent(txt::Extent::tail); break;
		case KeyCode::del      : m_field_cursor.erase(); break;
		case KeyCode::backspace: m_field_cursor.erase_before(); break;
		default:
			if (event.key_input.cp != codepoint_none) {
				field_input(event.key_input.cp);
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
	return false;
}

void
TableGrid::render_impl(
	UI::Widget::RenderData& rd
) noexcept {
	base::render_impl(rd);
	render_field(rd);
}

void
TableGrid::render_header(
	UI::GridRenderData& grid_rd,
	UI::index_type const col_begin,
	UI::index_type const col_end,
	Rect const& frame
) noexcept {
	static constexpr char const
	s_type_prefix[]{'n', '*', 'i', 'f', '#', 's'};

	grid_rd.rd.terminal.put_line(
		frame.pos,
		get_geometry().get_frame().size.width,
		Axis::horizontal,
		tty::make_cell(' ',
			tty::Color::term_default,
			tty::Color::blue
		)
	);

	Rect cell_frame = frame;
	cell_frame.pos.x += col_begin * GRID_TMP_COLUMN_WIDTH;
	tty::Cell type_prefix = tty::make_cell(
		' ',
		tty::Color::green | tty::Attr::bold,
		tty::Color::blue
	);
	for (auto col = col_begin; col < col_end; ++col) {
		auto const& table_column = m_table.get_schema().column(col);
		cell_frame.size.width = min_ce(
			GRID_TMP_COLUMN_WIDTH,
			frame.pos.x + frame.size.width - cell_frame.pos.x
		);
		if (0 >= cell_frame.size.width) {
			break;
		}
		type_prefix.u8block = s_type_prefix[enum_cast(table_column.type.type())];
		grid_rd.rd.terminal.put_cell(cell_frame.pos.x + 0, cell_frame.pos.y, type_prefix);
		type_prefix.u8block = ':';
		grid_rd.rd.terminal.put_cell(cell_frame.pos.x + 1, cell_frame.pos.y, type_prefix);
		grid_rd.rd.terminal.put_sequence(
			cell_frame.pos.x + 2,
			cell_frame.pos.y,
			{table_column.name},
			cell_frame.size.width - 2,
			grid_rd.primary_fg | tty::Attr::bold,
			tty::Color::blue
		);
		if (GRID_TMP_COLUMN_WIDTH > cell_frame.size.width) {
			break;
		}
		cell_frame.pos.x += cell_frame.size.width;
	}
}

void
TableGrid::render_content(
	UI::GridRenderData& grid_rd,
	UI::index_type const row_begin,
	UI::index_type const row_end,
	UI::index_type const col_begin,
	UI::index_type const col_end,
	Rect const& frame
) noexcept {
	/*DUCT_DEBUGF(
		"render_content:"
		" row_range = {%3d, %3d}"
		", col_range = {%3d, %3d}"
		", view.col_range = {%3d, %3d}"
		", pos = {%3d, %3d}"
		", size = {%3d, %3d}",
		row_begin, row_end,
		col_begin, col_end,
		get_view().col_range.x, get_view().col_range.y,
		frame.pos.x, frame.pos.y,
		frame.size.width, frame.size.height
	);*/

	Rect cell_frame = frame;
	cell_frame.size.height = 1;
	char value_buffer[48];
	duct::IO::omemstream format_stream{value_buffer, sizeof(value_buffer)};
	auto cell = tty::make_cell(' ');
	tty::attr_type attr_fg;
	txt::Sequence seq{};
	Hord::Data::ValueRef value{};
	Hord::Data::Table::Iterator it_table = m_table.iterator_at(row_begin);
	for (UI::index_type row = row_begin; row < row_end; ++row) {
		if (m_rows[row].states.test(Row::Flags::selected)) {
			attr_fg = grid_rd.selected_fg;
			cell.attr_bg = grid_rd.selected_bg;
		} else {
			attr_fg = grid_rd.content_fg;
			cell.attr_bg = grid_rd.content_bg;
		}
		cell_frame.pos.x = frame.pos.x + (col_begin - get_view().col_range.x) * GRID_TMP_COLUMN_WIDTH;

	for (UI::index_type col = col_begin; col < col_end; ++col) {
		if (row == m_cursor.row && col == m_cursor.col && is_focused()) {
			cell.attr_bg |= tty::Attr::inverted;
		} else {
			cell.attr_bg &= ~tty::Attr::inverted;
		}
		cell_frame.size.width = min_ce(
			GRID_TMP_COLUMN_WIDTH,
			frame.pos.x + frame.size.width - cell_frame.pos.x
		);
		if (0 >= cell_frame.size.width) {
			break;
		}

		value = it_table.get_field(col);
		cell.attr_fg
			= value.type.type() == Hord::Data::ValueType::null
			? tty::Color::red
			: attr_fg
		;
		if (value.type.type() == Hord::Data::ValueType::string) {
			seq = {value.data.string, value.size};
		} else {
			format_stream.seekp(0);
			format_stream << value;
			seq = {value_buffer, static_cast<std::size_t>(format_stream.tellp())};
		}
		grid_rd.rd.terminal.put_line(
			cell_frame.pos,
			cell_frame.size.width,
			Axis::horizontal,
			cell
		);
		grid_rd.rd.terminal.put_sequence(
			cell_frame.pos.x,
			cell_frame.pos.y,
			seq,
			cell_frame.size.width,
			cell.attr_fg, cell.attr_bg
		);
		if (GRID_TMP_COLUMN_WIDTH > cell_frame.size.width) {
			break;
		}
		cell_frame.pos.x += cell_frame.size.width;
	}
		++cell_frame.pos.y;
		++it_table;
	}
}

bool
TableGrid::content_insert(
	UI::index_type /*row_begin*/,
	UI::index_type /*count*/
) noexcept {
	// TODO
	return false;
}

bool
TableGrid::content_erase(
	UI::index_type /*row_begin*/,
	UI::index_type /*count*/
) noexcept {
	// TODO
	return false;
}

void
TableGrid::update_field_view() noexcept {
	auto const& frame = m_field_geom.get_frame();
	auto const inner_width = max_ce(0, frame.size.width - 2 - 1);
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
TableGrid::reflow_field() noexcept {
	auto const& content_frame = get_view().content_frame;
	Quad cell_quad{
		{
			content_frame.pos.x + (m_cursor.col - get_view().col_range.x) * GRID_TMP_COLUMN_WIDTH,
			content_frame.pos.y + (m_cursor.row - get_view().row_range.x)
		},
		{0, 0}
	};
	cell_quad.v2.x = cell_quad.v1.x + GRID_TMP_COLUMN_WIDTH;
	cell_quad.v2.y = cell_quad.v1.y + 1;
	Quad const fq = rect_abs_quad(content_frame);
	vec2_clamp(cell_quad.v1, fq.v1, fq.v2);
	vec2_clamp(cell_quad.v2, fq.v1, fq.v2);
	UI::reflow(quad_rect(cell_quad), m_field_geom);
	update_field_view();
}

void
TableGrid::render_field(
	UI::Widget::RenderData& rd
) noexcept {
	if (!has_input_control()) {
		return;
	}
	rd.update_group(UI::group_field);
	auto const& frame = m_field_geom.get_frame();
	auto const& node = m_field_cursor.get_node();

	tty::attr_type const
		primary_fg = rd.get_attr(ui::property_primary_fg_active),
		primary_bg = rd.get_attr(ui::property_primary_bg_active),
		content_fg = rd.get_attr(ui::property_content_fg_active),
		content_bg = rd.get_attr(ui::property_content_bg_active)
	;
	tty::Cell cell_side = tty::make_cell('[', primary_fg, primary_bg);
	rd.terminal.put_cell(frame.pos.x, frame.pos.y, cell_side);
	auto const inner_width = max_ce(geom_value_type{0}, frame.size.width - 2);
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
				m_field_cursor.col() - m_field_view.col()
			)
		),
		frame.pos.y
	);
}

bool
TableGrid::field_input(
	char32 const cp
) noexcept {
	// Blacklist
	if (cp == '\t') {
		return false;
	}
	switch (m_field_type.type()) {
	case Hord::Data::ValueType::integer:
		if (!(
			('0' <= cp && cp <= '9') ||
			cp == '-' || cp == '+'
		)) {
			return false;
		}
		break;

	case Hord::Data::ValueType::decimal:
		if (!(
			('0' <= cp && cp <= '9') ||
			cp == '-' || cp == '+' ||
			cp == '.' ||
			cp == 'e' || cp == 'E'
		)) {
			return false;
		}
		break;

	case Hord::Data::ValueType::object_id:
		if (!(
			('0' <= cp && cp <= '9') ||
			('a' <= cp && cp <= 'f') ||
			('A' <= cp && cp <= 'F')
		)) {
			return false;
		}
		break;

	default: break;
	}
	m_field_cursor.insert_step(cp);
	return true;
}

#undef GRID_TMP_COLUMN_WIDTH

} // namespace UI
} // namespace Onsang

