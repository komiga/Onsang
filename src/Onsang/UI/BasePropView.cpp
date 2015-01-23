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

#include <Beard/ui/Field.hpp>

#include <Hord/Data/Defs.hpp>
#include <Hord/Data/Table.hpp>
#include <Hord/Data/Metadata.hpp>
#include <Hord/IO/Defs.hpp>
#include <Hord/Object/Defs.hpp>
#include <Hord/Cmd/Object.hpp>

namespace Onsang {
namespace UI {

void
add_base_prop_view(
	UI::ObjectView& object_view,
	unsigned const index
) {
	auto const root = object_view.get_root();
	auto& session = object_view.m_session;
	auto& object = object_view.m_object;

	// Slug property
	auto field_slug = UI::Field::make(root, object.get_slug());
	field_slug->get_geometry().set_sizing(UI::Axis::horizontal, UI::Axis::horizontal);
	UI::bind_field_describer(field_slug, "slug");
	field_slug->signal_user_modified.bind([&session, &object](
		UI::Field::SPtr field_slug,
		bool const accept
	) {
		if (accept) {
			// NB: signal_notify_command handles result
			Hord::Cmd::Object::SetSlug{session}(
				object, field_slug->get_text()
			);
		} else {
			field_slug->set_text(object.get_slug());
		}
	});

	// Metadata property
	auto grid_metadata = UI::TableGrid::make(
		root, session, object,
		object.get_metadata().table()
	);
	UI::bind_field_describer(grid_metadata, "metadata");
	grid_metadata->signal_cell_edited.bind([&session, &object](
		Hord::Data::Table::Iterator& it,
		UI::index_type col,
		String const& string_value,
		Hord::Data::ValueRef& new_value
	) {
		if (col == 0) {
			Hord::Cmd::Object::RenameMetaField{session}(
				object, it.index, string_value
			);
		} else if (col == 1) {
			Hord::Cmd::Object::SetMetaField{session}(
				object, it.index, new_value
			);
		}
	});

	auto view = UI::PropView::make(root, "base", UI::Axis::vertical);
	auto& field_slug_ref = *field_slug;
	auto& grid_metadata_ref = *grid_metadata;
	view->signal_notify_command.bind([&object, &field_slug_ref, &grid_metadata_ref](
		UI::View* const /*parent_view*/,
		UI::PropView& /*prop_view*/,
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::type_info const& type_info
	) {
		if (
			command.get_object_id() != object.get_id() ||
			!command.ok_action()
		) {
			return;
		}
		switch (type_info.id) {
		case Hord::Cmd::Object::SetSlug::COMMAND_ID:
			field_slug_ref.set_text(object.get_slug());
			break;

		case Hord::Cmd::Object::SetMetaField::COMMAND_ID: {
			auto const& c = static_cast<Hord::Cmd::Object::SetMetaField const&>(command);
			if (c.created()) {
				grid_metadata_ref.insert_before(c.get_field_index(), 1);
			} else {
				grid_metadata_ref.notify_cell_changed(c.get_field_index(), 1);
			}
		}	break;

		case Hord::Cmd::Object::RenameMetaField::COMMAND_ID: {
			auto const& c = static_cast<Hord::Cmd::Object::RenameMetaField const&>(command);
			grid_metadata_ref.notify_cell_changed(c.get_field_index(), 0);
		}	break;

		case Hord::Cmd::Object::RemoveMetaField::COMMAND_ID: {
			auto const& c = static_cast<Hord::Cmd::Object::RemoveMetaField const&>(command);
			grid_metadata_ref.erase(c.get_field_index(), 1);
		}	break;
		}
	});

	view->push_back(std::move(field_slug));
	view->push_back(std::move(grid_metadata));
	object_view.add_prop_view(std::move(view), index);
}

} // namespace UI
} // namespace Onsang
