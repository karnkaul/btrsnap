#pragma once
#include "btrsnap/error.hpp"
#include <expected>

namespace btrsnap::detail {
[[nodiscard]] auto to_error(Error::Type type, std::string_view message) -> std::unexpected<Error>;
[[nodiscard]] inline auto filesystem_error(std::string_view message) -> std::unexpected<Error> { return to_error(Error::Type::Filesystem, message); }
} // namespace btrsnap::detail
