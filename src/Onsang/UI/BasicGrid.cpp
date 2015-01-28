/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/utility.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/BasicGrid.hpp>
#include <Onsang/App.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

void
BasicGrid::reflow_impl() noexcept {
	base::reflow_impl();
	reflow_view(geometry().frame());
	adjust_view();
	queue_header_render();
	queue_cell_render(0, row_count());
	enqueue_actions(
		UI::UpdateActions::render |
		UI::UpdateActions::flag_noclear
	);
}

bool
BasicGrid::handle_event_impl(
	UI::Event const& event
) noexcept {
	switch (event.type) {
	case UI::EventType::key_input:
		if (!has_input_control() && event.key_input.mod == KeyMod::none) {
			bool handled = true;
			switch (event.key_input.code) {
			case KeyCode::up   : row_step(-1); break;
			case KeyCode::down : row_step(+1); break;
			case KeyCode::left : col_step(-1); break;
			case KeyCode::right: col_step(+1); break;
			case KeyCode::home: row_abs(0); break;
			case KeyCode::end : row_abs(max_ce(0, row_count() - 1)); break;
			case KeyCode::pgup:
				row_step(min_ce(0, -view().fit_count - 1)); break;
			case KeyCode::pgdn:
				row_step(max_ce(0, +view().fit_count - 1)); break;

			default:
				switch (event.key_input.cp) {
				case 'k': row_step(-1); break;
				case 'j': row_step(+1); break;
				case 'h': col_step(-1); break;
				case 'l': col_step(+1); break;
				case 'a': select_all(); break;
				case 'A': select_none(); break;
				case 's': select_toggle(m_cursor.row, 1); break;
				default:
					handled = false;
					break;
				}
			}
			return handled;
		}
		break;

	case UI::EventType::focus_changed:
		queue_cell_render(
			m_cursor.row, m_cursor.row + 1,
			m_cursor.col, m_cursor.col + 1
		);
		if (is_focused()) {
			App::instance.m_ui.csline->set_location(
				std::to_string(m_cursor.row) + ", " +
				std::to_string(m_cursor.col)
			);
		} else {
			App::instance.m_ui.csline->clear_location();
		}
		return false;

	default:
		break;
	}
	return false;
}

void
BasicGrid::render_impl(
	UI::Widget::RenderData& rd
) noexcept {
	UI::GridRenderData grid_rd{
		rd,
		is_focused(),
		is_focused()
	};
	render_view(
		grid_rd,
		!enum_cast(queued_actions() & UI::UpdateActions::flag_noclear)
	);

	auto const& view = this->view();
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
BasicGrid::content_action(
	UI::ProtoGrid::ContentAction action,
	UI::index_type row_begin,
	UI::index_type count
) noexcept {
	using CA = UI::ProtoGrid::ContentAction;

	DUCT_ASSERTE(row_count() == signed_cast(m_sel.size()));
	// Cast insert_after in terms of insert_before
	if (CA::insert_after == action) {
		++row_begin;
	}
	row_begin = value_clamp(row_begin, 0, row_count());
	auto const row_end = min_ce(row_begin + count, row_count());
	auto clear_flag = UI::UpdateActions::none;
	switch (action) {
	// Select
	case CA::select: // fall-through
	case CA::unselect: {
		bool const enable = CA::select == action;
		auto const end = m_sel.begin() + row_end;
		for (auto it = m_sel.begin() + row_begin; end > it; ++it) {
			*it = enable;
		}
		queue_cell_render(row_begin, row_end);
		clear_flag = UI::UpdateActions::flag_noclear;
	}	break;

	case CA::select_toggle: {
		auto const end = m_sel.begin() + row_end;
		for (auto it = m_sel.begin() + row_begin; end > it; ++it) {
			*it = !*it;
		}
		queue_cell_render(row_begin, row_end);
		clear_flag = UI::UpdateActions::flag_noclear;
	}	break;

	// Insert
	case CA::insert_after: // fall-through
	case CA::insert_before:
		for (UI::index_type i = 0; i < count; ++i) {
			if (content_insert(row_begin + i)) {
				m_sel.insert(m_sel.begin() + row_begin + i, false);
				content_action_internal(CA::insert_before, row_begin + i, 1);
			}
		}
		if (row_count() == 1) {
			set_cursor(m_cursor.col, 0);
		} else if (row_begin <= m_cursor.row) {
			set_cursor(m_cursor.col, m_cursor.row + count);
		}
		adjust_view();
		break;

	// Erase
	case CA::erase:
		for (UI::index_type i = 0; i < count; ++i) {
			if (content_erase(row_begin)) {
				m_sel.erase(m_sel.cbegin() + row_begin);
				content_action_internal(CA::erase, row_begin, 1);
			}
		}
		break;

	case CA::erase_selected:
		for (UI::index_type index = 0; index < row_count();) {
			if (m_sel[index] && content_erase(index)) {
				m_sel.erase(m_sel.cbegin() + index);
				content_action_internal(CA::erase, index, 1);
			} else {
				++index;
			}
		}
		break;
	} // switch (action)

	// Post action
	switch (action) {
	case CA::erase:
	case CA::erase_selected:
		// Let cursor clamp to new bounds
		set_cursor(m_cursor.col, m_cursor.row);
		adjust_view();
		break;

	default:
		break;
	}
	enqueue_actions(
		UI::UpdateActions::render |
		clear_flag
	);
}

void
BasicGrid::adjust_view() noexcept {
	auto const& view = this->view();
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
			col_count(),
			true
		);
		enqueue_actions(UI::UpdateActions::render);
	}
}

void
BasicGrid::set_cursor(
	UI::index_type col,
	UI::index_type row
) noexcept {
	col = value_clamp(col, 0, max_ce(0, col_count() - 1));
	row = value_clamp(row, 0, max_ce(0, row_count() - 1));
	if (col != m_cursor.col || row != m_cursor.row) {
		if (
			value_in_bounds(m_cursor.row, 0, row_count()) &&
			value_in_bounds(m_cursor.col, 0, col_count())
		) {
			queue_cell_render(
				m_cursor.row, m_cursor.row + 1,
				m_cursor.col, m_cursor.col + 1
			);
		}
		if (row_count() > 0) {
			queue_cell_render(
				row, row + 1,
				col, col + 1
			);
		}
		enqueue_actions(
			UI::UpdateActions::render |
			UI::UpdateActions::flag_noclear
		);
		m_cursor.col = col;
		m_cursor.row = row;
		adjust_view();
		if (is_focused()) {
			App::instance.m_ui.csline->set_location(
				std::to_string(m_cursor.row) + ", " +
				std::to_string(m_cursor.col)
			);
		}
	}
}

void
BasicGrid::resize_grid(
	UI::index_type new_col_count,
	UI::index_type new_row_count
) {
	m_sel.resize(new_row_count);
	set_row_count(new_row_count);
	set_col_count(new_col_count);
	set_cursor(0, 0);
	update_view(
		0, view().fit_count,
		0, col_count(),
		false
	);
	queue_header_render();
	queue_cell_render(0, row_count());
	enqueue_actions(
		UI::UpdateActions::render
	);
}

} // namespace UI
} // namespace Onsang
