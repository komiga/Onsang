/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief UI definitions.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/utility.hpp>

#include <Beard/geometry.hpp>
#include <Beard/ui/Defs.hpp>
#include <Beard/ui/Widget/Defs.hpp>

namespace Onsang {
namespace UI {

using namespace Beard;
using namespace Beard::ui;

enum class OnsangWidgetType : unsigned {
	BASE = enum_cast(UI::Widget::Type::USERSPACE_BASE) + 0xFF00,
	TabbedContainer,
	CommandStatusLine,
	TableGrid,
	TableSchemaEditor,
	SessionView,
	ObjectView,
	PropView,
};

/*#define DPROP_(name) \
	property_ ## name = UI::hash(DUCT_STRINGIFY(name))

enum : UI::property_hash_type {
};

#undef DPROP_*/

#define DGROUP_(name) \
	group_ ## name = UI::hash(DUCT_STRINGIFY(name))

enum : UI::group_hash_type {
	// CommandStatusLine
	DGROUP_(csl),
};

#undef DGROUP_

} // namespace UI
} // namespace Onsang
