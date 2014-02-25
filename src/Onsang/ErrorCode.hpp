/**
@file ErrorCode.hpp
@brief %ErrorCode enum.

@author Tim Howard
@copyright 2013 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_ERRORCODE_HPP_
#define ONSANG_ERRORCODE_HPP_

#include <Onsang/config.hpp>

namespace Onsang {

enum class ErrorCode : unsigned {
// config
	/**
		An unknown variable was encountered.
	*/
	config_var_unknown = 0u,
	/**
		Variable validation failed.
	*/
	config_var_invalid,
	/**
		Node already exists.
	*/
	config_node_already_exists,

	LAST
};

} // namespace Onsang

#endif // ONSANG_ERRORCODE_HPP_
