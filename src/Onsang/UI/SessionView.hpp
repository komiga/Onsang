/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Session view.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/System/Defs.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedView.hpp>

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
	: public UI::TabbedView
{
public:
	using SPtr = aux::shared_ptr<UI::SessionView>;

private:
	using base = UI::TabbedView;
	enum class ctor_priv {};

public:
	System::Session& m_session;

private:
	SessionView() noexcept = delete;
	SessionView(SessionView const&) = delete;
	SessionView& operator=(SessionView const&) = delete;

public:
// UI::View implementation
	String
	view_title() noexcept override;

	String
	view_description() noexcept override;

public:
// special member functions
	~SessionView() noexcept override = default;

	SessionView(
		ctor_priv const,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent,
		System::Session& session
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
			std::move(parent),
			session
		);
		widget->init();
		return widget;
	}

// operations
	void
	add_object_view(
		Hord::Object::ID const object_id,
		unsigned index = static_cast<unsigned>(-1)
	);

	void
	update_view_title(
		Hord::Object::ID const object_id
	);
};

} // namespace UI
} // namespace Onsang
