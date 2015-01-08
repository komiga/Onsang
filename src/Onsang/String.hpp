/**
@file
@brief String type.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#pragma once

#include <Onsang/config.hpp>

#include <Hord/String.hpp>

namespace Onsang {

#define ONSANG_STR_LIT(x_) HORD_STR_LIT(x_)
using String = Hord::String;

} // namespace Onsang
