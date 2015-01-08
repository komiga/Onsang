/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief TableGrid.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/BasicGrid.hpp>

#include <Beard/ui/Field.hpp>

namespace Onsang {
namespace UI {

class TableGrid final
	: public UI::BasicGrid
{
public:
	using SPtr = aux::shared_ptr<UI::TableGrid>;

private:
	using base = UI::BasicGrid;

private:
	enum class ctor_priv {};

	TableGrid() noexcept = delete;
	TableGrid(TableGrid const&) = delete;
	TableGrid& operator=(TableGrid const&) = delete;

	// System::Session& m_session;
	Hord::Object::Unit& m_object;
	Hord::Data::Table& m_table;
	Hord::IO::PropType m_prop_type;
	UI::Field::SPtr m_field;

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
		UI::index_type const row_begin,
		UI::index_type const row_end,
		UI::index_type const col_begin,
		UI::index_type const col_end,
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
	~TableGrid() noexcept override = default;

	TableGrid(
		ctor_priv const,
		UI::group_hash_type const group,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent,
		System::Session& /*session*/,
		Hord::Object::Unit& object,
		Hord::Data::Table& table,
		Hord::IO::PropType const prop_type
	) noexcept
		: base(
			static_cast<UI::Widget::Type>(UI::OnsangWidgetType::TableGrid),
			enum_combine(
				UI::Widget::Flags::trait_focusable,
				UI::Widget::Flags::visible
			),
			group,
			{{0, 0}, true, Axis::both, Axis::both},
			std::move(root),
			std::move(parent),
			table.num_columns(),
			table.num_records()
		)
		// , m_session(session)
		, m_object(object)
		, m_table(table)
		, m_prop_type(prop_type)
		, m_field()
	{}

	static UI::TableGrid::SPtr
	make(
		UI::RootWPtr root,
		System::Session& session,
		Hord::Object::Unit& object,
		Hord::Data::Table& table,
		Hord::IO::PropType const prop_type,
		UI::group_hash_type const group = UI::group_default,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		auto p = aux::make_shared<TableGrid>(
			ctor_priv{},
			group,
			std::move(root),
			std::move(parent),
			session,
			object,
			table,
			prop_type
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

	TableGrid(TableGrid&&) = default;
	TableGrid& operator=(TableGrid&&) = default;
};

} // namespace UI
} // namespace Onsang
