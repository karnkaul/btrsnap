#pragma once
#include "btrsnap/clock.hpp"
#include <filesystem>
#include <vector>

namespace btrsnap {
namespace fs = std::filesystem;

struct Snapshot {
	fs::path path{};
	Timestamp timestamp{};
};

struct Manifest {
	fs::path subvolume{};
	int snapshots_limit{};
	std::chrono::days archive_period{};
	int archive_limit{};

	std::vector<Snapshot> primary{};
	std::vector<Snapshot> archived{};
};
} // namespace btrsnap
