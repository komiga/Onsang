/**
@file utility.hpp
@brief Utilities.

@author Tim Howard
@copyright 2013-2014 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_UTILITY_HPP_
#define ONSANG_UTILITY_HPP_

#include <Onsang/config.hpp>

#include <duct/utility.hpp>

namespace Onsang {

using duct::make_const;
using duct::signed_cast;
using duct::unsigned_cast;
using duct::enum_cast;
using duct::enum_bitor;
using duct::enum_bitand;
using duct::enum_combine;
using duct::min_ce;
using duct::max_ce;

} // namespace Onsang

#endif // ONSANG_UTILITY_HPP_
