#pragma once
#include "btrsnap/error.hpp"
#include <expected>

namespace btrsnap::detail {
[[nodiscard]] auto to_error(Error::Type type, std::string_view message) -> std::unexpected<Error>;
} // namespace btrsnap::detail
