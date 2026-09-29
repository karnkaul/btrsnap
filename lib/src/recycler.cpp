#include "btrsnap/recycler.hpp"
#include "btrsnap/util.hpp"
#include "klib/log/typed.hpp"

namespace btrsnap {
namespace {
[[nodiscard]] auto get_excess_snapshots(std::vector<Snapshot> sorted, int const keep) -> std::vector<Snapshot> {
	auto const excess_count = int(sorted.size()) - keep;
	if (excess_count <= 0) { return {}; }

	// pop snapshots to keep.
	sorted.resize(std::size_t(excess_count));
	return sorted;
}

auto const log = klib::log::Typed<Recycler>{};
} // namespace

auto Recycler::recycle_snapshots(Config const& config) const -> Report {
	auto excess = get_excess_live_snapshots(config);
	if (excess.empty()) {
		log.info("No snapshots to recycle");
		return {};
	}

	auto ret = Report{};
	ret.archived = populate_archive(config, excess);
	if (!ret.archived.empty()) { excess = get_excess_live_snapshots(config); }

	ret.deleted = delete_snapshots(std::move(excess));

	excess = get_excess_archived_snapshots(config);
	ret.deleted.append_range(delete_snapshots(std::move(excess)));

	return ret;
}

auto Recycler::get_live_snapshots(Config const& config) const -> std::vector<Snapshot> { return get_sorted_snapshots_in(config.get_snapshots_path()); }

auto Recycler::get_archived_snapshots(Config const& config) const -> std::vector<Snapshot> { return get_sorted_snapshots_in(config.get_archive_path()); }

auto Recycler::get_excess_live_snapshots(Config const& config) const -> std::vector<Snapshot> {
	return get_excess_snapshots(get_live_snapshots(config), config.snapshot_limit);
}

auto Recycler::get_excess_archived_snapshots(Config const& config) const -> std::vector<Snapshot> {
	return get_excess_snapshots(get_archived_snapshots(config), config.archive_limit);
}

auto Recycler::delete_excess_live_snapshots(Config const& config) const -> std::vector<Result<Snapshot>> {
	return delete_snapshots(get_excess_live_snapshots(config));
}

auto Recycler::delete_excess_archived_snapshots(Config const& config) const -> std::vector<Result<Snapshot>> {
	return delete_snapshots(get_excess_archived_snapshots(config));
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

auto Recycler::populate_archive(Config const& config, std::span<Snapshot const> excess) const -> std::vector<Snapshot> {
	auto const archive_path = config.get_archive_path();
	if (archive_path.empty() || excess.empty()) { return {}; }

	auto latest_timestamp = [&] -> std::optional<Timestamp> {
		auto const existing = get_archived_snapshots(config);
		if (!existing.empty()) { return existing.back().timestamp; }
		return {};
	}();

	auto err = std::error_code{};
	if (!fs::exists(archive_path) && !fs::create_directories(archive_path, err)) {
		log.warn("Failed to create archive directory: {}", archive_path.generic_string());
		return {};
	}

	auto ret = std::vector<Snapshot>{};
	for (auto const& src : excess) {
		auto const delta_time = [&] -> std::optional<Timestamp> {
			if (latest_timestamp) { return src.timestamp - *latest_timestamp; }
			return {};
		}();
		if (delta_time && *delta_time < config.archive_period) { continue; }

		auto dst = archive_path / src.path.filename();
		fs::rename(src.path, dst, err);
		if (err != std::error_code{}) {
			log.warn("Failed to archive snapshot: {}", src.path.generic_string());
		} else {
			log.info("Snapshot archived: {}", dst.generic_string());
			latest_timestamp = src.timestamp;
			ret.push_back(Snapshot{.path = std::move(dst), .timestamp = src.timestamp});
		}
	}
	return ret;
}

auto Recycler::get_sorted_snapshots_in(fs::path const& path) const -> std::vector<Snapshot> {
	auto ret = util::list_snapshots(*m_btrfs, path);
	// move youngest to back.
	std::ranges::sort(ret, [](Snapshot const& a, Snapshot const& b) { return a.timestamp < b.timestamp; });
	return ret;
}
} // namespace btrsnap

namespace btrsnap::foo {
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

auto Recycler::get_sorted_snapshots_in(fs::path const& path) const -> std::vector<Snapshot> {
	auto ret = util::list_snapshots(*m_btrfs, path);
	// move youngest to back.
	std::ranges::sort(ret, [](Snapshot const& a, Snapshot const& b) { return a.timestamp < b.timestamp; });
	return ret;
}

auto Recycler::get_excess_snapshots(std::vector<Snapshot> sorted, int const keep) -> std::vector<Snapshot> {
	auto const excess_count = int(sorted.size()) - keep;
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

auto Recycler::populate_archive(Storage const& storage, std::span<Snapshot const> excess, std::chrono::days const period) const -> std::vector<Snapshot> {
	if (excess.empty()) { return {}; }

	auto const& archive_path = storage.get_archive_directory();

	auto latest_timestamp = [&] -> std::optional<Timestamp> {
		auto const existing = get_sorted_snapshots_in(storage.get_archive_directory());
		if (!existing.empty()) { return existing.back().timestamp; }
		return {};
	}();

	auto result = util::ensure_directory(archive_path);
	if (!result) {
		log.warn("Failed to create archive directory: {}", result.error().message);
		return {};
	}

	auto ret = std::vector<Snapshot>{};
	auto err = std::error_code{};
	for (auto const& src : excess) {
		auto const delta_time = [&] -> std::optional<Timestamp> {
			if (latest_timestamp) { return src.timestamp - *latest_timestamp; }
			return {};
		}();
		if (delta_time && *delta_time < period) { continue; }

		auto dst = archive_path / src.path.filename();
		fs::rename(src.path, dst, err);
		if (err != std::error_code{}) {
			log.warn("Failed to archive snapshot: {}", src.path.generic_string());
		} else {
			log.info("Snapshot archived: {}", dst.generic_string());
			latest_timestamp = src.timestamp;
			ret.push_back(Snapshot{.path = std::move(dst), .timestamp = src.timestamp});
		}
	}
	return ret;
}
} // namespace btrsnap::foo
