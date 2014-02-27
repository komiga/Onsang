/**
@file Log.hpp
@brief Logging.

@author Tim Howard
@copyright 2013 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_LOG_HPP_
#define ONSANG_LOG_HPP_

#include <Onsang/config.hpp>
#include <Onsang/Error.hpp>

#ifdef ONSANG_CLIENT
#include <Beard/Error.hpp>
#endif

#include <Hord/Log.hpp>
#include <Hord/Error.hpp>

namespace Onsang {

namespace Log = Hord::Log;

#ifdef ONSANG_CLIENT
inline void
report_error(
	Beard::Error const& err
) {
	Log::acquire(Log::error)
		<< '[' << Beard::get_error_name(err.get_code()) << "]\n"
		<< err.get_message()
		<< '\n'
	;
}
#endif

inline void
report_error(
	Hord::Error const& err
) {
	Log::acquire(Log::error)
		<< '[' << Hord::get_error_name(err.get_code()) << "]\n"
		<< err.get_message()
		<< '\n'
	;
}

inline void
report_error(
	Onsang::Error const& err
) {
	Log::acquire(Log::error)
		<< '[' << get_error_name(err.get_code()) << "]\n"
		<< err.get_message()
		<< '\n'
	;
}

} // namespace Onsang

#endif // ONSANG_LOG_HPP_
