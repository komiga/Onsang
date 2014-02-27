
#include <Onsang/config.hpp>
#include <Onsang/Log.hpp>
#include <Onsang/ConfigNode.hpp>
#include <Onsang/Client/Unit.hpp>

using namespace Onsang;

signed
main(
	signed argc,
	char* argv[]
) {
	Log::Controller& lc = Log::get_controller();
	lc.file(false);
	lc.stdout(true);
	/*lc.set_file_path("~/.local/share/Onsang/log");
	lc.file(true);*/

	// Initialize
	bool initialized = false;
	Client::Unit unit;
	try {
		Log::acquire()
			<< "Client: initializing\n"
		;
		initialized = unit.init(argc, argv);
	} catch (Error& err) {
		report_error(err);
	}
	if (!initialized) {
		Log::acquire()
			<< "Client: failed to initialize\n"
		;
		return -1;
	}

	// Start
	try {
		Log::acquire()
			<< "Client: starting\n"
		;
		// The terminal will get all screwy if we don't disable stdout
		lc.stdout(false);
		unit.start();
	} catch (Error& err) {
		lc.stdout(true);
		Log::acquire(Log::error)
			<< "Client: caught exception from start():\n"
		;
		report_error(err);
		return -2;
	}

	lc.stdout(true);
	Log::acquire()
		<< "Client: stopping\n"
	;
	return 0;
}
