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

enum struct SnapshotNumber : int {};

struct SnapshotList {
	using Number = SnapshotNumber;

	struct Entry {
		Number number{};
		Snapshot snapshot{};
		bool is_archived{};
	};

	std::vector<Entry> entries{};
};
} // namespace btrsnap
