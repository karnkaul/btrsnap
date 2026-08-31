#pragma once
#include "btrsnap/error.hpp"
#include <expected>

namespace btrsnap {
template <typename Type>
using Result = std::expected<Type, Error>;
} // namespace btrsnap
