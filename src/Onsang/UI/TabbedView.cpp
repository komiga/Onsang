/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedView.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

// Beard::ui::Widget::Base implementation

void
TabbedView::cache_geometry_impl() noexcept {
	m_container->cache_geometry();
	if (!get_geometry().is_static()) {
		auto const& ws = m_container->get_geometry().get_request_size();
		auto rs = get_geometry().get_request_size();
		rs.width  = max_ce(rs.width , ws.width);
		rs.height = max_ce(rs.height, ws.height);
		get_geometry().set_request_size(std::move(rs));
	}
}

void
TabbedView::reflow_impl(
	Rect const& area,
	bool const cache
) noexcept {
	base::reflow_impl(area, cache);
	m_container->reflow(area, false);
}

void
TabbedView::render_impl(
	UI::Widget::RenderData& rd
) noexcept {
	m_container->render(rd);
}

signed
TabbedView::num_children_impl() const noexcept {
	return 1;
}

UI::Widget::SPtr
TabbedView::get_child_impl(
	signed const index
) {
	DUCT_ASSERTE(0 == index);
	return m_container;
}

// UI::View implementation

unsigned
TabbedView::sub_view_index() noexcept {
	return m_container->m_position;
}

UI::View::SPtr
TabbedView::sub_view() noexcept {
	return m_container->empty()
		? UI::View::SPtr{}
		: std::dynamic_pointer_cast<UI::View>(
			m_container->m_tabs[m_container->m_position].widget
		)
	;
}

void
TabbedView::set_sub_view_impl(
	unsigned const index
) noexcept {
	m_container->set_current_tab(index);
}

unsigned
TabbedView::num_sub_views() noexcept {
	return m_container->m_tabs.size();
}

void
TabbedView::close_sub_view(
	unsigned index
) noexcept {
	if (index < m_container->m_tabs.size()) {
		m_container->remove(index);
	}
}

} // namespace UI
} // namespace Onsang
