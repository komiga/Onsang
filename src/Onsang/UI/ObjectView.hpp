/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Object view.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedView.hpp>

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
	: public UI::TabbedView
{
public:
	using SPtr = aux::shared_ptr<UI::ObjectView>;

private:
	using base = UI::TabbedView;
	enum class ctor_priv {};

public:
	System::Session& m_session;
	Hord::Object::Unit& m_object;

private:
	ObjectView() noexcept = delete;
	ObjectView(ObjectView const&) = delete;
	ObjectView& operator=(ObjectView const&) = delete;

public:
// UI::View implementation
	String
	view_description() noexcept override;

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
		widget->init();
		return widget;
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
