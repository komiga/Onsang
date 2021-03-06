/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/Error.hpp>
#include <Onsang/serialization.hpp>
#include <Onsang/Log.hpp>

#include <Beard/Error.hpp>

#include <Hord/Error.hpp>

#include <exception>

namespace Onsang {
namespace Log {

void
report_error_ptr(
	std::exception_ptr err
) {
	try {
		std::rethrow_exception(err);
	} catch (std::exception& err) {
		Log::report_error(err);
	} catch (SerializerError const& err) {
		Log::report_error(err);
	} catch (Hord::Error const& err) {
		Log::report_error(err);
	} catch (Beard::Error const& err) {
		Log::report_error(err);
	} catch (Onsang::Error const& err) {
		Log::report_error(err);
	} catch (...) {
		Log::acquire(Log::error)
			<< "[unknown exception]\n"
		;
	}
}

} // namespace Log
} // namespace Onsang
