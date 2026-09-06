#pragma once
#include "klib/string/c_string.hpp"
#include <chrono>
#include <optional>
#include <string>

namespace btrsnap {
using Clock = std::chrono::system_clock;
using Seconds = std::chrono::seconds;
using ZonedSeconds = std::chrono::zoned_time<Seconds>;

[[nodiscard]] auto to_timestamp(klib::CString timestamp) -> std::optional<Clock::time_point>;
[[nodiscard]] auto to_zoned_seconds(Clock::time_point const& timestamp) -> ZonedSeconds;
[[nodiscard]] auto to_pathname(ZonedSeconds const& zoned_seconds) -> std::string;
[[nodiscard]] auto format_delta_time(ZonedSeconds const& now, Clock::time_point const& timestamp, int num_largest = 2) -> std::string;
} // namespace btrsnap
