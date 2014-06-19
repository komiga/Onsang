
#include <Onsang/Error.hpp>
#include <Onsang/serialization.hpp>
#include <Onsang/Log.hpp>

#ifdef ONSANG_CLIENT
#include <Beard/Error.hpp>
#endif

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
#ifdef ONSANG_CLIENT
	} catch (Beard::Error const& err) {
		Log::report_error(err);
#endif
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
