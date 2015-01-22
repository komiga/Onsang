/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Bare field widget.
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

class BareField {
public:
	UI::Geom m_geom{{2, 1}, true, Axis::horizontal, Axis::horizontal};
	txt::Tree m_text_tree{};
	txt::Cursor m_cursor{m_text_tree};
	txt::Cursor m_view{m_text_tree};

private:
	BareField(BareField const&) = delete;
	BareField& operator=(BareField const&) = delete;

public:
// special member functions
	~BareField() noexcept = default;

	BareField() = default;
	BareField(
		UI::Geom&& geom
	)
		: m_geom(std::move(geom))
	{}
	BareField(BareField&&) = default;
	BareField& operator=(BareField&&) = default;

// operations
	void
	update_view() noexcept;

	void
	input_control_changed(
		UI::Widget::Base& widget
	) noexcept;

	void
	reflow() noexcept;

	void
	reflow_into(
		Rect const& area
	) noexcept;

	void
	render(
		UI::Widget::RenderData& rd,
		bool active
	) noexcept;

	bool
	input(
		UI::KeyInputData const& key_input
	);
};

} // namespace UI
} // namespace Onsang
