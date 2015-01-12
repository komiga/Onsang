/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Command line and status line widget.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/UI/Defs.hpp>

#include <Beard/txt/Tree.hpp>
#include <Beard/txt/Cursor.hpp>
#include <Beard/ui/Widget/Base.hpp>
#include <Beard/ui/Root.hpp>

#include <utility>

namespace Onsang {
namespace UI {

// Forward declarations
class CommandStatusLine;

/**
	Object view widget.
*/
class CommandStatusLine
	: public UI::Widget::Base
{
public:
	using SPtr = aux::shared_ptr<UI::CommandStatusLine>;

	enum class MessageType : unsigned {
		description,
		control,
		error,
	};

private:
	using base = UI::Widget::Base;
	enum class ctor_priv {};

	UI::CommandStatusLine::MessageType
	m_message_type{
		UI::CommandStatusLine::MessageType::description
	};
	String m_message{};
	String m_location{};

	UI::Widget::WPtr m_prev_focus;
	txt::Tree m_field_text_tree;
	txt::Cursor m_field_cursor;
	txt::Cursor m_field_view;

	CommandStatusLine() noexcept = delete;
	CommandStatusLine(CommandStatusLine const&) = delete;
	CommandStatusLine& operator=(CommandStatusLine const&) = delete;

protected:
// Beard::ui::Widget::Base implementation
	void
	set_input_control_impl(
		bool const enabled
	) noexcept override;

	void
	reflow_impl(
		Rect const& area,
		bool const cache
	) noexcept override;

	bool
	handle_event_impl(
		UI::Event const& event
	) noexcept override;

	void
	render_impl(
		UI::Widget::RenderData& rd
	) noexcept override;

// -
	void
	update_field_view() noexcept;

	void
	set_message(
		UI::CommandStatusLine::MessageType type,
		String&& text
	);

public:
// special member functions
	~CommandStatusLine() noexcept override = default;

	CommandStatusLine(
		ctor_priv const,
		UI::RootWPtr&& root,
		UI::Widget::WPtr&& parent
	) noexcept
		: base(
			static_cast<UI::Widget::Type>(
				UI::OnsangWidgetType::CommandStatusLine
			),
			enum_combine(
				UI::Widget::Flags::visible
			),
			UI::group_null,
			{{0, 1}, false, UI::Axis::horizontal, UI::Axis::horizontal},
			std::move(root),
			std::move(parent)
		)
		, m_field_text_tree()
		, m_field_cursor(m_field_text_tree)
		, m_field_view(m_field_text_tree)
	{}

	CommandStatusLine(CommandStatusLine&&) = default;
	CommandStatusLine& operator=(CommandStatusLine&&) = default;

	static UI::CommandStatusLine::SPtr
	make(
		UI::RootWPtr root,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		auto widget = aux::make_shared<UI::CommandStatusLine>(
			ctor_priv{},
			std::move(root),
			std::move(parent)
		);
		return widget;
	}

// properties
	void
	set_description(
		String text
	) {
		set_message(MessageType::description, std::move(text));
	}

	void
	set_control(
		String text
	) {
		set_message(MessageType::control, std::move(text));
	}

	void
	set_error(
		String text
	) {
		set_message(MessageType::error, std::move(text));
	}

	void
	clear_message() {
		set_message(MessageType::description, {});
	}

	void
	set_location(
		String text
	);

	void
	clear_location() {
		set_location({});
	}

// operations
	void
	prompt_command();
};

} // namespace UI
} // namespace Onsang
