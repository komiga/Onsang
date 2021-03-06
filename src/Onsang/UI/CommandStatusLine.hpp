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
#include <Onsang/UI/BareField.hpp>

#include <Beard/txt/Tree.hpp>
#include <Beard/txt/Cursor.hpp>
#include <Beard/ui/Widget/Base.hpp>
#include <Beard/ui/Root.hpp>

#include <utility>

namespace Onsang {
namespace UI {

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

public:
	UI::CommandStatusLine::MessageType m_message_type{};
	String m_message{};
	String m_location{};

	UI::Widget::WPtr m_prev_focus{};
	UI::BareField m_field{};

private:
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
	reflow_impl() noexcept override;

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
			(
				UI::Widget::Flags::visible
			),
			UI::group_csl,
			{{0, 1}, true, UI::Axis::horizontal, UI::Axis::horizontal},
			std::move(root),
			std::move(parent)
		)
	{}

	CommandStatusLine(CommandStatusLine&&) = default;
	CommandStatusLine& operator=(CommandStatusLine&&) = default;

	static UI::CommandStatusLine::SPtr
	make(
		UI::RootWPtr root,
		UI::Widget::WPtr parent = UI::Widget::WPtr()
	) {
		return aux::make_shared<UI::CommandStatusLine>(
			ctor_priv{},
			std::move(root),
			std::move(parent)
		);
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
