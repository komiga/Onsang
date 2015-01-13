/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TableGrid.hpp>
#include <Onsang/UI/ObjectView.hpp>
#include <Onsang/UI/FieldDescriber.hpp>
#include <Onsang/UI/PropView.hpp>

#include <Beard/ui/Container.hpp>
#include <Beard/ui/Field.hpp>

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
	UI::ObjectView& obj_view,
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

	auto const cont = UI::Container::make(root, UI::Axis::vertical);
	cont->get_geometry().set_sizing(UI::Axis::both, UI::Axis::both);
	cont->push_back(std::move(grid_data));
	obj_view.add_prop_view("data", std::move(cont), index);
}

void
add_data_prop_view(
	UI::ObjectView::SPtr const& obj_view_ptr,
	unsigned const index
) {
	auto& obj_view = *obj_view_ptr;
	auto const root = obj_view.get_root();
	auto& session = obj_view.m_session;
	auto& object = obj_view.m_object;

	switch (object.get_base_type()) {
	case Hord::Object::BaseType::Table:
		UI::add_table_data_prop_view(
			root, obj_view, session, static_cast<Hord::Table::Unit&>(object), index
		);
		break;

	default: break;
	}
}

} // namespace UI
} // namespace Onsang
