/**
@file config.hpp
@brief Configuration.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_CONFIG_HPP_
#define ONSANG_CONFIG_HPP_

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

#endif // ONSANG_CONFIG_HPP_
