/**
@file UI/Defs.hpp
@brief UI definitions.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/utility.hpp>

#include <Beard/geometry.hpp>
#include <Beard/ui/Defs.hpp>
#include <Beard/ui/Widget/Defs.hpp>

namespace Onsang {
namespace UI {

// Forward declarations
enum class OnsangWidgetType : unsigned;

using namespace Beard;
using namespace Beard::ui;

enum class OnsangWidgetType : unsigned {
	BASE = enum_cast(UI::Widget::Type::USERSPACE_BASE) + 0xFF00,
	TabbedContainer,
	ObjectView,
	TableGrid,
};

} // namespace UI
} // namespace Onsang
