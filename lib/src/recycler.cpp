#include "btrsnap/recycler.hpp"
#include "btrsnap/util.hpp"
#include "klib/log/typed.hpp"

namespace btrsnap {
namespace {
auto const log = klib::log::Typed<Recycler>{};
} // namespace

auto Recycler::recycle_snapshots(Storage const& storage, Info const& info) const -> Report {
	auto const get_excess_live_snapshots = [&] {
		return get_excess_snapshots(get_sorted_snapshots_in(storage.get_snapshots_directory()), info.snapshot_limit);
	};

	auto excess = get_excess_live_snapshots();
	if (excess.empty()) {
		log.info("No snapshots to recycle");
		return {};
	}

	auto ret = Report{};
	ret.archived = populate_archive(storage, excess, info.archive_period);
	if (!ret.archived.empty()) { excess = get_excess_live_snapshots(); }

	ret.deleted = delete_snapshots(std::move(excess));

	excess = get_excess_snapshots(get_sorted_snapshots_in(storage.get_archive_directory()), info.archive_limit);
	ret.deleted.append_range(delete_snapshots(std::move(excess)));

	return ret;
}

auto Recycler::get_snapshot_list(Storage const& storage) const -> SnapshotList {
	using Entry = SnapshotList::Entry;

	auto ret = SnapshotList{};
	auto const transfer = [&ret](std::span<Snapshot> snapshots, bool const is_archived) {
		for (auto& snapshot : snapshots) { ret.entries.push_back(Entry{.snapshot = std::move(snapshot), .is_archived = is_archived}); }
	};

	auto snapshots = util::list_snapshots(*m_btrfs, storage.get_snapshots_directory());
	transfer(snapshots, false);
	snapshots = util::list_snapshots(*m_btrfs, storage.get_archive_directory());
	transfer(snapshots, true);

	std::ranges::sort(ret.entries, [](Entry const& a, Entry const& b) { return a.snapshot.timestamp < b.snapshot.timestamp; });

	auto number = 1;
	for (auto& entry : ret.entries) { entry.number = SnapshotNumber{number++}; }

	return ret;
}

auto Recycler::get_sorted_snapshots_in(fs::path const& path) const -> std::vector<Snapshot> {
	auto ret = util::list_snapshots(*m_btrfs, path);
	// move youngest to back.
	std::ranges::sort(ret, [](Snapshot const& a, Snapshot const& b) { return a.timestamp < b.timestamp; });
	return ret;
}

auto Recycler::get_excess_snapshots(std::vector<Snapshot> sorted, int const keep) -> std::vector<Snapshot> {
	auto const excess_count = static_cast<int>(sorted.size()) - keep;
	if (excess_count <= 0) { return {}; }

	// pop snapshots to keep.
	sorted.resize(std::size_t(excess_count));
	return sorted;
}

auto Recycler::delete_snapshots(std::vector<Snapshot> snapshots) const -> std::vector<Result<Snapshot>> {
	if (snapshots.empty()) { return {}; }

	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& snapshot : snapshots) {
		auto const result = m_btrfs->delete_subvolume(snapshot.path.generic_string());
		if (!result) {
			log.warn("Failed to delete snapshot: {}", result.error().message);
		} else {
			log.info("Snapshot deleted: {}", snapshot.path.generic_string());
		}
		ret.push_back(result.transform([&] { return std::move(snapshot); }));
	}
	return ret;
}

auto Recycler::archive_snapshot(Snapshot const& source, fs::path const& parent_directory) const -> Result<Snapshot> {
	auto ret = util::copy_snapshot(*m_btrfs, source, parent_directory);
	if (!ret) {
		log.warn("Failed to archive snapshot: {}, {}", source.path.generic_string(), ret.error().message);
	} else {
		log.info("Snapshot archived: {}", ret->path.generic_string());
	}
	return ret;
}

auto Recycler::populate_archive(Storage const& storage, std::span<Snapshot const> excess, std::chrono::days const period) const -> std::vector<Snapshot> {
	if (excess.empty()) { return {}; }

	auto latest_timestamp = [&] -> std::optional<Timestamp> {
		auto const existing = get_sorted_snapshots_in(storage.get_archive_directory());
		if (!existing.empty()) { return existing.back().timestamp; }
		return {};
	}();

	auto result = util::ensure_directory(storage.get_archive_directory());
	if (!result) {
		log.warn("Failed to create archive directory: {}", result.error().message);
		return {};
	}

	auto ret = std::vector<Snapshot>{};
	for (auto const& src : excess) {
		auto const delta_time = [&] -> std::optional<Timestamp> {
			if (latest_timestamp) { return src.timestamp - *latest_timestamp; }
			return {};
		}();
		if (delta_time && *delta_time < period) { continue; }

		if (auto result = archive_snapshot(src, storage.get_archive_directory())) {
			latest_timestamp = src.timestamp;
			ret.push_back(std::move(*result));
		}
	}
	return ret;
}
} // namespace btrsnap
