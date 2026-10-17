#include "btrsnap/list.hpp"
#include "btrsnap/util.hpp"

namespace btrsnap {
SnapshotList::SnapshotList(Subvolume const& subvolume) : m_btrfs(&subvolume.get_btrfs()) {
	auto const transfer = [this](std::span<Snapshot> snapshots, bool const is_archived) {
		for (auto& snapshot : snapshots) { m_entries.push_back(Entry{.snapshot = std::move(snapshot), .is_archived = is_archived}); }
	};

	auto const& storage = subvolume.get_storage();
	auto snapshots = util::list_snapshots(*m_btrfs, storage.get_snapshots_directory());
	transfer(snapshots, false);
	snapshots = util::list_snapshots(*m_btrfs, storage.get_archive_directory());
	transfer(snapshots, true);

	std::ranges::sort(m_entries, [](Entry const& a, Entry const& b) { return a.snapshot.timestamp < b.snapshot.timestamp; });

	auto number = 1;
	for (auto& entry : m_entries) { entry.number = SnapshotNumber{number++}; }
}

auto SnapshotList::find_entry(Number const number) const -> klib::Ptr<Entry const> {
	auto const it = std::ranges::find_if(m_entries, [number](Entry const& entry) { return entry.number == number; });
	if (it == m_entries.end()) { return nullptr; }
	return &*it;
}

auto SnapshotList::find_snapshot(Number const number) const -> klib::Ptr<Snapshot const> {
	auto const entry = find_entry(number);
	if (!entry) { return nullptr; }
	return &entry->snapshot;
}

auto SubvolumeList::take_snapshots(Timestamp const timestamp) -> std::vector<Result<Snapshot>> {
	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& subvolume : subvolumes) { ret.push_back(subvolume.take_snapshot(timestamp)); }
	return ret;
}

auto SubvolumeList::recycle_snapshots() -> RecycleReport {
	auto ret = RecycleReport{};
	for (auto& subvolume : subvolumes) { ret.append(subvolume.recycle_snapshots()); }
	return ret;
}

auto SubvolumeList::clear_all_snapshots() -> std::vector<Result<Snapshot>> {
	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& subvolume : subvolumes) { ret.append_range(subvolume.clear_all_snapshots()); }
	return ret;
}

void SubvolumeList::print_snapshots(std::ostream& out, Timestamp const now) const {
	for (auto const& subvolume : subvolumes) { subvolume.print_snapshots(out, now); }
}
} // namespace btrsnap
