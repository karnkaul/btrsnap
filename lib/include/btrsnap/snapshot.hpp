#pragma once
#include "btrsnap/clock.hpp"
#include <filesystem>
#include <optional>

namespace btrsnap {
namespace fs = std::filesystem;

struct Snapshot {
	fs::path path{};
	std::optional<Clock::time_point> timestamp{};
};
} // namespace btrsnap
