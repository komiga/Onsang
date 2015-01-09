/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedContainer.hpp>

#include <Beard/ui/Widget/Base.hpp>
#include <Beard/ui/Root.hpp>

namespace Onsang {
namespace UI {

void TabbedContainer::push_back(UI::Widget::SPtr /*widget*/) {}

// Beard::ui::Widget::Base implementation
void
TabbedContainer::cache_geometry_impl() noexcept {
	auto rs = get_geometry().get_request_size();
	if (!m_tabs.empty()) {
		Tab const& tab = m_tabs.at(m_position);
		if (tab.widget) {
			tab.widget->cache_geometry();
			auto const& ws = tab.widget->get_geometry().get_request_size();
			rs.width  = max_ce(rs.width , ws.width);
			rs.height = max_ce(rs.height, ws.height);
		}
	}
	if (!get_geometry().is_static()) {
		get_geometry().set_request_size(std::move(rs));
	}
}

void
TabbedContainer::reflow_impl(
	Rect const& area,
	bool const cache
) noexcept {
	base::reflow_impl(area, cache);
	if (m_tabs.empty()) {
		return;
	}
	Rect child_frame = area;
	++child_frame.pos.y;
	--child_frame.size.height;
	vec2_clamp_min(child_frame.size, Vec2{0, 0});
	unsigned index = 0;
	for (auto& tab : m_tabs) {
		if (index == m_position) {
			tab.widget->reflow(child_frame, false);
		}
		++index;
	}
}

void
TabbedContainer::render_impl(
	UI::Widget::RenderData& rd
) noexcept {
	if (m_tabs.empty()) {
		 return;
	}
	auto const& frame = get_geometry().get_frame();
	rd.terminal.clear_back(frame);
	unsigned index = 0;
	signed xpos = 0;
	for (auto& tab : m_tabs) {
		bool const active = index == m_position;
		rd.terminal.put_sequence(
			frame.pos.x + xpos,
			frame.pos.y,
			Beard::txt::Sequence{tab.title, 0u, tab.title.size()},
			unsigned_cast(max_ce(0, frame.size.width)),
			rd.get_attr(active
				? ui::property_content_fg_selected
				: ui::property_content_fg_inactive
			),
			rd.get_attr(active
				? ui::property_content_bg_selected
				: ui::property_content_bg_inactive
			)
		);
		if (active) {
			tab.widget->render(rd);
		}
		if (index + 1 != m_tabs.size()) {
			xpos += tab.title.size() + 1;
			rd.terminal.put_cell(
				frame.pos.x + xpos,
				frame.pos.y,
				Beard::tty::make_cell(
					U'│',
					Beard::tty::Attr::bold |
					Beard::tty::Color::blue,
					Beard::tty::Color::term_default
				)
			);
			xpos += 2;
		}
		++index;
	}
}

signed
TabbedContainer::num_children_impl() const noexcept {
	return static_cast<signed>(m_tabs.size());
}

UI::Widget::SPtr
TabbedContainer::get_child_impl(
	signed const index
) {
	return m_tabs.at(static_cast<unsigned>(index)).widget;
}

// operations
void
TabbedContainer::push_back(
	String title,
	UI::Widget::SPtr&& widget
) {
	insert(std::move(title), std::move(widget), m_tabs.size());
	set_current_tab(get_last_index());
}

void
TabbedContainer::insert(
	String title,
	UI::Widget::SPtr widget,
	unsigned index
) {
	index = min_ce(index, static_cast<unsigned>(m_tabs.size()));
	widget->set_parent(shared_from_this(), index);
	if (m_tabs.empty()) {
		m_position = 0;
		widget->set_visible(true);
	} else if (index <= m_position) {
		++m_position;
		widget->set_visible(false);
	}
	m_tabs.insert(
		m_tabs.cbegin() + index,
		Tab{std::move(title), std::move(widget)}
	);
	for (; m_tabs.size() > index; ++index) {
		m_tabs[index].widget->set_index(static_cast<UI::index_type>(index));
	}
	queue_actions(enum_combine(
		UI::UpdateActions::reflow,
		UI::UpdateActions::render
	));
}

void
TabbedContainer::remove(
	unsigned index
) {
	m_tabs.erase(m_tabs.cbegin() + index);
	m_position = min_ce(m_position, get_last_index());
	for (; m_tabs.size() > index; ++index) {
		m_tabs[index].widget->set_index(static_cast<UI::index_type>(index));
	}
	if (!m_tabs.empty()) {
		m_tabs[m_position].widget->set_visible(true);
	}
	queue_actions(enum_combine(
		UI::UpdateActions::reflow,
		UI::UpdateActions::render
	));
}

void
TabbedContainer::remove_current() {
	if (!m_tabs.empty()) {
		get_root()->clear_focus();
		remove(m_position);
	}
}

void
TabbedContainer::clear() {
	m_tabs.clear();
	m_position = 0;
	queue_actions(enum_combine(
		UI::UpdateActions::reflow,
		UI::UpdateActions::render
	));
}

void
TabbedContainer::set_title(
	unsigned const index,
	String title
) {
	m_tabs.at(index).title = std::move(title);
	queue_actions(enum_combine(
		UI::UpdateActions::reflow,
		UI::UpdateActions::render
	));
}

void
TabbedContainer::set_tab(
	std::size_t const index,
	UI::Widget::SPtr widget
) {
	m_tabs.at(index).widget = std::move(widget);
}

void
TabbedContainer::set_current_tab(
	unsigned index
) {
	index = min_ce(index, get_last_index());
	if (!m_tabs.empty() && index != m_position) {
		// TODO: Track focused widget per-tab
		get_root()->clear_focus();
		m_tabs[m_position].widget->set_visible(false);
		m_tabs[index].widget->set_visible(true);
		m_position = index;
		queue_actions(enum_combine(
			UI::UpdateActions::reflow,
			UI::UpdateActions::render
		));
	}
}

void
TabbedContainer::prev_tab() {
	if (0 < m_position) {
		set_current_tab(m_position - 1);
	}
}

void
TabbedContainer::next_tab() {
	set_current_tab(m_position + 1);
}

void
TabbedContainer::move_tab(
	unsigned from,
	unsigned to,
	bool const insert
) {
	from = min_ce(from, get_last_index());
	to = min_ce(to, get_last_index());
	if (from != to) {
		if (insert) {
			Tab tab = m_tabs[from];
			m_tabs.erase(m_tabs.cbegin() + from);
			m_tabs.insert(m_tabs.cbegin() + to, std::move(tab));
			for (
				unsigned index = min_ce(from, to);
				m_tabs.size() > index;
				++index
			) {
				m_tabs[index].widget->set_index(static_cast<UI::index_type>(index));
			}
		} else {
			m_tabs[from].widget->set_index(to);
			m_tabs[to].widget->set_index(from);
			std::swap(m_tabs[from], m_tabs[to]);
		}
		if (from == m_position) {
			m_position = to;
		}
		queue_actions(enum_combine(
			UI::UpdateActions::flag_noclear,
			UI::UpdateActions::render
		));
	}
}

void
TabbedContainer::move_left(
	unsigned const index,
	bool const insert
) {
	if (index > 0) {
		move_tab(index, index - 1, insert);
	}
}

void
TabbedContainer::move_right(
	unsigned const index,
	bool const insert
) {
	move_tab(index, index + 1, insert);
}

} // namespace UI
} // namespace Onsang
