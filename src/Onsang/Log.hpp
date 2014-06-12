/**
@file Log.hpp
@brief Logging.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_LOG_HPP_
#define ONSANG_LOG_HPP_

#include <Onsang/config.hpp>
#include <Onsang/Error.hpp>
#include <Onsang/serialization.hpp>

#ifdef ONSANG_CLIENT
#include <Beard/Error.hpp>
#endif

#include <Hord/Log.hpp>
#include <Hord/Error.hpp>

#include <exception>

namespace Onsang {

namespace Log = Hord::Log;

inline void
report_error(
	std::exception const& err
) {
	Log::acquire(Log::error)
		<< "[std] "
		<< err.what()
		<< '\n'
	;
}

inline void
report_error(
	SerializerError const& err
) {
	Log::acquire(Log::error)
		<< "[Serializer:" << get_ser_error_name(err.get_code()) << "] "
		<< err.get_message()
		<< '\n'
	;
}

#ifdef ONSANG_CLIENT
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
#endif

inline void
report_error(
	Hord::Error const& err
) {
	Log::acquire(Log::error)
		<< "[Hord:" << Hord::get_error_name(err.get_code()) << "] "
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
report_error(
	std::exception_ptr
);

} // namespace Onsang

#endif // ONSANG_LOG_HPP_
