/**
@file String.hpp
@brief String type.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_STRING_HPP_
#define ONSANG_STRING_HPP_

#include <Onsang/config.hpp>

#include <Hord/String.hpp>

namespace Onsang {

#define ONSANG_STR_LIT(x__) HORD_STR_LIT(x__)
using String = Hord::String;

} // namespace Onsang

#endif // ONSANG_STRING_HPP_
