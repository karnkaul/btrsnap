#pragma once
#include "klib/string/c_string.hpp"
#include <chrono>
#include <optional>
#include <string>

namespace btrsnap {
using Clock = std::chrono::system_clock;
using Seconds = std::chrono::seconds;
using Timestamp = std::chrono::seconds;

[[nodiscard]] auto to_timestamp(klib::CString serialized) -> std::optional<Timestamp>;
[[nodiscard]] auto current_timestamp() -> Timestamp;
[[nodiscard]] auto to_pathname(Timestamp timestamp) -> std::string;
[[nodiscard]] auto format_delta_time(Seconds delta_time, int num_largest = 2) -> std::string;
} // namespace btrsnap
