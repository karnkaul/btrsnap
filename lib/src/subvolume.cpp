#include "btrsnap/subvolume.hpp"
#include "btrsnap/btrfs.hpp"
#include "btrsnap/util.hpp"
#include "klib/log/typed.hpp"
#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

namespace btrsnap {
namespace {
using namespace std::chrono_literals;

[[nodiscard]] auto delete_snapshot(IBtrfs const& btrfs, Snapshot snapshot) -> Result<Snapshot> {
	return btrfs.delete_subvolume(snapshot.path.generic_string()).transform([&] { return std::move(snapshot); });
}

constexpr void clamp_limit(int& out) { out = std::max(out, 0); }
constexpr void clamp_period(std::chrono::days& out) { out = std::max(std::chrono::days{1}, out); }

auto trim_front(std::span<Snapshot> list, std::uint32_t const keep) -> std::span<Snapshot> {
	if (list.size() < std::size_t(keep)) { return {}; }
	return list.subspan(std::size_t(keep));
}

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

auto Subvolume::delete_live_snapshots(std::uint32_t const keep) -> std::vector<Result<Snapshot>> {
	auto snapshots = get_live_snapshots();
	return delete_snapshots_from(std::move(snapshots), keep);
}

auto Subvolume::delete_archived_snapshots(std::uint32_t const keep) -> std::vector<Result<Snapshot>> {
	auto snapshots = get_archived_snapshots();
	return delete_snapshots_from(std::move(snapshots), keep);
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

auto Subvolume::delete_snapshots_from(std::vector<Snapshot> snapshots, std::uint32_t const keep) const -> std::vector<Result<Snapshot>> {
	auto const to_delete = trim_front(snapshots, keep);

	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& snapshot : to_delete) { ret.push_back(delete_snapshot(*m_btrfs, std::move(snapshot))); }

	return ret;
}
} // namespace btrsnap
