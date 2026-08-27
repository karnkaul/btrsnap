#include "detail/shell.hpp"
#include "detail/log.hpp"
#include "klib/cli/shell.hpp"

namespace btrsnap::detail {
namespace {
constexpr auto elevate_v = std::string_view{"sudo"};
} // namespace

auto shell::execute(klib::CString const expression) -> bool {
	log.debug("-- {}", expression);
	return klib::shell::execute(expression) == klib::shell::success_v;
}

auto shell::execute_privileged(std::string_view const expression) -> bool { return execute(std::format("{} {}", elevate_v, expression)); }
} // namespace btrsnap::detail
