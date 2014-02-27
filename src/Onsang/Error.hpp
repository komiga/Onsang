/**
@file Error.hpp
@brief %Error class.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_ERROR_HPP_
#define ONSANG_ERROR_HPP_

#include <Onsang/config.hpp>
#include <Onsang/ErrorCode.hpp>
#include <Onsang/String.hpp>

#include <duct/GR/Error.hpp>

namespace Onsang {

using Error
= duct::GR::Error<
	Onsang::ErrorCode,
	Onsang::String
>;

char const*
get_error_name(
	ErrorCode const error_code
) noexcept;

} // namespace Onsang

#endif // ONSANG_ERROR_HPP_
