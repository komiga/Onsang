/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Logging.
*/

#pragma once

#include <Onsang/config.hpp>
#include <Onsang/Error.hpp>
#include <Onsang/serialization.hpp>

#include <Beard/Error.hpp>

#include <Hord/Log.hpp>
#include <Hord/Error.hpp>

#include <exception>

namespace Onsang {
namespace Log {

using Hord::Log::report_error;
using namespace Hord::Log;

inline void
report_error(
	Beard::Error const& err
) {
	Log::acquire(Log::error)
		<< "[Beard:" << Beard::get_error_name(err.get_code()) << "] "
		<< err.get_message()
		<< '\n'
	;
}

inline void
report_error(
	Onsang::Error const& err
) {
	Log::acquire(Log::error)
		<< "[Onsang:" << get_error_name(err.get_code()) << "] "
		<< err.get_message()
		<< '\n'
	;
}

void
report_error_ptr(
	std::exception_ptr
);

} // namespace Log
} // namespace Onsang
