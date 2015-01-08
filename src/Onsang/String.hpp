/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief String type.
*/

#pragma once

#include <Onsang/config.hpp>

#include <Hord/String.hpp>

namespace Onsang {

#define ONSANG_STR_LIT(x_) HORD_STR_LIT(x_)
using String = Hord::String;

} // namespace Onsang
