#pragma once
#include "btrsnap/clock.hpp"
#include "klib/ptr.hpp"
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

	[[nodiscard]] auto find_snapshot(Number number) const -> klib::Ptr<Snapshot const>;

	std::vector<Entry> entries{};
};
} // namespace btrsnap
