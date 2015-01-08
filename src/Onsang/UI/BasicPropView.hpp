/**
@file
@brief Basic prop view.

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
#include <Onsang/UI/TableGrid.hpp>

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

/**
	This prop view includes the identity and metadata props.
*/
void
add_basic_prop_view(
	aux::shared_ptr<UI::ObjectView> const& obj_view_ptr,
	unsigned const index = static_cast<unsigned>(-1)
) {
	auto& obj_view = *obj_view_ptr;
	auto const root = obj_view.get_root();
	auto& session = obj_view.get_session();
	auto& object = obj_view.get_object();

	auto const cont = UI::Container::make(root, UI::Axis::vertical);
	cont->get_geometry().set_sizing(
		UI::Axis::both,
		UI::Axis::both
	);

	// Slug property
	auto field_slug = UI::Field::make(root, object.get_slug());
	field_slug->get_geometry().set_sizing(
		UI::Axis::horizontal,
		UI::Axis::horizontal
	);
	field_slug->signal_control_changed.bind([&obj_view](
		aux::shared_ptr<UI::Field> field_slug,
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
		assert(cmd(object, "null", nullptr, true));
		assert(cmd(object, "integer", 1234567890, true));
		assert(cmd(object, "decimal", 3.14156f, true));
		assert(cmd(object, "object_id", object.get_id(), true));
		assert(cmd(object, "string", "string value", true));
	}
	auto grid_metadata = UI::TableGrid::make(
		root, session, object,
		object.get_metadata().table(),
		Hord::IO::PropType::metadata
	);

	cont->push_back(std::move(field_slug));
	cont->push_back(std::move(grid_metadata));
	obj_view.add_view("basic", std::move(cont), index);
}

} // namespace UI
} // namespace Onsang
