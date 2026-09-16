#include "btrsnap/subvolume.hpp"
#include "btrsnap/btrfs.hpp"
#include "btrsnap/recycler.hpp"
#include "btrsnap/util.hpp"
#include "klib/log/typed.hpp"
#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

namespace btrsnap {
namespace {
using namespace std::chrono_literals;

constexpr void clamp_limit(int& out) { out = std::max(out, 0); }
constexpr void clamp_period(std::chrono::days& out) { out = std::max(std::chrono::days{1}, out); }

struct Printer {
	void print(std::span<Snapshot const> snapshots) const {
		for (auto const [index, snapshot] : std::views::enumerate(snapshots)) {
			auto const number = int(index + 1);
			std::println(out, "{}. {}/  [{}]", number, snapshot.path.filename().string(), format_delta_time(now - snapshot.timestamp));
		}
		std::println(out);
	}

	std::ostream& out;

	Timestamp now{current_timestamp()};
};

[[nodiscard]] auto to_sorted(std::vector<Snapshot> snapshots) {
	std::ranges::sort(snapshots, [](Snapshot const& a, Snapshot const& b) { return a.timestamp > b.timestamp; });
	return snapshots;
}

auto const log = klib::log::Typed<Subvolume>{};
} // namespace

Subvolume::Subvolume(gsl::not_null<IBtrfs const*> btrfs, Config config) : m_btrfs(btrfs), m_config(std::move(config)) {}

auto Subvolume::create(gsl::not_null<IBtrfs const*> btrfs, Config config) -> Result<Subvolume> {
	auto result = btrfs->is_subvolume(config.subvolume);
	if (!result) { return std::unexpected{std::move(result.error())}; }

	auto const snapshot_directory = fs::path{config.subvolume} / config.snapshots_subdirectory;
	if (!fs::exists(snapshot_directory)) {
		result = btrfs->create_subvolume(snapshot_directory.generic_string());
		if (!result) { return std::unexpected{std::move(result.error())}; }
		log.info("Created snapshot subvolume: {}", snapshot_directory.generic_string());
	}

	result = btrfs->is_subvolume(snapshot_directory.generic_string());
	if (!result) { return std::unexpected{std::move(result.error())}; }

	clamp_limit(config.snapshot_limit);
	clamp_period(config.archive_period);
	clamp_limit(config.archive_limit);

	return Subvolume{btrfs, std::move(config)};
}

auto Subvolume::get_live_snapshots() const -> std::vector<Snapshot> { return to_sorted(util::list_snapshots(*m_btrfs, m_config.get_snapshots_path())); }

auto Subvolume::get_archived_snapshots() const -> std::vector<Snapshot> { return to_sorted(util::list_snapshots(*m_btrfs, m_config.get_archive_path())); }

auto Subvolume::take_snapshot(Timestamp const timestamp) -> Result<Snapshot> {
	auto subdirectory = m_config.get_snapshots_path() / to_pathname(timestamp);
	return m_btrfs->create_snapshot(m_config.get_subvolume_path().generic_string(), subdirectory.generic_string()).transform([&] {
		return Snapshot{.path = std::move(subdirectory), .timestamp = timestamp};
	});
}

auto Subvolume::recycle_snapshots() -> RecycleReport { return Recycler{m_btrfs}.recycle_snapshots(m_config); }

auto Subvolume::clear_all_snapshots() -> std::vector<Result<Snapshot>> {
	auto recycler = Recycler{m_btrfs};
	auto snapshots = recycler.get_live_snapshots(m_config);
	snapshots.append_range(recycler.get_archived_snapshots(m_config));
	return recycler.delete_snapshots(std::move(snapshots));
}

void Subvolume::print_snapshots(std::ostream& out, Timestamp const now) const {
	std::println(out, "{}/", m_config.get_subvolume_path().generic_string());

	auto const printer = Printer{.out = out, .now = now};

	auto snapshots = get_live_snapshots();
	std::println(out, "primary ({}/{}):", snapshots.size(), m_config.snapshot_limit);
	printer.print(snapshots);

	snapshots = get_archived_snapshots();
	std::println(out, "archive ({}/{}):", snapshots.size(), m_config.archive_limit);
	printer.print(snapshots);
}
} // namespace btrsnap
