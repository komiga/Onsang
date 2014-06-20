
#include <Onsang/System/Session.hpp>

#include <Onsang/detail/gr_core.hpp>

namespace Onsang {
namespace System {

#define ONSANG_SCOPE_CLASS System::Session

#define ONSANG_SCOPE_FUNC open
void
Session::open() {
	get_datastore().open(m_auto_create);
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC close
void
Session::close() {
	get_datastore().close();
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC process
void
Session::process() {
	// TODO: Update views
}
#undef ONSANG_SCOPE_FUNC

#undef ONSANG_SCOPE_CLASS

} // namespace System
} // namespace Onsang

