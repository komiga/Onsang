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
	field_slug->signal_user_modified.bind([&object_view](
		UI::Field::SPtr field_slug,
		bool const accept
	) {
		auto& object = object_view.m_object;
		if (accept) {
			Hord::Cmd::Object::SetSlug cmd{object_view.m_session};
			if (!cmd(object, field_slug->get_text())) {
				field_slug->set_text(object.get_slug());
			}
		} else {
			field_slug->set_text(object.get_slug());
		}
	});
	field_slug->signal_event_filter.bind(UI::FieldDescriber{"slug"});

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
	grid_metadata->signal_event_filter.bind(UI::FieldDescriber{"metadata"});

	auto view = UI::PropView::make(root, "base", UI::Axis::vertical);
	view->push_back(std::move(field_slug));
	view->push_back(std::move(grid_metadata));
	object_view.add_prop_view(std::move(view), index);
}

} // namespace UI
} // namespace Onsang
