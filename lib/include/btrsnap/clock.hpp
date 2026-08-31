#pragma once
#include "klib/string/c_string.hpp"
#include <chrono>
#include <optional>
#include <string>

namespace btrsnap {
using Clock = std::chrono::system_clock;

[[nodiscard]] auto to_timestamp(klib::CString timestamp) -> std::optional<Clock::time_point>;
[[nodiscard]] auto to_pathname(Clock::time_point timestamp) -> std::string;
} // namespace btrsnap
