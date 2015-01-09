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

#include <Beard/txt/Defs.hpp>
#include <Beard/ui/Field.hpp>

#include <Hord/Data/Defs.hpp>
#include <Hord/Data/Table.hpp>
#include <Hord/Data/Ops.hpp>
#include <Hord/Object/Defs.hpp>
#include <Hord/Cmd/Object.hpp>

#include <duct/IO/memstream.hpp>

#include <string>

namespace Onsang {
namespace UI {

#define GRID_TMP_COLUMN_WIDTH 25

void
TableGrid::reflow_impl(
	Rect const& area,
	bool const cache
) noexcept {
	base::reflow_impl(area, cache);
	if (has_input_control()) {
		reflow_field(cache);
		m_field->queue_actions(
			UI::UpdateActions::render
		);
	}
}

bool
TableGrid::handle_event_impl(
	UI::Event const& event
) noexcept {
	if (base::handle_event_impl(event)) {
		return true;
	}
	switch (event.type) {
	case UI::EventType::key_input:
		if (has_input_control()) {
			bool const handled = m_field->handle_event(event);
			if (handled && !m_field->has_input_control()) {
				if (event.key_input.code != UI::KeyCode::esc) {
					String const edit_value{m_field->get_text()};
					auto const field_type = m_table.column(m_cursor.col).type;
					if (
						field_type == Hord::Data::ValueType::string ||
						field_type == Hord::Data::ValueType::dynamic
					) {
						// TODO: Use callback instead
						auto it = m_table.iterator_at(m_cursor.row);
						it.set_field(m_cursor.col, edit_value);
						m_object.get_prop_states().assign(
							m_prop_type,
							Hord::IO::PropState::modified
						);
					}
				}
				set_input_control(false);
				m_field->clear_actions();
				queue_cell_render(
					m_cursor.row, m_cursor.row + 1,
					m_cursor.col, m_cursor.col + 1
				);
				queue_actions(enum_combine(
					UI::UpdateActions::render,
					UI::UpdateActions::flag_noclear
				));
			}
			return handled;
		} else {
			switch (event.key_input.code) {
			case KeyCode::enter: {
				auto it = m_table.iterator_at(m_cursor.row);
				auto const value = it.get_field(m_cursor.col);
				if (
					value.type.type() == Hord::Data::ValueType::string
				) {
					// TODO: Use callback instead
					m_field->set_text({value.data.string, value.size});
				} else {
					m_field->set_text("");
				}
				set_input_control(true);
				reflow_field(true);
				// TODO: Field should have a toggle function
				m_field->handle_event(event);
				m_field->queue_actions(enum_combine(
					UI::UpdateActions::render
				));
				return true;
			}

			case KeyCode::f1:
				set_header_enabled(!is_header_enabled());
				queue_actions(enum_combine(
					UI::UpdateActions::reflow,
					UI::UpdateActions::render
				));
				return true;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}
	return false;
}

void
TableGrid::render_impl(
	UI::Widget::RenderData& rd
) noexcept {
	base::render_impl(rd);
	if (has_input_control()) {
		rd.update_group(m_field->get_group());
		m_field->render(rd);
	}
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
		type_prefix.u8block = s_type_prefix[table_column.type.value()];
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
	txt::Sequence seq{};
	Hord::Data::ValueRef value{};
	Hord::Data::Table::Iterator it_table = m_table.iterator_at(row_begin);
	for (UI::index_type row = row_begin; row < row_end; ++row) {
		if (m_rows[row].states.test(Row::Flags::selected)) {
			cell.attr_fg = grid_rd.selected_fg;
			cell.attr_bg = grid_rd.selected_bg;
		} else {
			cell.attr_fg = grid_rd.content_fg;
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
TableGrid::reflow_field(
	bool const cache
) noexcept {
	auto const& frame = get_view().content_frame;
	Quad cell_quad{
		{
			frame.pos.x + (m_cursor.col - get_view().col_range.x) * GRID_TMP_COLUMN_WIDTH,
			frame.pos.y + m_cursor.row - get_view().row_range.x
		},
		{0, 0}
	};
	cell_quad.v2.x = cell_quad.v1.x + GRID_TMP_COLUMN_WIDTH;
	cell_quad.v2.y = cell_quad.v1.y + 1;
	Quad const fq = rect_abs_quad(frame);
	vec2_clamp(cell_quad.v1, fq.v1, fq.v2);
	vec2_clamp(cell_quad.v2, fq.v1, fq.v2);
	m_field->reflow(quad_rect(cell_quad), cache);
}

#undef GRID_TMP_COLUMN_WIDTH

} // namespace UI
} // namespace Onsang

