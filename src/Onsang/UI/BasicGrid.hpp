/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief BasicGrid.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/UI/Defs.hpp>

#include <Beard/ui/Widget/Base.hpp>
#include <Beard/ui/ProtoGrid.hpp>

namespace Onsang {
namespace UI {

class BasicGrid
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
	BasicGrid() noexcept = delete;
	BasicGrid(BasicGrid const&) = delete;
	BasicGrid& operator=(BasicGrid const&) = delete;

protected:
	struct {
		UI::index_type col{-1};
		UI::index_type row{-1};
	} m_cursor{};

	row_vector_type m_rows;

protected:
// UI::Widget::Base implementation
	virtual void
	reflow_impl(
		Rect const& area,
		bool const cache
	) noexcept override;

	virtual bool
	handle_event_impl(
		UI::Event const& event
	) noexcept override;

	virtual void
	render_impl(
		UI::Widget::RenderData& rd
	) noexcept override;

private:
// UI::ProtoGrid implementation
	void
	content_action(
		UI::ProtoGrid::ContentAction action,
		UI::index_type row_begin,
		UI::index_type count
	) noexcept override;

protected:
	// Carried forward
	virtual void
	render_header(
		UI::GridRenderData& grid_rd,
		UI::index_type const col_begin,
		UI::index_type const col_end,
		Rect const& frame
	) noexcept = 0;

	// Carried forward
	virtual void
	render_content(
		UI::GridRenderData& grid_rd,
		UI::index_type row_begin,
		UI::index_type row_end,
		UI::index_type col_begin,
		UI::index_type col_end,
		Rect const& frame
	) noexcept = 0;

protected:
// BasicGrid implementation
	virtual bool
	content_insert(
		UI::index_type row_begin,
		UI::index_type count
	) noexcept = 0;

	virtual bool
	content_erase(
		UI::index_type row_begin,
		UI::index_type count
	) noexcept = 0;

protected:
	void
	adjust_view() noexcept;

public:
	~BasicGrid() noexcept override = default;

	BasicGrid(
		UI::Widget::Type const type,
		UI::Widget::Flags const flags,
		UI::group_hash_type const group,
		ui::Geom&& geometry,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent,
		UI::index_type const col_count,
		UI::index_type const row_count
	) noexcept
		: base(
			type,
			flags,
			group,
			std::move(geometry),
			std::move(root),
			std::move(parent),
			col_count,
			row_count
		)
		, m_rows(row_count)
	{}

	BasicGrid(BasicGrid&&) = default;
	BasicGrid& operator=(BasicGrid&&) = default;

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
BasicGrid::reflow_impl(
	Rect const& area,
	bool const cache
) noexcept {
	base::reflow_impl(area, cache);
	Rect view_frame = get_geometry().get_frame();
	// ++view_frame.pos.x;
	// ++view_frame.pos.y;
	// view_frame.size.width -= 2;
	// view_frame.size.height -= 2;
	reflow_view(view_frame);
	adjust_view();
	queue_header_render();
	queue_cell_render(0, get_row_count());
}

bool
BasicGrid::handle_event_impl(
	UI::Event const& event
) noexcept {
	switch (event.type) {
	case UI::EventType::key_input:
		if (!has_input_control()) {
			bool handled = true;
			switch (event.key_input.code) {
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
BasicGrid::render_impl(
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
	using ContentAction = UI::ProtoGrid::ContentAction;

	assert(get_row_count() == static_cast<signed>(m_rows.size()));
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
		m_rows.insert(
			m_rows.begin() + row_begin,
			static_cast<std::size_t>(count),
			Row{}
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
		adjust_view();
		break;

	// Erase
	case ContentAction::erase:
		if (content_erase(row_begin, count)) {
			m_rows.erase(
				m_rows.cbegin() + row_begin,
				m_rows.cbegin() + row_end
			);
			content_action_internal(
				ContentAction::erase,
				row_begin,
				count
			);
		} else {
			return;
		}
		break;

	case ContentAction::erase_selected:
		UI::index_type
			head = 0,
			tail = head,
			rcount = 0
		;
		while (get_row_count() >= tail) {
			if (
				get_row_count() == tail ||
				!m_rows[tail].states.test(Row::Flags::selected)
			) {
				if (tail > head) {
					rcount = tail - head;
					if (content_erase(head, rcount)) {
						m_rows.erase(
							m_rows.cbegin() + head,
							m_rows.cbegin() + tail
						);
						content_action_internal(ContentAction::erase, head, rcount);
						/*if (head <= m_cursor.row) {
							set_cursor(
								m_cursor.col,
								max_ce(head, m_cursor.row - rcount)
							);
						}*/
						// tail is not selected, so no sense in checking it again
						tail = ++head;
					} else {
						head = ++tail;
					}
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
BasicGrid::adjust_view() noexcept {
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
BasicGrid::set_cursor(
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
			/*m_rows[m_cursor.row][m_cursor.col].states.disable(
				Column::Flags::focused
			);*/
			queue_cell_render(
				m_cursor.row, m_cursor.row + 1,
				m_cursor.col, m_cursor.col + 1
			);
		}
		if (!m_rows.empty()) {
			/*m_rows[row][col].states.enable(
				Column::Flags::focused
			);*/
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
