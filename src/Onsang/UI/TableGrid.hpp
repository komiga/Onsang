/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Table editor widget.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/BareField.hpp>
#include <Onsang/UI/BasicGrid.hpp>

#include <Beard/keys.hpp>
#include <Beard/txt/Defs.hpp>
#include <Beard/txt/Tree.hpp>
#include <Beard/txt/Cursor.hpp>
#include <Beard/ui/Geom.hpp>

namespace Onsang {
namespace UI {

class TableGrid final
	: public UI::BasicGrid
{
public:
	using SPtr = aux::shared_ptr<UI::TableGrid>;

private:
	using base = UI::BasicGrid;
	enum class ctor_priv {};

public:
	// System::Session& m_session;
	Hord::Object::Unit& m_object;
	Hord::Data::Table& m_table;
	Hord::IO::PropType m_prop_type;
	UI::BareField m_field{};
	Hord::Data::Type m_field_type{};

private:
// UI::Widget::Base implementation
	void
	set_input_control_impl(
		bool const enabled
	) noexcept override;

	void
	reflow_impl() noexcept override;

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
		UI::index_type row
	) noexcept override;

	bool
	content_erase(
		UI::index_type row
	) noexcept override;

// -
	void
	reflow_field() noexcept;

	bool
	field_input(
		char32 cp
	) noexcept;

private:
	TableGrid() noexcept = delete;
	TableGrid(TableGrid const&) = delete;
	TableGrid& operator=(TableGrid const&) = delete;

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
			(
				UI::Widget::Flags::trait_focusable |
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
		p->set_cursor(0, 0);
		return p;
	}

	TableGrid(TableGrid&&) = default;
	TableGrid& operator=(TableGrid&&) = default;
};

} // namespace UI
} // namespace Onsang
