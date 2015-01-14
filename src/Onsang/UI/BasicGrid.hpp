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
#include <Beard/ui/Root.hpp>
#include <Beard/ui/ProtoGrid.hpp>

namespace Onsang {
namespace UI {

// TODO: Store selected ranges instead of row states

class BasicGrid
	: public UI::ProtoGrid
{
private:
	using base = UI::ProtoGrid;

public:
	struct Row final {
		enum class Flags : std::uint8_t {
			selected = bit(0u)
		};

		duct::StateStore<Flags> states{};
	};

	using row_vector_type = aux::vector<Row>;

	struct {
		UI::index_type col{-1};
		UI::index_type row{-1};
	} m_cursor{};

	row_vector_type m_rows;

private:
	BasicGrid() noexcept = delete;
	BasicGrid(BasicGrid const&) = delete;
	BasicGrid& operator=(BasicGrid const&) = delete;

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
	virtual
	~BasicGrid() noexcept override = 0;

	BasicGrid(
		UI::Widget::Type const type,
		UI::Widget::Flags const flags,
		UI::group_hash_type const group,
		UI::Geom&& geometry,
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
inline BasicGrid::~BasicGrid() noexcept = default;

} // namespace UI
} // namespace Onsang
