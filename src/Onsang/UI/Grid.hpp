/**
@file UI/Grid.hpp
@brief Grid.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/UI/Defs.hpp>

#include <Beard/ui/Widget/Base.hpp>
#include <Beard/ui/ProtoGrid.hpp>
#include <Beard/ui/Field.hpp>

namespace Onsang {
namespace UI {

#define GRID_TMP_COLUMN_WIDTH 25

class Grid final
	: public UI::ProtoGrid
{
private:
	using base = UI::ProtoGrid;

public:
	struct Row final {
		enum class Flags : unsigned {
			selected = bit(0u)
		};

		duct::StateStore<Flags> states{};
	};

	using row_vector_type = aux::vector<Row>;

private:
	enum class ctor_priv {};

	Grid() noexcept = delete;
	Grid(Grid const&) = delete;
	Grid& operator=(Grid const&) = delete;

	struct {
		UI::index_type col{-1};
		UI::index_type row{-1};
	} m_cursor{};

	Hord::Object::Unit& m_object;
	struct Control {
		using get_cell_type = void(
			Hord::Object::Unit& /*object*/,
			UI::index_type /*row*/,
			UI::index_type /*col*/,
			String& /*value*/
		);

		using set_cell_type = void(
			Hord::Object::Unit& /*object*/,
			UI::index_type /*row*/,
			UI::index_type /*col*/,
			String const& /*value*/
		);

		get_cell_type& get_cell;
		set_cell_type& set_cell;

		Control(
			get_cell_type& get_cell,
			set_cell_type& set_cell
		) noexcept
			: get_cell(get_cell)
			, set_cell(set_cell)
		{}
	} m_control;

	row_vector_type m_rows;
	aux::shared_ptr<UI::Field> m_field;

private:
// UI::Widget::Base implementation
	void
	reflow_impl(
		Rect const& area,
		bool const cache
	) noexcept override;

	bool
	handle_event_impl(
		UI::Event const& event
	) noexcept override;

	void
	render_impl(
		UI::Widget::RenderData& rd
	) noexcept override;

// UI::ProtoGrid implementation
	void
	content_action(
		UI::ProtoGrid::ContentAction action,
		UI::index_type row_begin,
		UI::index_type count
	) noexcept override;

	void
	render_header(
		UI::GridRenderData& grid_rd,
		UI::index_type const col_begin,
		UI::index_type const col_end,
		Rect const& frame
	) noexcept override;

	void
	render_content(
		UI::GridRenderData& grid_rd,
		UI::index_type const row,
		UI::index_type const col_begin,
		UI::index_type const col_end,
		Rect const& frame
	) noexcept override;

private:
	void
	adjust_view() noexcept;

	void
	reflow_field(
		bool const cache
	) noexcept;

public:
	~Grid() noexcept override = default;

	Grid(
		ctor_priv const,
		UI::group_hash_type const group,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent,
		Hord::Object::Unit& object,
		Control::get_cell_type& get_cell,
		Control::set_cell_type& set_cell,
		UI::index_type const col_count,
		UI::index_type const row_count
	) noexcept
		: base(
			static_cast<UI::Widget::Type>(UI::OnsangWidgetType::Grid),
			enum_combine(
				UI::Widget::Flags::trait_focusable,
				UI::Widget::Flags::visible
			),
			group,
			{{0, 0}, true, Axis::both, Axis::both},
			std::move(root),
			std::move(parent),
			col_count,
			row_count
		)
		, m_object(object)
		, m_control(get_cell, set_cell)
		, m_rows(row_count)
		, m_field()
	{}

	static aux::shared_ptr<Grid>
	make(
		UI::RootWPtr root,
		Hord::Object::Unit& object,
		Control::get_cell_type& get_cell,
		Control::set_cell_type& set_cell,
		UI::index_type const col_count,
		UI::index_type const row_count,
		UI::group_hash_type const group = UI::group_default,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		auto p = aux::make_shared<Grid>(
			ctor_priv{},
			group,
			std::move(root),
			std::move(parent),
			object,
			get_cell,
			set_cell,
			col_count,
			row_count
		);
		p->m_field = UI::Field::make(
			p->get_root(),
			{},
			nullptr,
			UI::group_field/*,
			p*/
		);
		p->m_field->get_geometry().set_sizing(Axis::x, Axis::x);
		p->m_field->set_focused(true);
		p->m_field->clear_actions();
		p->set_cursor(0, 0);
		return p;
	}

	Grid(Grid&&) = default;
	Grid& operator=(Grid&&) = default;

