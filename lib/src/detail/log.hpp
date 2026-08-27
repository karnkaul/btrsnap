#include "klib/log/tagged.hpp"

namespace btrsnap::detail {
inline auto const log = klib::log::Tagged{"btrsnap"};
} // namespace btrsnap::detail
