#pragma once
#include "btrsnap/clock.hpp"
#include <filesystem>
#include <vector>

namespace btrsnap {
namespace fs = std::filesystem;

struct Snapshot {
	struct List;

	fs::path path{};
	Timestamp timestamp{};
};

enum struct SnapshotNumber : int {};

struct Snapshot::List {
	using Number = SnapshotNumber;

	struct Entry {
		Number number{};
		Snapshot snapshot{};
		bool is_archived{};
	};

	std::vector<Entry> entries{};
};
} // namespace btrsnap
