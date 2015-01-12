/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Object view.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedContainer.hpp>

#include <Beard/ui/Widget/Base.hpp>
#include <Beard/ui/Root.hpp>

#include <Hord/Object/Defs.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

// Forward declarations
class ObjectView;

/**
	Object view widget.
*/
class ObjectView
	: public UI::Widget::Base
{
public:
	using SPtr = aux::shared_ptr<UI::ObjectView>;

private:
	using base = UI::Widget::Base;
	enum class ctor_priv {};

	System::Session& m_session;
	Hord::Object::Unit& m_object;
	UI::TabbedContainer::SPtr m_container;

	ObjectView() noexcept = delete;
	ObjectView(ObjectView const&) = delete;
	ObjectView& operator=(ObjectView const&) = delete;

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
	~ObjectView() noexcept override = default;

	ObjectView(
		ctor_priv const,
		UI::RootWPtr&& root,
		System::Session& session,
		Hord::Object::Unit& object,
		UI::Widget::WPtr&& parent
	) noexcept
		: base(
			static_cast<UI::Widget::Type>(
				UI::OnsangWidgetType::ObjectView
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
		, m_object(object)
		, m_container(TabbedContainer::make(get_root_weak()))
	{}

	ObjectView(ObjectView&&) = default;
	ObjectView& operator=(ObjectView&&) = default;

	static UI::ObjectView::SPtr
	make(
		UI::RootWPtr root,
		System::Session& session,
		Hord::Object::Unit& object,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		auto widget = aux::make_shared<UI::ObjectView>(
			ctor_priv{},
			std::move(root),
			session,
			object,
			std::move(parent)
		);
		widget->m_container->set_parent(UI::Widget::WPtr{widget}, 0);
		return widget;
	}

// properties
	System::Session&
	get_session() noexcept{
		return m_session;
	}

	Hord::Object::Unit&
	get_object() noexcept {
		return m_object;
	}

	Hord::Object::Unit const&
	get_object() const noexcept {
		return m_object;
	}

	UI::TabbedContainer::SPtr
	get_container() noexcept {
		return m_container;
	}

// operations
	void
	add_prop_view(
		String&& name,
		UI::Widget::SPtr&& widget,
		unsigned const index = static_cast<unsigned>(-1)
	) {
		m_container->insert(std::move(name), std::move(widget), index);
	}
};

/**
	Make an object view with props by object type.
*/
UI::ObjectView::SPtr
make_object_view(
	UI::RootWPtr&& root,
	System::Session& session,
	Hord::Object::Unit& object
);

} // namespace UI
} // namespace Onsang
