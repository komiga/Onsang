/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TableSchemaEditor.hpp>

#include <Beard/ui/Container.hpp>
#include <Beard/ui/Spacer.hpp>
#include <Beard/ui/Button.hpp>

#include <Hord/Data/Defs.hpp>
#include <Hord/Data/TableSchema.hpp>
#include <Hord/Data/Table.hpp>
#include <Hord/Table/Unit.hpp>

#include <cstring>
#include <algorithm>

namespace Onsang {
namespace UI {

namespace {
static txt::Sequence const
s_column_name[TableSchemaEditor::NUM_COLUMNS]{
	{"index"}, {"name"}, {"type"}, {"size"}, {"signed"}
};

static UI::geom_value_type const
s_column_width []{5 + 1, 16 + 1, 9 + 1, 4 + 1, 6},
s_column_offset[]{0, 5 + 1, 21 + 2, 30 + 3, 34 + 4, 40 + 4};
} // anonymous namespace

void
TableSchemaEditor::set_input_control_impl(
	bool const enabled
) noexcept {
	base::set_input_control_impl(enabled);
	if (has_input_control()) {
		reflow_field();
	} else {
		queue_cell_render(
			m_cursor.row, m_cursor.row + 1,
			m_cursor.col, m_cursor.col + 1
		);
	}
	m_field.input_control_changed(*this);
	queue_actions(
		ui::UpdateActions::render |
		ui::UpdateActions::flag_noclear
	);
}

void
TableSchemaEditor::reflow_impl() noexcept {
	base::reflow_impl();
	if (has_input_control()) {
		reflow_field();
	}
}

static KeyInputMatch const
s_kim_activate_cell[]{
	{KeyMod::none, KeyCode::enter, codepoint_none, false},
	{KeyMod::none, KeyCode::none, ' ', false},
},
s_kim_insert_before[]{
	{KeyMod::none, KeyCode::insert, codepoint_none, false},
	{KeyMod::none, KeyCode::none, 'i', false},
},
s_kim_insert_after[]{
	{KeyMod::shift, KeyCode::insert, codepoint_none, false},
	{KeyMod::none, KeyCode::none, 'I', false},
};

bool
TableSchemaEditor::handle_event_impl(
	UI::Event const& event
) noexcept {
	if (base::handle_event_impl(event)) {
		return true;
	} else if (event.type != UI::EventType::key_input) {
		return false;
	}
	if (!has_input_control()) {
		if (key_input_match(event.key_input, s_kim_insert_before)) {
			insert_before(m_cursor.row, 1);
			return true;
		} else if (key_input_match(event.key_input, s_kim_insert_after)) {
			insert_after(m_cursor.row, 1);
			return true;
		} else if (event.key_input.cp == 'R') {
			reset();
			return true;
		}
		if (m_data.empty()) {
			return false;
		}
		auto& data = m_data[m_cursor.row];
		if (
			key_input_match(event.key_input, s_kim_activate_cell) &&
			!(data.orig.index != ~0u && data.edit.index == ~0u)
		) {
			if (m_cursor.col == 1) {
				m_field.m_cursor.assign(data.edit.name);
				set_input_control(true);
			} else {
				cursor_step_value(false);
			}
			return true;
		} else if (event.key_input.cp == 'e') {
			erase_selected();
			return true;
		} else if (event.key_input.cp == 'E') {
			erase(m_cursor.row, 1);
			erase_selected();
			return true;
		} else if (event.key_input.cp == 'r' && data.orig.index != ~0u) {
			data.edit = data.orig;
			data.modified.clear();
			queue_cell_render(
				m_cursor.row, m_cursor.row + 1
			);
			queue_actions(
				ui::UpdateActions::render |
				ui::UpdateActions::flag_noclear
			);
			return true;
		}
		return false;
	} else if (event.key_input.code == KeyCode::enter) {
		auto const& node = m_field.m_cursor.get_node();
		auto& data = m_data[m_cursor.row];
		if (node.points() > 0) {
			data.edit.name = node.to_string();
			data.modified.set(1 << unsigned_cast(m_cursor.col), data.edit.name != data.orig.name);
		} else {
			m_field.m_cursor.assign(data.edit.name);
		}
		m_field.m_cursor.clear();
		set_input_control(false);
		return true;
	} else if (event.key_input.code == KeyCode::esc) {
		m_field.m_cursor.clear();
		set_input_control(false);
		return true;
	} else {
		if (m_field.input(event.key_input)) {
			queue_actions(
				ui::UpdateActions::render |
				ui::UpdateActions::flag_noclear
			);
		}
		return true;
	}
	return false;
}

void
TableSchemaEditor::render_impl(
	UI::Widget::RenderData& rd
) noexcept {
	base::render_impl(rd);
	if (!has_input_control()) {
		return;
	}
	rd.update_group(UI::group_field);
	m_field.render(rd, true);
}

void
TableSchemaEditor::render_header(
	UI::GridRenderData& grid_rd,
	UI::index_type const col_begin,
	UI::index_type const col_end,
	Rect const& frame
) noexcept {
	grid_rd.rd.terminal.put_line(
		frame.pos, frame.size.width,
		Axis::horizontal,
		tty::make_cell(' ',
			tty::Color::term_default,
			tty::Color::blue
		)
	);

	auto const& view = get_view();
	auto const x_end = frame.pos.x + frame.size.width;
	Vec2 pos = frame.pos;
	Vec2 size = frame.size;
	pos.x += s_column_offset[col_begin] - s_column_offset[view.col_range.x];
	for (auto col = col_begin; col < col_end; ++col) {
		size.width = min_ce(s_column_width[col], x_end - pos.x);
		if (size.width <= 0) {
			break;
		}
		grid_rd.rd.terminal.put_sequence(
			pos.x, pos.y,
			s_column_name[col],
			size.width,
			grid_rd.primary_fg | tty::Attr::bold,
			tty::Color::blue
		);
		pos.x += size.width;
	}
}

void
TableSchemaEditor::render_content(
	UI::GridRenderData& grid_rd,
	UI::index_type const row_begin,
	UI::index_type const row_end,
	UI::index_type const col_begin,
	UI::index_type const col_end,
	Rect const& frame
) noexcept {
	/*DUCT_DEBUGF(
		"TableSchemaEditor::render_content:"
		" row_range = {%3d, %3d}"
		", col_range = {%3d, %3d}"
		", view.col_range = {%3d, %3d}"
		", view.row_range = {%3d, %3d}"
		", pos = {%3d, %3d}"
		", size = {%3d, %3d}",
		row_begin, row_end,
		col_begin, col_end,
		get_view().col_range.x, get_view().col_range.y,
		get_view().row_range.x, get_view().row_range.y,
		frame.pos.x, frame.pos.y,
		frame.size.width, frame.size.height
	);*/

	auto const& view = get_view();
	auto const x_end = frame.pos.x + frame.size.width;
	auto const begin_offset = s_column_offset[col_begin] - s_column_offset[view.col_range.x];
	auto const range_width = s_column_offset[col_end] - s_column_offset[col_begin];
	Vec2 pos = frame.pos;
	Vec2 size = frame.size;
	size.height = 1;
	auto cell = tty::make_cell(' ');
	String scratch;
	for (UI::index_type row = row_begin; row < row_end; ++row) {
		pos.x = frame.pos.x + begin_offset;
		auto const& data = m_data[row];
		if (m_sel[row]) {
			cell.attr_fg = grid_rd.selected_fg;
			cell.attr_bg = grid_rd.selected_bg;
		} else if (data.orig.index == ~0u) {
			cell.attr_fg = tty::Color::white | tty::Attr::bold;
			cell.attr_bg = tty::Color::green;
		} else if (data.edit.index == ~0u) {
			cell.attr_fg = tty::Color::white | tty::Attr::bold;
			cell.attr_bg = tty::Color::red;
		} else {
			cell.attr_fg = grid_rd.content_fg;
			cell.attr_bg = grid_rd.content_bg;
		}
		grid_rd.rd.terminal.put_line(pos, range_width, Axis::horizontal, cell);

	for (UI::index_type col = col_begin; col < col_end; ++col) {
		size.width = min_ce(s_column_width[col], x_end - pos.x);
		if (size.width <= 0) {
			break;
		}
		if (row == m_cursor.row && col == m_cursor.col && is_focused()) {
			cell.attr_bg |= tty::Attr::inverted;
			grid_rd.rd.terminal.put_line(pos, size.width, Axis::horizontal, cell);
		} else {
			cell.attr_bg &= ~tty::Attr::inverted;
		}
		grid_rd.rd.terminal.put_sequence(
			pos.x, pos.y,
			get_cell_seq(row, col, scratch),
			size.width,
			data.edit.index != ~0u && data.modified.test(1 << unsigned_cast(col))
			? tty::Color::yellow
			: cell.attr_fg,
			cell.attr_bg
		);
		pos.x += size.width;
	}
		++pos.y;
	}
}

bool
TableSchemaEditor::content_insert(
	UI::index_type row
) noexcept {
	Data data{
		{~0u, {}, {}},
		{~0u, {}, {}},
		{}
	};
	m_data.insert(m_data.cbegin() + row, data);
	return true;
}

bool
TableSchemaEditor::content_erase(
	UI::index_type row
) noexcept {
	auto& data = m_data[row];
	if (data.orig.index == ~0u) {
		m_data.erase(m_data.cbegin() + row);
		return true;
	} else {
		data.edit.index = ~0u;
		m_sel[row] = false;
	}
	return false;
}

txt::Sequence
TableSchemaEditor::get_cell_seq(
	UI::index_type row,
	UI::index_type col,
	String& scratch
) noexcept {
	auto const& data = m_data[row];
	switch (col) {
	case 0:
		if (data.orig.index == ~0u) {
			return {"INS"};
		} else if (data.edit.index == ~0u) {
			return {"DEL"};
		} else {
			scratch = std::to_string(data.edit.index);
			return {scratch};
		}

	case 1:
		return {data.edit.name};

	case 2: {
		auto const value = Hord::Data::get_value_type_name(data.edit.type.type());
		return {value, std::strlen(value)};
	}

	case 3: {
		auto const value = Hord::Data::get_size_name(data.edit.type.size());
		return {value, std::strlen(value)};
	}

	case 4:
		if (enum_cast(data.edit.type.flags() & Hord::Data::ValueFlag::integer_signed)) {
			return {"true"};
		} else {
			return {"false"};
		}
	}
	return {};
}

void
TableSchemaEditor::reflow_field() noexcept {
	auto const& view = get_view();
	auto const& content_frame = view.content_frame;
	Quad cell_quad{
		{
			content_frame.pos.x + s_column_offset[m_cursor.col] - s_column_offset[view.col_range.x],
			content_frame.pos.y + (m_cursor.row - view.row_range.x)
		},
		{0, 0}
	};
	cell_quad.v2.x = cell_quad.v1.x + s_column_width[m_cursor.col];
	cell_quad.v2.y = cell_quad.v1.y + 1;
	Quad const fq = rect_abs_quad(content_frame);
	vec2_clamp(cell_quad.v1, fq.v1, fq.v2);
	vec2_clamp(cell_quad.v2, fq.v1, fq.v2);
	m_field.reflow_into(quad_rect(cell_quad));
}

void
TableSchemaEditor::reset() {
	auto const& schema = m_table.get_data().get_schema();
	m_data.resize(schema.num_columns());
	auto it = m_data.begin();
	for (auto const& schema_column : schema.get_columns()) {
		it->orig = schema_column;
		it->edit = schema_column;
		it->modified.clear();
		++it;
	}
	resize_grid(NUM_COLUMNS, signed_cast(m_data.size()));
	std::fill(m_sel.begin(), m_sel.end(), false);
}

void
TableSchemaEditor::cursor_step_value(
	bool const prev
) {
	if (m_data.empty()) {
		return;
	}
	auto& data = m_data[m_cursor.row];
	auto value_type = data.edit.type.type();
	auto size = data.edit.type.size();
	auto flags = data.edit.type.flags();
	signed const step = prev ? -1 : 1;
	switch (m_cursor.col) {
	case 0:
	case 1:
		return;

	case 2:
		if (prev && value_type == Hord::Data::value_type_first) {
			value_type = Hord::Data::value_type_last;
		} else if (!prev && value_type == Hord::Data::value_type_last) {
			value_type = Hord::Data::value_type_first;
		} else {
			value_type = static_cast<Hord::Data::ValueType>(static_cast<signed>(value_type) + step);
		}
		data.modified.set(1 << 2, data.orig.type.type() != value_type);
		break;

	case 3:
		if (prev && size == Hord::Data::size_first) {
			size = Hord::Data::size_last;
		} else if (!prev && size == Hord::Data::size_last) {
			size = Hord::Data::size_first;
		} else {
			size = static_cast<Hord::Data::Size>(static_cast<signed>(size) + step);
		}
		data.modified.set(1 << 3, data.orig.type.size() != size);
		break;

	case 4:
		if (enum_cast(flags & Hord::Data::ValueFlag::integer_signed)) {
			flags &= ~Hord::Data::ValueFlag::integer_signed;
		} else {
			flags |= Hord::Data::ValueFlag::integer_signed;
		}
		data.modified.set(1 << 4, data.orig.type.flags() != flags);
		break;
	}
	if (
		value_type != Hord::Data::ValueType::integer &&
		enum_cast(flags & Hord::Data::ValueFlag::integer_signed)
	) {
		flags &= ~Hord::Data::ValueFlag::integer_signed;
		data.modified.set(1 << 4, data.orig.type.flags() != flags);
	}
	data.edit.type = {value_type, flags, size};
	queue_cell_render(
		m_cursor.row, m_cursor.row + 1,
		m_cursor.col, m_cursor.col + 1
	);
	queue_actions(
		ui::UpdateActions::render |
		ui::UpdateActions::flag_noclear
	);
}

} // namespace UI
} // namespace Onsang
