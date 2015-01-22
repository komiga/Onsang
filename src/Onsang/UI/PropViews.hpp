/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Prop views.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/PropView.hpp>
#include <Onsang/UI/ObjectView.hpp>

#include <Beard/ui/Root.hpp>
#include <Beard/ui/ProtoSlotContainer.hpp>

namespace Onsang {
namespace UI {

/**
	This prop view includes the identity and metadata props.
*/
void
add_base_prop_view(
	UI::ObjectView& object_view,
	unsigned index = static_cast<unsigned>(-1)
);

/**
	This prop view includes the primary and auxiliary props.
*/
void
add_data_prop_view(
	UI::ObjectView& object_view,
	unsigned index = static_cast<unsigned>(-1)
);

} // namespace UI
} // namespace Onsang
