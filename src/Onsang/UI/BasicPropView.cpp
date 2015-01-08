/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TableGrid.hpp>
#include <Onsang/UI/ObjectView.hpp>
#include <Onsang/UI/BasicPropView.hpp>

#include <Beard/ui/Container.hpp>
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
add_basic_prop_view(
	UI::ObjectView::SPtr const& obj_view_ptr,
	unsigned const index
) {
	auto& obj_view = *obj_view_ptr;
	auto const root = obj_view.get_root();
	auto& session = obj_view.get_session();
	auto& object = obj_view.get_object();

	auto const cont = UI::Container::make(root, UI::Axis::vertical);
	cont->get_geometry().set_sizing(UI::Axis::both, UI::Axis::both);

	// Slug property
	auto field_slug = UI::Field::make(root, object.get_slug());
	field_slug->get_geometry().set_sizing(UI::Axis::horizontal, UI::Axis::horizontal);
	field_slug->signal_control_changed.bind([&obj_view](
		UI::Field::SPtr field_slug,
		bool const has_control
	) {
		if (!has_control) {
			auto& object = obj_view.get_object();
			Hord::Cmd::Object::SetSlug cmd{obj_view.get_session()};
			if (!cmd(object, field_slug->get_text())) {
				field_slug->set_text(object.get_slug());
			}
		}
	});

	// Metadata property
	if (object.get_metadata().num_fields() == 0) {
		Hord::Cmd::Object::SetMetaField cmd{session};
		cmd(object, "null", nullptr);
		cmd(object, "integer", 1234567890);
		cmd(object, "decimal", 3.14156f);
		cmd(object, "object_id", object.get_id());
		cmd(object, "string", "string-value");
	}
	auto grid_metadata = UI::TableGrid::make(
		root, session, object,
		object.get_metadata().table(),
		Hord::IO::PropType::metadata
	);

	cont->push_back(std::move(field_slug));
	cont->push_back(std::move(grid_metadata));
	obj_view.add_prop_view("basic", std::move(cont), index);
}

} // namespace UI
} // namespace Onsang
