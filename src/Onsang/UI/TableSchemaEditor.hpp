/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Table schema editor widget.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/BareField.hpp>
#include <Onsang/UI/BasicGrid.hpp>

#include <Beard/txt/Defs.hpp>
#include <Beard/ui/Root.hpp>

#include <Hord/Data/Defs.hpp>
#include <Hord/Data/TableSchema.hpp>
#include <Hord/Data/Table.hpp>
#include <Hord/Table/Unit.hpp>

namespace Onsang {
namespace UI {

class TableSchemaEditor
	: public UI::BasicGrid
{
public:
	using SPtr = aux::shared_ptr<UI::TableSchemaEditor>;

	enum : unsigned {
		NUM_COLUMNS = 5,
	};
	struct Data {
		Hord::Data::TableSchema::Column orig;
		Hord::Data::TableSchema::Column edit;
		duct::StateStore<unsigned, unsigned> modified;
	};

private:
	using base = UI::BasicGrid;
	enum class ctor_priv {};

public:
	System::Session& m_session;
	Hord::Table::Unit& m_table;
	aux::vector<Data> m_data{};

	UI::BareField m_field{};

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

	txt::Sequence
	get_cell_seq(
		UI::index_type row,
		UI::index_type col,
		String& scratch
	) noexcept;

private:
	TableSchemaEditor() noexcept = delete;
	TableSchemaEditor(TableSchemaEditor const&) = delete;
	TableSchemaEditor& operator=(TableSchemaEditor const&) = delete;

public:
// special member functions
	~TableSchemaEditor() noexcept override = default;

	TableSchemaEditor(
		ctor_priv const,
		UI::RootWPtr root,
		UI::Widget::WPtr&& parent,
		System::Session& session,
		Hord::Table::Unit& table
	) noexcept
		: base(
			static_cast<UI::Widget::Type>(UI::OnsangWidgetType::TableSchemaEditor),
			(
				UI::Widget::Flags::trait_focusable |
				UI::Widget::Flags::visible
			),
			UI::group_null,
			{{0, 0}, true, UI::Axis::both, UI::Axis::both},
			std::move(root),
			std::move(parent),
			NUM_COLUMNS,
			0
		)
		, m_session(session)
		, m_table(table)
	{}

	TableSchemaEditor(TableSchemaEditor&&) = default;
	TableSchemaEditor& operator=(TableSchemaEditor&&) = default;

	static UI::TableSchemaEditor::SPtr
	make(
		UI::RootWPtr root,
		System::Session& session,
		Hord::Table::Unit& table,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		auto widget = aux::make_shared<UI::TableSchemaEditor>(
			ctor_priv{},
			std::move(root),
			std::move(parent),
			session,
			table
		);
		widget->set_cursor(0, 0);
		widget->reset();
		return widget;
	}

// operations
	void
	reset();

	void
	cursor_step_value(
		bool const prev
	);
};

} // namespace UI
} // namespace Onsang
