/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Basic prop view.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/ObjectView.hpp>

namespace Onsang {
namespace UI {

/**
	This prop view includes the identity and metadata props.
*/
void
add_basic_prop_view(
	UI::ObjectView::SPtr const& obj_view_ptr,
	unsigned const index = static_cast<unsigned>(-1)
);

} // namespace UI
} // namespace Onsang
