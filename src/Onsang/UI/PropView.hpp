/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Prop view widget.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/UI/Defs.hpp>
#include <Onsang/UI/View.hpp>

#include <Beard/ui/Signal.hpp>
#include <Beard/ui/Root.hpp>
#include <Beard/ui/ProtoSlotContainer.hpp>

namespace Onsang {
namespace UI {

// Forward declarations
class PropView;

/**
	Prop view widget.
*/
class PropView
	: public UI::ProtoSlotContainer
	, public UI::View
{
public:
	using SPtr = aux::shared_ptr<UI::PropView>;

private:
	using base = UI::ProtoSlotContainer;
	enum class ctor_priv {};

public:
	String m_name;

	UI::Signal<void(
		UI::View* parent_view,
		UI::PropView& prop_view,
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::type_info const& type_info
	)> signal_notify_command;

private:
	PropView() noexcept = delete;
	PropView(PropView const&) = delete;
	PropView& operator=(PropView const&) = delete;

public:
// UI::View implementation
	String
	view_title() noexcept override {
		return m_name;
	}

	String
	view_description() noexcept override {
		return "prop view: " + m_name;
	}

	unsigned
	sub_view_index() noexcept override {
		return 0;
	}

	UI::View::SPtr
	sub_view() noexcept override {
		return UI::View::SPtr{};
	}

	void
	set_sub_view_impl(
		unsigned const /*index*/
	) noexcept override {}

	unsigned
	num_sub_views() noexcept override {
		return 0;
	}

	using UI::View::close_sub_view;
	void
	close_sub_view(
		unsigned /*index*/
	) noexcept override {}

	void
	sub_view_title_changed(
		unsigned /*index*/
	) noexcept override {}

	void
	notify_command(
		UI::View* parent_view,
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::type_info const& type_info
	) noexcept override {
		if (signal_notify_command.is_bound()) {
			signal_notify_command(parent_view, *this, command, type_info);
		}
	}

public:
// special member functions
	~PropView() noexcept override = default;

	PropView(
		ctor_priv const,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent,
		Axis const orientation,
		String&& name
	) noexcept
		: base(
			static_cast<UI::Widget::Type>(
				UI::OnsangWidgetType::PropView
			),
			(
				UI::Widget::Flags::trait_container |
				UI::Widget::Flags::visible
			),
			UI::group_null,
			{{0, 0}, true, UI::Axis::both, UI::Axis::both},
			std::move(root),
			std::move(parent),
			orientation
		)
		, UI::View()
		, m_name(std::move(name))
		, signal_notify_command()
	{}

	PropView(PropView&&) = default;
	PropView& operator=(PropView&&) = default;

	static UI::PropView::SPtr
	make(
		UI::RootWPtr root,
		String name,
		Axis orientation = Axis::vertical,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		auto widget = aux::make_shared<UI::PropView>(
			ctor_priv{},
			std::move(root),
			std::move(parent),
			orientation,
			std::move(name)
		);
		return widget;
	}
};

} // namespace UI
} // namespace Onsang
