/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief View base class.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/String.hpp>
#include <Onsang/utility.hpp>

#include <Hord/Cmd/Defs.hpp>
#include <Hord/Cmd/Unit.hpp>

namespace Onsang {
namespace UI {

class View {
public:
	using SPtr = aux::shared_ptr<UI::View>;

private:
	View(View const&) = delete;
	View& operator=(View const&) = delete;

public:
// special member functions
	virtual
	~View() noexcept = 0;

	View() noexcept = default;
	View(View&&) = default;
	View& operator=(View&&) = default;

// properties
	virtual String
	view_title() noexcept = 0;

	virtual String
	view_description() noexcept = 0;

// operations
	virtual unsigned
	sub_view_index() noexcept = 0;

	virtual UI::View::SPtr
	sub_view() noexcept = 0;

protected:
	virtual void
	set_sub_view_impl(
		unsigned index
	) noexcept = 0;

public:
	void
	set_sub_view(
		unsigned index
	) noexcept;

	virtual unsigned
	num_sub_views() noexcept = 0;

	unsigned
	sub_view_index_last() noexcept {
		return max_ce(0, signed_cast(num_sub_views()) - 1);
	}

	void
	prev_sub_view() noexcept {
		set_sub_view(max_ce(0, signed_cast(sub_view_index()) - 1));
	}

	void
	next_sub_view() noexcept {
		set_sub_view(sub_view_index() + 1);
	}

	virtual void
	close_sub_view(
		unsigned index
	) noexcept = 0;

	void
	close_sub_view() noexcept {
		close_sub_view(sub_view_index());
	}

	virtual void
	sub_view_title_changed(
		unsigned index
	) noexcept = 0;

// notifications
	virtual void
	notify_command(
		UI::View* parent_view,
		Hord::Cmd::UnitBase const& command,
		Hord::Cmd::TypeInfo const& type_info
	) noexcept = 0;
};
inline View::~View() noexcept = default;

} // namespace UI
} // namespace Onsang
