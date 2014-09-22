/**
@file UI/BasicPropView.hpp
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
#include <Onsang/UI/Grid.hpp>

#include <Beard/ui/Container.hpp>
#include <Beard/ui/Field.hpp>

#include <Hord/Object/Defs.hpp>

namespace Onsang {
namespace UI {

void metadata_grid_control_get_cell(
	Hord::Object::Unit& object,
	UI::index_type row,
	UI::index_type col,
	String& value
) {
	static char const* const header[]{"type", "name", "value"};

	auto const& fields = object.get_metadata().fields;
	assert(-1 == row || static_cast<signed>(fields.size()) > row);
	assert(2 >= col);
	if (-1 == row) {
		value.assign(header[col]);
		return;
	}
	auto const& f = fields[row];
	if (0 == col) {
		switch (f->get_type_info().type) {
		case Hord::Data::MetaFieldType::String: value.assign("String"); break;
		case Hord::Data::MetaFieldType::Int32: value.assign("Int32"); break;
		case Hord::Data::MetaFieldType::Int64: value.assign("Int64"); break;
		case Hord::Data::MetaFieldType::Bool: value.assign("Bool"); break;
		default: assert(false); break;
		}
	} else if (1 == col) {
		value.assign(f->name);
	} else {
		switch (f->get_type_info().type) {
		case Hord::Data::MetaFieldType::String:
			value.assign(static_cast<Hord::Data::StringMetaField const&>(*f).value);
			break;

		case Hord::Data::MetaFieldType::Int32:
			value.assign(std::to_string(
				static_cast<Hord::Data::Int32MetaField const&>(*f).value
			));
			break;

		case Hord::Data::MetaFieldType::Int64:
			value.assign(std::to_string(
				static_cast<Hord::Data::Int64MetaField const&>(*f).value
			));
			break;

		case Hord::Data::MetaFieldType::Bool:
			value.assign(
				static_cast<Hord::Data::BoolMetaField const&>(*f).value
				? "true"
				: "false"
			);
			break;

		default:
			assert(false);
			break;
		}
	}
}

void metadata_grid_control_set_cell(
	Hord::Object::Unit& object,
	UI::index_type row,
	UI::index_type col,
	String const& value
) {
	auto& fields = object.get_metadata().fields;
	assert(static_cast<signed>(fields.size()) > row);
	assert(2 >= col);
	auto& f = fields[row];
	if (0 == col) {
		return;
	} else if (1 == col) {
		f->name.assign(value);
	} else {
		switch (f->get_type_info().type) {
		case Hord::Data::MetaFieldType::String:
			static_cast<Hord::Data::StringMetaField&>(
				*f
			).value.assign(value);
			break;

		case Hord::Data::MetaFieldType::Int32:
			static_cast<Hord::Data::Int32MetaField&>(
				*f
			).value = std::stoi(value, nullptr, 10);
			break;

		case Hord::Data::MetaFieldType::Int64:
			static_cast<Hord::Data::Int64MetaField&>(
				*f
			).value = std::stoll(value, nullptr, 10);
			break;

		case Hord::Data::MetaFieldType::Bool:
			static_cast<Hord::Data::BoolMetaField&>(
				*f
			).value = (0 == value.compare("true"));
			break;

		default:
			assert(false);
			break;
		}
	}
	object.get_prop_states().assign(
		Hord::IO::PropType::metadata,
		Hord::IO::PropState::modified
	);
}

/**
	This prop view includes the identity and metadata props.
*/
void
add_basic_prop_view(
	aux::shared_ptr<UI::ObjectView> const& obj_view,
	unsigned const index = static_cast<unsigned>(-1)
) {
	auto const root = obj_view->get_root();
	// auto& hive = obj_view->get_session().get_hive();
	auto& object = obj_view->get_object();

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
	field_slug->signal_control_changed.bind([&object](
		aux::shared_ptr<UI::Field> field_slug,
		bool const has_control
	) {
		/*auto const obj_view = std::shared_pointer_cast<UI::ObjectView>(
			field_slug->get_parent()
		);*/
		if (!has_control) {
			auto const value = field_slug->get_text();
			if (value.empty()) {
				field_slug->set_text(object.get_slug());
			} else if (value != object.get_slug()) {
				// TODO: Use a "set slug" command
				object.set_slug(value);
				object.get_prop_states().assign(
					Hord::IO::PropType::identity,
					Hord::IO::PropState::modified
				);
			}
		}
	});

	// Metadata property
	if (object.get_metadata().fields.empty()) {
		object.get_metadata().fields.push_back(
			duct::cc_unique_ptr<Hord::Data::MetaField>{
				new Hord::Data::StringMetaField("test_name", "test_value")
			}
		);
		object.get_prop_states().assign(
			Hord::IO::PropType::metadata,
			Hord::IO::PropState::modified
		);
	}
	auto grid_metadata = UI::Grid::make(
		root,
		object,
		metadata_grid_control_get_cell,
		metadata_grid_control_set_cell,
		3, object.get_metadata().fields.size()
	);

	cont->push_back(std::move(field_slug));
	cont->push_back(std::move(grid_metadata));
	obj_view->add_view("basic", std::move(cont), index);
}

} // namespace UI
} // namespace Onsang
