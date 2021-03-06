/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Configuration.
*/

#pragma once

#include <Hord/config.hpp>

#include <cstddef>
#include <cstdint>

namespace Onsang {

/**
	Allocator for auxiliary specializations.

	@note Defaults to @c std::allocator.
*/
#define ONSANG_AUX_ALLOCATOR std::allocator

} // namespace Onsang
