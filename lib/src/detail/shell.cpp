#include "detail/shell.hpp"
#include "detail/log.hpp"
#include "klib/cli/shell.hpp"

namespace btrsnap::detail {
auto shell::execute(klib::CString const expression) -> bool {
	log.debug("-- {}", expression);
	return klib::shell::execute(expression) == klib::shell::success_v;
}
} // namespace btrsnap::detail
