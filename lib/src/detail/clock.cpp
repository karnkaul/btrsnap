#include "detail/clock.hpp"
#include <format>
#include <sstream>

namespace btrsnap {
auto detail::to_timestamp(klib::CString const timestamp) -> std::optional<Clock::time_point> {
	auto str = std::istringstream{timestamp.c_str()};
	auto ret = Clock::time_point{};
	if (str >> std::chrono::parse("%Y-%m-%d_%H-%M-%S", ret)) { return ret; }
	return {};
}

auto detail::to_pathname(Clock::time_point const timestamp) -> std::string {
	auto const clamped = std::chrono::time_point_cast<std::chrono::seconds>(timestamp);
	auto const zoned = std::chrono::zoned_time{std::chrono::current_zone(), clamped};
	return std::format("{:%Y-%m-%d_%H-%M-%S}", zoned);
}
} // namespace btrsnap
