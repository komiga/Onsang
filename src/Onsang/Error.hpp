/**
@file Error.hpp
@brief %Error class.

@author Tim Howard
@copyright 2013 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_ERROR_HPP_
#define ONSANG_ERROR_HPP_

#include "./config.hpp"
#include "./ErrorCode.hpp"

#include <duct/GR/Error.hpp>

namespace Onsang {

using Error
= duct::GR::Error<
	Onsang::ErrorCode,
	Onsang::String
>;

} // namespace Onsang

#endif // ONSANG_ERROR_HPP_
