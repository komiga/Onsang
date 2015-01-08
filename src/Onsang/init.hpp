/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Initialization functions.
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
