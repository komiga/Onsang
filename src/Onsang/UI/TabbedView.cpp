/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedView.hpp>
#include <Onsang/App.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

// UI::View implementation

unsigned
TabbedView::sub_view_index() noexcept {
	return m_position;
}

UI::View::SPtr
TabbedView::sub_view() noexcept {
	return empty()
		? UI::View::SPtr{}
		: std::dynamic_pointer_cast<UI::View>(
			m_tabs[m_position].widget
		)
	;
}

void
TabbedView::set_sub_view_impl(
	unsigned const index
) noexcept {
	set_current_tab(index);
}

unsigned
TabbedView::num_sub_views() noexcept {
	return m_tabs.size();
}

void
TabbedView::close_sub_view(
	unsigned index
) noexcept {
	if (index < m_tabs.size()) {
		remove(index);
		if (m_tabs.empty()) {
			App::instance.m_ui.csline->clear_location();
			App::instance.m_ui.csline->set_description(view_description());
		}
	}
}

void
TabbedView::sub_view_title_changed(
	unsigned index
) noexcept {
	if (index < m_tabs.size()) {
		auto const& tab = m_tabs[index];
		auto const view = std::dynamic_pointer_cast<UI::View>(tab.widget);
		if (view) {
			set_title(index, view->view_title());
		}
	}
}

void
TabbedView::notify_command(
	UI::View* const /*parent_view*/,
	Hord::Cmd::UnitBase const& command,
	Hord::Cmd::TypeInfo const& type_info
) noexcept {
	for (unsigned index = 0; index < m_tabs.size(); ++index) {
		auto& tab = m_tabs[index];
		auto const view = std::dynamic_pointer_cast<UI::View>(tab.widget);
		if (!view) {
			continue;
		}
		view->notify_command(this, command, type_info);
	}
}

} // namespace UI
} // namespace Onsang