public:
	row_vector_type&
	get_rows() noexcept {
		return m_rows;
	}

	void
	set_cursor(
		UI::index_type col,
		UI::index_type row
	) noexcept;

	void
	col_abs(
		UI::index_type const col
	) noexcept {
		set_cursor(col, m_cursor.row);
	}

	void
	col_step(
		UI::index_type const amt
	) noexcept {
		col_abs(m_cursor.col + amt);
	}

	void
	row_abs(
		UI::index_type row
	) noexcept {
		set_cursor(m_cursor.col, row);
	}

	void
	row_step(
		UI::index_type const amt
	) noexcept {
		row_abs(m_cursor.row + amt);
	}
};

void
Grid::reflow_impl(
	Rect const& area,
	bool const cache
) noexcept {
	base::reflow_impl(area, cache);
	Rect view_frame = get_geometry().get_frame();
	++view_frame.pos.x;
	++view_frame.pos.y;
	view_frame.size.width -= 2;
	view_frame.size.height -= 2;
	reflow_view(view_frame);
	adjust_view();
	queue_header_render();
	queue_cell_render(0, get_row_count());

	if (has_input_control()) {
		reflow_field(cache);
		m_field->queue_actions(
			UI::UpdateActions::render
		);
	}
}

bool
Grid::handle_event_impl(
	UI::Event const& event
) noexcept {
	switch (event.type) {
	case UI::EventType::key_input:
		if (has_input_control()) {
			bool const handled = m_field->handle_event(event);
			if (handled && !m_field->has_input_control()) {
				set_input_control(false);
				m_field->clear_actions();
				m_control.set_cell(m_object, m_cursor.row, m_cursor.col, m_field->get_text());
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
			bool handled = true;
			switch (event.key_input.code) {
			case KeyCode::enter:
				if (0 < get_row_count()) {
					set_input_control(true);
					reflow_field(true);
					String value;
					m_control.get_cell(m_object, m_cursor.row, m_cursor.col, value);
					m_field->set_text(std::move(value));
					m_field->handle_event(event);
					m_field->queue_actions(enum_combine(
						UI::UpdateActions::render
					));
				}
				break;

			case KeyCode::up   : row_step(-1); break;
			case KeyCode::down : row_step(+1); break;
			case KeyCode::left : col_step(-1); break;
			case KeyCode::right: col_step(+1); break;
			case KeyCode::home: row_abs(0); break;
			case KeyCode::end : row_abs(max_ce(0, get_row_count() - 1)); break;
			case KeyCode::pgup:
				row_step(min_ce(0, -get_view().fit_count - 1)); break;
			case KeyCode::pgdn:
				row_step(max_ce(0, +get_view().fit_count - 1)); break;

			case KeyCode::f1:
				set_header_enabled(!is_header_enabled());
				queue_actions(enum_combine(
					UI::UpdateActions::reflow,
					UI::UpdateActions::render
				));
				break;

			default:
				switch (event.key_input.cp) {
				case 'k': row_step(-1); break;
				case 'j': row_step(+1); break;
				case 'h': col_step(-1); break;
				case 'l': col_step(+1); break;
				case 'a': select_all(); break;
				case 'A': select_none(); break;
				case ' ': select_toggle(m_cursor.row, 1); break;
				//case 'e': erase(m_cursor.row, 1); break;
				//case 'E': erase_selected(); break;
				//case 'i': insert_after (m_cursor.row, 1); break;
				//case 'I': insert_before(m_cursor.row, 1); break;
				default:
					handled = false;
					break;
				}
			}
			return handled;
		}
		break;

	case UI::EventType::focus_changed:
		queue_cell_render(m_cursor.row, m_cursor.row + 1);
		return false;

	default:
		break;
	}
	return false;
}

void
Grid::render_impl(
	UI::Widget::RenderData& rd
) noexcept {
	/*DUCT_DEBUGF(
		"clearing render: %d",
		signed{!enum_bitand(
			get_queued_actions(),
			UI::UpdateActions::flag_noclear
		)}
	);*/

	UI::GridRenderData grid_rd{
		rd,
		is_focused(),
		is_focused()
	};
	auto const& view = get_view();

	render_view(
		grid_rd,
		!enum_bitand(
			get_queued_actions(),
			UI::UpdateActions::flag_noclear
		)
	);

	if (has_input_control()) {
		grid_rd.rd.update_group(m_field->get_group());
		m_field->render(grid_rd.rd);
	}

	Rect const empty_frame{
		{view.content_frame.pos.x, view.content_frame.pos.y + view.row_count},
		{
			view.content_frame.size.width,
			max_ce(0, view.fit_count - view.row_count)
		}
	};
	if (0 < empty_frame.size.height) {
		grid_rd.rd.terminal.clear_back(empty_frame);
		grid_rd.rd.terminal.put_line(
			empty_frame.pos,
			empty_frame.size.height,
			Axis::vertical,
			tty::make_cell('~',
				tty::Color::blue | tty::Attr::bold,
				tty::Color::term_default
			)
		);
	}
}

void
Grid::content_action(
	UI::ProtoGrid::ContentAction action,
	UI::index_type row_begin,
	UI::index_type count
) noexcept {
	using ContentAction = UI::ProtoGrid::ContentAction;

	assert(get_row_count() == static_cast<signed>(m_rows.size()));
	/*Log::acquire()
		<< "#rows: "
		<< get_row_count() << " / " << m_rows.size() << '\n'
	;*/
	// Cast insert_after in terms of insert_before
	if (ContentAction::insert_after == action) {
		++row_begin;
	}
	row_begin = value_clamp(row_begin, 0, get_row_count());
	auto const row_end = min_ce(row_begin + count, get_row_count());
	auto clear_flag = UI::UpdateActions::none;
	switch (action) {
	// Select
	case ContentAction::select:
	case ContentAction::unselect: {
		bool const enable = ContentAction::select == action;
		auto const end = m_rows.begin() + row_end;
		for (auto it = m_rows.begin() + row_begin; end > it; ++it) {
			it->states.set(
				Row::Flags::selected,
				enable
			);
		}
		queue_cell_render(row_begin, row_end);
		clear_flag = UI::UpdateActions::flag_noclear;
	}	break;

	case ContentAction::select_toggle: {
		auto const end = m_rows.begin() + row_end;
		for (auto it = m_rows.begin() + row_begin; end > it; ++it) {
			it->states.set(
				Row::Flags::selected,
				!it->states.test(Row::Flags::selected)
			);
		}
		queue_cell_render(row_begin, row_end);
		clear_flag = UI::UpdateActions::flag_noclear;
	}	break;

	// Insert
	case ContentAction::insert_after: // fall-through
	case ContentAction::insert_before:
		if (row_begin <= m_cursor.row && 0 < get_row_count()) {
			m_rows[m_cursor.row][m_cursor.col].states.disable(
				Column::Flags::focused
			);
		}
		m_rows.insert(
			m_rows.begin() + row_begin,
			static_cast<std::size_t>(count),
			Row{static_cast<std::size_t>(get_col_count())}
		);
		content_action_internal(
			ContentAction::insert_before,
			row_begin,
			count
		);
		if (1 == get_row_count()) {
			set_cursor(m_cursor.col, 0);
		} else if (row_begin <= m_cursor.row) {
			set_cursor(m_cursor.col, m_cursor.row + count);
		}
		m_rows[m_cursor.row][m_cursor.col].states.enable(
			Column::Flags::focused
		);
		adjust_view();
		break;

	// Erase
	case ContentAction::erase:
		m_rows.erase(
			m_rows.cbegin() + row_begin,
			m_rows.cbegin() + row_end
		);
		content_action_internal(
			ContentAction::erase,
			row_begin,
			count
		);
		break;

	case ContentAction::erase_selected:
		UI::index_type
			head = 0,
			tail = head,
			rcount = 0
		;
		for (; get_row_count() >= tail;) {
			if (
				get_row_count() == tail ||
				!m_rows[tail].states.test(Row::Flags::selected)
			) {
				if (tail > head) {
					m_rows.erase(
						m_rows.cbegin() + head,
						m_rows.cbegin() + tail
					);
					rcount = tail - head;
					content_action_internal(ContentAction::erase, head, rcount);
					if (head <= m_cursor.row) {
						set_cursor(
							m_cursor.col,
							max_ce(head, m_cursor.row - rcount)
						);
					}
					// tail is not selected, so no sense in checking it again
					tail = ++head;
				} else {
					head = ++tail;
				}
			} else {
				++tail;
			}
		}
		break;
	} // switch (action)

	// Post action
	switch (action) {
	case ContentAction::erase:
	case ContentAction::erase_selected:
		// Let cursor clamp to new bounds
		set_cursor(m_cursor.col, m_cursor.row);
		if (0 < get_row_count()) {
			// Set focused flag in case set_cursor() did nothing
			m_rows[m_cursor.row][m_cursor.col].states.enable(
				Column::Flags::focused
			);
		}
		adjust_view();
		break;

	default:
		break;
	}

	queue_actions(enum_combine(
		UI::UpdateActions::render,
		clear_flag
	));
}

void
Grid::render_header(
	UI::GridRenderData& grid_rd,
	UI::index_type const col_begin,
	UI::index_type const col_end,
	Rect const& frame
) noexcept {
	auto& rd = grid_rd.rd;
	rd.terminal.put_line(
		frame.pos,
		frame.size.width,
		Axis::horizontal,
		tty::make_cell(' ',
			tty::Color::term_default,
			tty::Color::blue
		)
	);
	Rect cell_frame = frame;
	cell_frame.pos.x += col_begin * GRID_TMP_COLUMN_WIDTH;
	String name{};
	for (auto col = col_begin; col_end > col; ++col) {
		cell_frame.size.width = min_ce(
			GRID_TMP_COLUMN_WIDTH,
			frame.pos.x + frame.size.width - cell_frame.pos.x
		);
		m_control.get_cell(m_object, -1, col, name);
		rd.terminal.put_sequence(
			cell_frame.pos.x,
			cell_frame.pos.y,
			txt::Sequence{name, 0u, name.size()},
			cell_frame.size.width,
			grid_rd.primary_fg | tty::Attr::bold,
			tty::Color::blue
		);
		if (GRID_TMP_COLUMN_WIDTH > cell_frame.size.width) {
			break;
		}
		cell_frame.pos.x += GRID_TMP_COLUMN_WIDTH;
	}
}

void
Grid::render_content(
	UI::GridRenderData& grid_rd,
	UI::index_type const row,
	UI::index_type const col_begin,
	UI::index_type const col_end,
	Rect const& frame
) noexcept {
	/*DUCT_DEBUGF(
		"render_content: row = %3d, col_range = {%3d, %3d}"
		", view.col_range = {%3d, %3d}, pos = {%3d, %3d}",
		row, col_begin, col_end,
		get_view().col_range.x, get_view().col_range.y,
		frame.pos.x, frame.pos.y
	);*/

	auto& rd = grid_rd.rd;
	auto const& r = m_rows[row];
	tty::attr_type attr_fg = grid_rd.content_fg;
	tty::attr_type attr_bg = grid_rd.content_bg;
	if (r.states.test(Row::Flags::selected)) {
		attr_fg = grid_rd.selected_fg;
		attr_bg = grid_rd.selected_bg;
	}

	Rect cell_frame = frame;
	cell_frame.pos.x += col_begin * GRID_TMP_COLUMN_WIDTH;
	auto const end
		= get_col_count() < col_end
		? r.columns.cend()
		: r.columns.cbegin() + col_end
	;
	auto cell = tty::make_cell(' ', attr_fg, attr_bg);
	String value;
	UI::index_type col = col_begin;
	for (auto it = r.columns.cbegin() + col_begin; end > it; ++it, ++col) {
		cell_frame.size.width = min_ce(
			GRID_TMP_COLUMN_WIDTH,
			frame.pos.x + frame.size.width - cell_frame.pos.x
		);
		cell.attr_bg
			= (it->states.test(Column::Flags::focused) && is_focused())
			? attr_bg | tty::Attr::inverted
			: attr_bg
		;
		rd.terminal.put_line(
			cell_frame.pos,
			cell_frame.size.width,
			Axis::horizontal,
			cell
		);
		m_control.get_cell(m_object, row, col, value);
		rd.terminal.put_sequence(
			cell_frame.pos.x,
			cell_frame.pos.y,
			txt::Sequence{value, 0u, value.size()},
			cell_frame.size.width,
			attr_fg,
			cell.attr_bg
		);
		if (GRID_TMP_COLUMN_WIDTH > cell_frame.size.width) {
			break;
		}
		cell_frame.pos.x += GRID_TMP_COLUMN_WIDTH;
	}
}

void
Grid::adjust_view() noexcept {
	auto const& view = get_view();
	if (
		view.row_range.x >  m_cursor.row ||
		view.row_range.y <= m_cursor.row ||
		view.col_range.x >  m_cursor.col ||
		view.col_range.y <= m_cursor.col
	) {
		auto row_begin = m_cursor.row;
		if (view.row_range.y <= m_cursor.row) {
			row_begin -= view.fit_count - 1;
		}
		update_view(
			row_begin,
			row_begin + view.fit_count,
			0,
			get_col_count(),
			true
		);
		queue_actions(UI::UpdateActions::render);
	}
}

void
Grid::reflow_field(
	bool const cache
) noexcept {
	auto const& frame = get_view().content_frame;
	Quad cell_quad{
		{
			frame.pos.x + (m_cursor.col * GRID_TMP_COLUMN_WIDTH),
			frame.pos.y + m_cursor.row - get_view().row_range.x
		},
		{0, 0}
	};
	cell_quad.v2.x = cell_quad.v1.x + GRID_TMP_COLUMN_WIDTH;
	cell_quad.v2.y = cell_quad.v1.y + 1;
	Quad const fq = rect_abs_quad(frame);
	vec2_clamp(cell_quad.v1, fq.v1, fq.v2);
	vec2_clamp(cell_quad.v2, fq.v1, fq.v2);
	m_field->reflow(
		quad_rect(cell_quad),
		cache
	);
}

void
Grid::set_cursor(
	UI::index_type col,
	UI::index_type row
) noexcept {
	col = value_clamp(col, 0, max_ce(0, get_col_count() - 1));
	row = value_clamp(row, 0, max_ce(0, get_row_count() - 1));
	if (col != m_cursor.col || row != m_cursor.row) {
		if (
			value_in_bounds(m_cursor.row, 0, get_row_count()) &&
			value_in_bounds(m_cursor.col, 0, get_col_count())
		) {
			m_rows[m_cursor.row][m_cursor.col].states.disable(
				Column::Flags::focused
			);
			queue_cell_render(
				m_cursor.row, m_cursor.row + 1,
				m_cursor.col, m_cursor.col + 1
			);
		}
		if (!m_rows.empty()) {
			m_rows[row][col].states.enable(
				Column::Flags::focused
			);
			queue_cell_render(
				row, row + 1,
				col, col + 1
			);
		}
		queue_actions(enum_combine(
			UI::UpdateActions::render,
			UI::UpdateActions::flag_noclear
		));
		m_cursor.col = col;
		m_cursor.row = row;
		adjust_view();
	}
}

} // namespace UI
} // namespace Onsang
