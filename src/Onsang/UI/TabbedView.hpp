/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Tabbed view base widget.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedContainer.hpp>
#include <Onsang/UI/View.hpp>

#include <Beard/ui/Widget/Base.hpp>
#include <Beard/ui/Root.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

class TabbedView
	: public UI::Widget::Base
	, public UI::View
{
public:
	using SPtr = aux::shared_ptr<UI::TabbedView>;

private:
	using base = UI::Widget::Base;

public:
	UI::TabbedContainer::SPtr m_container;

private:
	TabbedView() noexcept = delete;
	TabbedView(TabbedView const&) = delete;
	TabbedView& operator=(TabbedView const&) = delete;

protected:
	void
	init() {
		m_container->set_parent(UI::Widget::WPtr{shared_from_this()}, 0);
	}

// Beard::ui::Widget::Base implementation
	void
	cache_geometry_impl() noexcept override;

	void
	reflow_impl(
		Rect const& area,
		bool const cache
	) noexcept override;

	void
	render_impl(
		UI::Widget::RenderData& rd
	) noexcept override;

	signed
	num_children_impl() const noexcept override;

	UI::Widget::SPtr
	get_child_impl(
		signed const index
	) override;

public:
// UI::View implementation
	unsigned
	sub_view_index() noexcept override;

	UI::View::SPtr
	sub_view() noexcept override;

	void
	set_sub_view_impl(
		unsigned const index
	) noexcept override;

	unsigned
	num_sub_views() noexcept override;

	using UI::View::close_sub_view;
	void
	close_sub_view(
		unsigned index
	) noexcept override;

	void
	sub_view_title_changed(
		unsigned index
	) noexcept override;

	virtual void
	notify_command(
		UI::View* parent_view,
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::type_info const& type_info
	) noexcept override;

public:
// special member functions
	virtual
	~TabbedView() noexcept override = 0;

	TabbedView(
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
		, UI::View()
		, m_container(UI::TabbedContainer::make(get_root_weak()))
	{}

	TabbedView(TabbedView&&) = default;
	TabbedView& operator=(TabbedView&&) = default;
};
inline TabbedView::~TabbedView() noexcept = default;

} // namespace UI
} // namespace Onsang
