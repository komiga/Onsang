/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief %ErrorCode enum.
*/

#pragma once

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

// session / session manager
	/**
		Session type was not recognized.
	*/
	session_type_unrecognized,

// command
	/**
		Command failed.
	*/
	command_failed,

// -
	LAST
};

} // namespace Onsang
