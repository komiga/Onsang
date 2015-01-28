/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Object view widget.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/System/Session.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/TabbedView.hpp>
#include <Onsang/UI/PropView.hpp>

#include <Beard/ui/Root.hpp>

#include <Hord/Object/Defs.hpp>

#include <duct/debug.hpp>

namespace Onsang {
namespace UI {

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
	view_title() noexcept override;

	String
	view_description() noexcept override;

	void
	notify_command(
		UI::View* parent_view,
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::TypeInfo const& type_info
	) noexcept override;

public:
// special member functions
	~ObjectView() noexcept override = default;

	ObjectView(
		ctor_priv const,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent,
		System::Session& session,
		Hord::Object::Unit& object
	) noexcept
		: base(
			static_cast<UI::Widget::Type>(
				UI::OnsangWidgetType::ObjectView
			),
			(
				UI::Widget::Flags::trait_container |
				UI::Widget::Flags::visible
			),
			UI::group_null,
			{{0, 0}, true, UI::Axis::both, UI::Axis::both},
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
			std::move(parent),
			session,
			object
		);
		return widget;
	}

// operations
	void
	add_prop_view(
		UI::PropView::SPtr prop_view,
		unsigned index = static_cast<unsigned>(-1)
	);
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
