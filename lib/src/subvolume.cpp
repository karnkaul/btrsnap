#include "btrsnap/subvolume.hpp"
#include "btrsnap/btrfs.hpp"
#include "klib/log/typed.hpp"
#include <algorithm>

namespace btrsnap {
namespace {
[[nodiscard]] auto to_snapshot(fs::path path) {
	auto const timestamp = to_timestamp(path.filename().string());
	return Snapshot{.path = std::move(path), .timestamp = timestamp};
}

[[nodiscard]] auto delete_snapshot(Snapshot snapshot) -> Result<Snapshot> {
	return btrfs::delete_subvolume(snapshot.path.string()).transform([&] { return std::move(snapshot); });
}

auto const log = klib::log::Typed<Subvolume>{};
} // namespace

auto Subvolume::create(fs::path path, std::string_view const snapshot_subdirectory) -> Result<Subvolume> {
	auto result = btrfs::is_subvolume(path.string());
	if (!result) { return std::unexpected{std::move(result.error())}; }

	auto snapshot_directory = path / snapshot_subdirectory;
	if (!fs::exists(snapshot_directory)) {
		result = btrfs::create_subvolume(snapshot_directory.string());
		if (!result) { return std::unexpected{std::move(result.error())}; }
		log.info("Created snapshot subvolume: {}", snapshot_directory.string());
	}

	result = btrfs::is_subvolume(snapshot_directory.string());
	if (!result) { return std::unexpected{std::move(result.error())}; }

	return Subvolume{std::move(path), std::move(snapshot_directory)};
}

auto Subvolume::get_all_snapshots() const -> std::vector<Snapshot> {
	auto ret = std::vector<Snapshot>{};
	auto err = std::error_code{};
	for (auto const& it : fs::directory_iterator{m_snapshot_directory, err}) {
		if (!it.is_directory()) { continue; }
		if (!btrfs::is_subvolume(it.path().string())) { continue; }
		ret.push_back(to_snapshot(it.path()));
	}
	return ret;
}

auto Subvolume::take_snapshot(Clock::time_point const timestamp) -> Result<Snapshot> {
	auto subdirectory = m_snapshot_directory / to_pathname(timestamp);
	return btrfs::create_snapshot(m_path.string(), subdirectory.string()).transform([&] {
		return Snapshot{.path = std::move(subdirectory), .timestamp = timestamp};
	});
}

Subvolume::Snapshots::Snapshots(Subvolume const& subvolume) : m_snapshots(subvolume.get_all_snapshots()) {
	std::erase_if(m_snapshots, [](Snapshot const& snapshot) { return !snapshot.timestamp.has_value(); });
	// move youngest snapshots to front.
	std::ranges::sort(m_snapshots, [](Snapshot const& a, Snapshot const& b) { return a.timestamp > b.timestamp; });
}

auto Subvolume::Snapshots::trim_snapshots(std::uint32_t const keep) const -> std::vector<Result<Snapshot>> {
	auto to_delete = std::span{m_snapshots};
	if (to_delete.size() < std::size_t(keep)) { return {}; }
	to_delete = to_delete.subspan(std::size_t(keep));
	if (to_delete.empty()) { return {}; }

	auto ret = std::vector<Result<Snapshot>>{};
	ret.reserve(to_delete.size());
	for (auto const& snapshot : to_delete) { ret.push_back(delete_snapshot(snapshot)); }
	return ret;
}
} // namespace btrsnap
