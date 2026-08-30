#include "detail/subvolume.hpp"
#include "detail/btrfs_api.hpp"
#include "detail/log.hpp"
#include "detail/shell.hpp"
#include <algorithm>
#include <print>
#include <ranges>

namespace btrsnap::detail {
namespace {
[[nodiscard]] constexpr auto clamp_snapshot_limit(int const limit) { return std::max(0, limit); }
} // namespace

auto SnapshotInfo::create(fs::path path) -> SnapshotInfo {
	auto const timestamp = to_timestamp(path.filename().string());
	return SnapshotInfo{
		.path = std::move(path),
		.timestamp = timestamp,
	};
}

auto Subvolume::create(Config const& config) -> std::optional<Subvolume> {
	auto const is_subvolume = btrfs::is_subvolume(config.subvolume);
	if (!is_subvolume) {
		log.error("[Subvolume] {}: {} ({})", is_subvolume.error().text, config.subvolume, is_subvolume.error().code);
		return {};
	}

	auto ret = Subvolume{config};
	if (fs::exists(ret.m_save_directory) && !fs::is_directory(ret.m_save_directory)) {
		log.error("[Subvolume] Snapshots path is not a directory: '{}'", ret.m_save_directory.string());
		return {};
	}

	return ret;
}

Subvolume::Subvolume(Config const& config)
	: m_path(config.subvolume), m_save_directory(m_path / config.subdirectory), m_snapshot_limit(clamp_snapshot_limit(config.limit)) {}

void Subvolume::list_snapshots() {
	populate_snapshots();
	auto const total = std::ranges::count_if(m_snapshots, [](SnapshotInfo const& snapshot) { return snapshot.timestamp.has_value(); });
	std::println("{} ({}/{}):", m_save_directory.string(), total, m_snapshot_limit);
	for (auto const [index, snapshot] : std::views::enumerate(m_snapshots)) {
		auto const number = int(index + 1);
		std::string_view const scope = snapshot.timestamp ? "" : "*";
		std::println("{}{}. {}", number, scope, snapshot.path.filename().string());
	}
	std::println();
}

auto Subvolume::take_snapshot() -> bool {
	if (!create_save_directory()) { return false; }

	auto const dirname = to_pathname(Clock::now());
	auto const subdirectory = m_save_directory / dirname;
	auto const expression = std::format("btrfs subvolume snapshot {} {}", m_path.string(), subdirectory.string());
	if (shell::execute(expression)) { return true; }

	log.error("[Subvolume] Failed to take snapshot of subvolume: '{}'", m_path.string());
	return false;
}

void Subvolume::fill_snapshots_to_delete(std::vector<SnapshotInfo>& out, int const keep) {
	KLIB_ASSERT(keep >= 0);
	populate_snapshots();

	std::erase_if(m_snapshots, [](SnapshotInfo const& snapshot) { return !snapshot.timestamp; });
	auto to_remove = std::span{m_snapshots};
	log.info("[Subvolume] '{}' : keep: {}, timestamped: {}", m_path.string(), keep, to_remove.size());
	if (int(to_remove.size()) < keep) { return; }

	to_remove = to_remove.subspan(std::size_t(keep));
	out.append_range(to_remove);
}

auto Subvolume::delete_snapshots(std::span<SnapshotInfo const> snapshots) -> bool {
	if (snapshots.empty()) { return true; }

	auto expression = std::string{"btrfs subvolume delete -c"};
	for (auto const& snapshot : snapshots) { std::format_to(std::back_inserter(expression), " {}", snapshot.path.string()); }

	if (shell::execute(expression)) { return true; }

	log.warn("[Subvolume] One or more snapshots failed to be deleted");
	return false;
}

void Subvolume::populate_snapshots() {
	m_snapshots.clear();
	if (!fs::is_directory(m_save_directory)) { return; }

	auto err = std::error_code{};
	for (auto const& it : fs::directory_iterator{m_save_directory, err}) {
		if (!it.is_directory()) { continue; }
		if (!btrfs::is_subvolume(it.path().string())) { continue; }
		m_snapshots.push_back(SnapshotInfo::create(it.path()));
	}
	std::ranges::sort(m_snapshots, [](SnapshotInfo const& a, SnapshotInfo const& b) { return a.timestamp > b.timestamp; });
}

auto Subvolume::create_save_directory() -> bool {
	if (fs::exists(m_save_directory)) { return true; }

	auto err = std::error_code{};
	if (fs::create_directories(m_save_directory, err)) { return true; }

	log.error("[Subvolume] Failed to create snapshots subdirectory: '{}'", m_save_directory.string());
	return false;
}
} // namespace btrsnap::detail
