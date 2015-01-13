/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/FieldDescriber.hpp>
#include <Onsang/UI/TableGrid.hpp>
#include <Onsang/UI/ObjectView.hpp>
#include <Onsang/UI/PropView.hpp>

#include <Hord/Data/Defs.hpp>
#include <Hord/Data/Table.hpp>
#include <Hord/IO/Defs.hpp>
#include <Hord/Object/Defs.hpp>
#include <Hord/Table/Defs.hpp>
#include <Hord/Table/Unit.hpp>
#include <Hord/Cmd/Object.hpp>

namespace Onsang {
namespace UI {

static void
add_table_data_prop_view(
	UI::RootSPtr root,
	UI::ObjectView& object_view,
	System::Session& session,
	Hord::Table::Unit& table,
	unsigned const index
) {
	// Data property
	auto grid_data = UI::TableGrid::make(
		root, session, table,
		table.get_data(),
		Hord::IO::PropType::primary
	);
	grid_data->signal_event_filter.bind(UI::FieldDescriber{"data"});

	auto view = UI::PropView::make(root, "data", UI::Axis::vertical);
	view->push_back(std::move(grid_data));
	object_view.add_prop_view(std::move(view), index);
}

void
add_data_prop_view(
	UI::ObjectView& object_view,
	unsigned const index
) {
	auto const root = object_view.get_root();
	auto& session = object_view.m_session;
	auto& object = object_view.m_object;

	switch (object.get_base_type()) {
	case Hord::Object::BaseType::Table:
		UI::add_table_data_prop_view(
			root, object_view, session, static_cast<Hord::Table::Unit&>(object), index
		);
		break;

	default: break;
	}
}

} // namespace UI
} // namespace Onsang
