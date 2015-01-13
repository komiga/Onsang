/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/config.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/View.hpp>
#include <Onsang/App.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

void
View::set_sub_view(
	unsigned index
) noexcept {
	if (index < num_sub_views() && index != sub_view_index()) {
		set_sub_view_impl(index);
		auto const sv = sub_view();
		if (sv) {
			App::instance.m_ui.csline->set_description(
				sv->view_description()
			);
		}
	}
}

} // namespace UI
} // namespace Onsang
