/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/config.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/ConfigNode.hpp>
#include <Onsang/Client/Unit.hpp>

#include <exception>

using namespace Onsang;

signed
main(
	signed argc,
	char* argv[]
) {
	auto& log_controller = Log::get_controller();
	log_controller.file(false);
	log_controller.stdout(true);

	// Initialize
	Client::Unit unit{};
	bool initialized = false;
	try {
		initialized = unit.init(argc, argv);
	} catch (...) {
		Log::report_error_ptr(std::current_exception());
	}
	if (!initialized) {
		Log::acquire()
			<< "Failed to initialize\n"
		;
		return -1;
	}

	// Start
	try {
		Log::acquire()
			<< "Starting\n"
		;
		unit.start();
	} catch (...) {
		Log::acquire(Log::error)
			<< "Error running unit:\n"
		;
		Log::report_error_ptr(std::current_exception());
		return -2;
	}

	Log::acquire()
		<< "Stopping\n"
	;
	return 0;
}
