#pragma once
#include "detail/clock.hpp"
#include <filesystem>
#include <optional>

namespace btrsnap::detail {
namespace fs = std::filesystem;

struct SnapshotInfo {
	[[nodiscard]] static auto create(fs::path path) -> SnapshotInfo;

	fs::path path{};
	std::optional<Clock::time_point> timestamp{};
};
} // namespace btrsnap::detail
