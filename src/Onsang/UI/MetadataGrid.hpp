/**
@file UI/MetadataGrid.hpp
@brief MetadataGrid.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/ObjectView.hpp>
#include <Onsang/UI/BasicGrid.hpp>

#include <Beard/txt/Defs.hpp>
#include <Beard/ui/Field.hpp>

#include <Hord/Data/Metadata.hpp>
#include <Hord/Cmd/Data.hpp>

#define GRID_TMP_COLUMN_WIDTH 25

namespace Onsang {
namespace UI {

class MetadataGrid final
	: public UI::BasicGrid
{
private:
	using base = UI::BasicGrid;

private:
	enum : UI::index_type {
		COLUMN_IDX_TYPE = 0,
		COLUMN_IDX_NAME = 1,
		COLUMN_IDX_VALUE = 2,
	};

	enum class ctor_priv {};

	MetadataGrid() noexcept = delete;
	MetadataGrid(MetadataGrid const&) = delete;
	MetadataGrid& operator=(MetadataGrid const&) = delete;

	System::Session& m_session;
	Hord::Object::Unit& m_object;
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
		UI::index_type col_end,
		Rect const& frame
	) noexcept override;

// BasicGrid implementation
	bool
	content_insert(
		UI::index_type row_begin,
		UI::index_type count
	) noexcept override;

	bool
	content_erase(
		UI::index_type row_begin,
		UI::index_type count
	) noexcept override;

private:
	void
	reflow_field(
		bool const cache
	) noexcept;

public:
	~MetadataGrid() noexcept override = default;

	MetadataGrid(
		ctor_priv const,
		UI::group_hash_type const group,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent,
		System::Session& session,
		Hord::Object::Unit& object
	) noexcept
		: base(
			static_cast<UI::Widget::Type>(UI::OnsangWidgetType::MetadataGrid),
			enum_combine(
				UI::Widget::Flags::trait_focusable,
				UI::Widget::Flags::visible
			),
			group,
			{{0, 0}, true, Axis::both, Axis::both},
			std::move(root),
			std::move(parent),
			3,
			object.get_metadata().fields.size()
		)
		, m_session(session)
		, m_object(object)
		, m_field()
	{}

	static aux::shared_ptr<MetadataGrid>
	make(
		UI::RootWPtr root,
		System::Session& session,
		Hord::Object::Unit& object,
		UI::group_hash_type const group = UI::group_default,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		auto p = aux::make_shared<MetadataGrid>(
			ctor_priv{},
			group,
			std::move(root),
			std::move(parent),
			session,
			object
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

	MetadataGrid(MetadataGrid&&) = default;
	MetadataGrid& operator=(MetadataGrid&&) = default;
};

void
MetadataGrid::reflow_impl(
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
MetadataGrid::handle_event_impl(
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
				set_input_control(false);
				m_field->clear_actions();
				// m_control.set_cell(m_object, m_cursor.row, m_cursor.col, m_field->get_text());
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
				if (0 == get_row_count() || COLUMN_IDX_TYPE == m_cursor.col) {
					return true;
				}
				auto& field = m_object.get_metadata().fields[m_cursor.row];
				// TODO: Type editing? (column 0)
				if (COLUMN_IDX_NAME == m_cursor.col) {
					m_field->set_text(field.name);
				} else {
					switch (field.value.type) {
					case Hord::Data::FieldType::Text:
						m_field->set_text(field.value.str);
						break;

					case Hord::Data::FieldType::Number:
						m_field->set_text(std::to_string(field.value.num));
						break;

					case Hord::Data::FieldType::Boolean:
						// Value toggle
						if (Hord::Cmd::Data::SetMetaField{m_session}(
							m_object,
							static_cast<unsigned>(m_cursor.row),
							{!field.value.bin}
						)) {
							queue_cell_render(
								m_cursor.row, m_cursor.row + 1,
								m_cursor.col, m_cursor.col + 1
							);
							queue_actions(enum_combine(
								UI::UpdateActions::flag_noclear,
								UI::UpdateActions::render
							));
						}
						return true;
					}
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
MetadataGrid::render_impl(
	UI::Widget::RenderData& rd
) noexcept {
	base::render_impl(rd);
	if (has_input_control()) {
		rd.update_group(m_field->get_group());
		m_field->render(rd);
	}
}

void
MetadataGrid::render_header(
	UI::GridRenderData& grid_rd,
	UI::index_type const col_begin,
	UI::index_type const col_end,
	Rect const& frame
) noexcept {
	static constexpr txt::Sequence const s_header_seq[]{
		"type", "name", "value"
	};

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
	for (auto col = col_begin; col_end > col; ++col) {
		cell_frame.size.width = min_ce(
			GRID_TMP_COLUMN_WIDTH,
			frame.pos.x + frame.size.width - cell_frame.pos.x
		);
		rd.terminal.put_sequence(
			cell_frame.pos.x,
			cell_frame.pos.y,
			s_header_seq[col],
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
MetadataGrid::render_content(
	UI::GridRenderData& grid_rd,
	UI::index_type const row,
	UI::index_type const col_begin,
	UI::index_type col_end,
	Rect const& frame
) noexcept {
	// NB: Holes due to Hord::Data::FieldType being a bitflag enum
	static constexpr txt::Sequence const s_type_seq[]{
		"", "Text", "Number", "", "Boolean"
	};
	static constexpr txt::Sequence const s_bool_seq[]{
		"false", "true"
	};

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
	auto cell = tty::make_cell(' ', attr_fg, attr_bg);
	auto const& field = m_object.get_metadata().fields[row];
	String value;
	txt::Sequence seq;
	col_end = max_ce(get_col_count(), col_end);
	for (UI::index_type col = col_begin; col_end > col; ++col) {
		cell_frame.size.width = min_ce(
			GRID_TMP_COLUMN_WIDTH,
			frame.pos.x + frame.size.width - cell_frame.pos.x
		);
		cell.attr_bg
			= (
				row == m_cursor.row &&
				col == m_cursor.col &&
				is_focused()
			)
			? attr_bg | tty::Attr::inverted
			: attr_bg
		;
		rd.terminal.put_line(
			cell_frame.pos,
			cell_frame.size.width,
			Axis::horizontal,
			cell
		);
		if (COLUMN_IDX_TYPE == col) {
			seq = s_type_seq[enum_cast(field.value.type)];
		} else if (COLUMN_IDX_NAME == col) {
			seq = {field.name};
		} else { // COLUMN_IDX_VALUE
			switch (field.value.type) {
			case Hord::Data::FieldType::Text:
				seq = {field.value.str};
				break;
			case Hord::Data::FieldType::Number:
				value.assign(std::to_string(field.value.num));
				seq = {value};
				break;
			case Hord::Data::FieldType::Boolean:
				seq = s_bool_seq[unsigned{field.value.bin}];
				break;
			}
		}
		rd.terminal.put_sequence(
			cell_frame.pos.x,
			cell_frame.pos.y,
			seq,
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

bool
MetadataGrid::content_insert(
	UI::index_type /*row_begin*/,
	UI::index_type /*count*/
) noexcept {
	// TODO
	return false;
}

bool
MetadataGrid::content_erase(
	UI::index_type /*row_begin*/,
	UI::index_type /*count*/
) noexcept {
	// TODO
	return false;
}

void
MetadataGrid::reflow_field(
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

} // namespace UI
} // namespace Onsang

#undef GRID_TMP_COLUMN_WIDTH
