/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Session view.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/System/Defs.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedContainer.hpp>

#include <Beard/ui/Widget/Base.hpp>
#include <Beard/ui/Root.hpp>

#include <Hord/Object/Defs.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

// Forward declarations
class SessionView;

/**
	Object view widget.
*/
class SessionView
	: public UI::Widget::Base
{
public:
	using SPtr = aux::shared_ptr<UI::SessionView>;

private:
	using base = UI::Widget::Base;
	enum class ctor_priv {};

public:
	System::Session& m_session;
	UI::TabbedContainer::SPtr m_container;

private:
	SessionView() noexcept = delete;
	SessionView(SessionView const&) = delete;
	SessionView& operator=(SessionView const&) = delete;

protected:
// Beard::ui::Widget::Base implementation
	void
	cache_geometry_impl() noexcept override {
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
	reflow_impl(
		Rect const& area,
		bool const cache
	) noexcept override {
		base::reflow_impl(area, cache);
		m_container->reflow(area, false);
	}

	/*bool
	handle_event_impl(
		UI::Event const& event
	) noexcept override {}*/

	void
	render_impl(
		UI::Widget::RenderData& rd
	) noexcept override {
		m_container->render(rd);
	}

	signed
	num_children_impl() const noexcept override {
		return 1;
	}

	UI::Widget::SPtr
	get_child_impl(
		signed const index
	) override {
		DUCT_ASSERTE(0 == index);
		return m_container;
	}

public:
// special member functions
	~SessionView() noexcept override = default;

	SessionView(
		ctor_priv const,
		UI::RootWPtr&& root,
		System::Session& session,
		UI::Widget::WPtr&& parent
	) noexcept
		: base(
			static_cast<UI::Widget::Type>(
				UI::OnsangWidgetType::SessionView
			),
			enum_combine(
				UI::Widget::Flags::trait_container,
				UI::Widget::Flags::visible
			),
			UI::group_null,
			{{0, 0}, false, UI::Axis::both, UI::Axis::both},
			std::move(root),
			std::move(parent)
		)
		, m_session(session)
		, m_container(TabbedContainer::make(get_root_weak()))
	{}

	SessionView(SessionView&&) = default;
	SessionView& operator=(SessionView&&) = default;

	static UI::SessionView::SPtr
	make(
		UI::RootWPtr root,
		System::Session& session,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		auto widget = aux::make_shared<UI::SessionView>(
			ctor_priv{},
			std::move(root),
			session,
			std::move(parent)
		);
		widget->m_container->set_parent(UI::Widget::WPtr{widget}, 0);
		return widget;
	}

// operations
	void
	add_object_view(
		Hord::Object::ID const object_id,
		unsigned const index = static_cast<unsigned>(-1)
	);

	void
	update_view_title(
		Hord::Object::ID const object_id
	);
};

} // namespace UI
} // namespace Onsang
