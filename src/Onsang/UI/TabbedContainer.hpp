/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Tabbed container widget.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/UI/Defs.hpp>

#include <Beard/ui/Widget/Base.hpp>
#include <Beard/ui/Root.hpp>

namespace Onsang {
namespace UI {

class TabbedContainer
	: public UI::Widget::Base
{
public:
	using SPtr = aux::shared_ptr<UI::TabbedContainer>;

	struct Tab {
		String title;
		UI::Widget::SPtr widget;
	};

private:
	using base = UI::Widget::Base;
	enum class ctor_priv {};

public:
	unsigned m_position;
	aux::vector<Tab> m_tabs;

	TabbedContainer() noexcept = delete;
	TabbedContainer(TabbedContainer const&) = delete;
	TabbedContainer& operator=(TabbedContainer const&) = delete;

private:
	void push_back(UI::Widget::SPtr /*widget*/);

protected:
// Beard::ui::Widget::Base implementation
	virtual void
	push_action_graph_impl(
		ui::Widget::set_type& set
	) noexcept override;

	virtual void
	cache_geometry_impl() noexcept override;

	virtual void
	reflow_impl() noexcept override;

	virtual void
	render_impl(
		UI::Widget::RenderData& rd
	) noexcept override;

	signed
	num_children_impl() const noexcept override;

	UI::Widget::SPtr
	child_at_impl(
		signed const index
	) override;

public:
// special member functions
	~TabbedContainer() noexcept override = default;

protected:
	TabbedContainer(
		UI::Widget::Type const type,
		UI::Widget::Flags const flags,
		UI::group_hash_type const group,
		UI::Geom&& geometry,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent
	) noexcept
		: base(
			type,
			(
				flags |
				UI::Widget::Flags::trait_container
			),
			group,
			std::move(geometry),
			std::move(root),
			std::move(parent)
		)
		, m_position(0)
		, m_tabs()
	{}

public:
	TabbedContainer(
		ctor_priv const,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent
	) noexcept
		: TabbedContainer(
			static_cast<UI::Widget::Type>(UI::OnsangWidgetType::TabbedContainer),
			(
				UI::Widget::Flags::trait_container |
				UI::Widget::Flags::visible
			),
			UI::group_null,
			{{0, 0}, true, UI::Axis::both, UI::Axis::both},
			std::move(root),
			std::move(parent)
		)
	{}

	TabbedContainer(TabbedContainer&&) = default;
	TabbedContainer& operator=(TabbedContainer&&) = default;

	static UI::TabbedContainer::SPtr
	make(
		UI::RootWPtr root,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		return aux::make_shared<UI::TabbedContainer>(
			ctor_priv{},
			std::move(root),
			std::move(parent)
		);
	}

// properties
	bool
	empty() const noexcept {
		return m_tabs.empty();
	}

	unsigned
	last_index() const noexcept {
		return m_tabs.empty()
			? 0
			: static_cast<unsigned>(m_tabs.size() - 1)
		;
	}

// operations
	void
	push_back(
		String title,
		UI::Widget::SPtr&& widget
	);

	unsigned
	insert(
		String title,
		UI::Widget::SPtr widget,
		unsigned index
	);

	void
	remove(
		unsigned index
	);

	void
	remove_current();

	void
	clear();

	void
	set_title(
		unsigned const index,
		String title
	);

	void
	set_tab(
		std::size_t const index,
		UI::Widget::SPtr widget
	);

	void
	set_current_tab(
		unsigned index
	);

	void
	prev_tab();

	void
	next_tab();

	void
	move_tab(
		unsigned from,
		unsigned to,
		bool const insert = false
	);

	void
	move_left(
		unsigned const index,
		bool const insert = false
	);

	void
	move_right(
		unsigned const index,
		bool const insert = false
	);
};

} // namespace UI
} // namespace Onsang
