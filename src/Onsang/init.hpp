/**
@file init.hpp
@brief Initialization functions.

@author Tim Howard
@copyright 2013-2014 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_INIT_HPP_
#define ONSANG_INIT_HPP_

#include <Onsang/config.hpp>

#include <Hord/System/Driver.hpp>

namespace Onsang {

// Registers command type tables and rule types
void
driver_init(
	Hord::System::Driver& driver
);

} // namespace Onsang

#endif // ONSANG_INIT_HPP_
