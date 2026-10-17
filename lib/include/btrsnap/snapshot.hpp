#pragma once
#include "btrsnap/clock.hpp"
#include <filesystem>

namespace btrsnap {
namespace fs = std::filesystem;

struct Snapshot {
	struct List;

	fs::path path{};
	Timestamp timestamp{};
};

enum struct SnapshotNumber : int {};
} // namespace btrsnap
