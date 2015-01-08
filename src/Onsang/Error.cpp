/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.
*/

#include <Onsang/utility.hpp>
#include <Onsang/ErrorCode.hpp>
#include <Onsang/String.hpp>
#include <Onsang/Error.hpp>

#include <type_traits>
#include <utility>

namespace Onsang {

namespace {
static char const
s_error_invalid[]{ONSANG_STR_LIT("INVALID")},
* const s_error_names[]{
// config
	ONSANG_STR_LIT("config_var_unknown"),
	ONSANG_STR_LIT("config_var_invalid"),
	ONSANG_STR_LIT("config_node_already_exists"),

// session / session manager
	ONSANG_STR_LIT("session_type_unrecognized"),

// command
	ONSANG_STR_LIT("command_failed"),
};
} // anonymous namespace

static_assert(
	enum_cast(ErrorCode::LAST)
	== std::extent<decltype(s_error_names)>::value,
	"ErrorCode name list is incomplete"
);

char const*
get_error_name(
	ErrorCode const error_code
) noexcept {
	std::size_t const index = static_cast<std::size_t>(error_code);
	if (index < std::extent<decltype(s_error_names)>::value) {
		return s_error_names[index];
	} else {
		return s_error_invalid;
	}
}

} // namespace Onsang
