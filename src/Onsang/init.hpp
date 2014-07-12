/**
@file init.hpp
@brief Initialization functions.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#pragma once

#include <Onsang/config.hpp>

#include <Hord/System/Driver.hpp>

namespace Onsang {

// Registers command type tables and rule types
void
driver_init(
	Hord::System::Driver& driver
);

} // namespace Onsang
