#include "btrsnap/subvolume.hpp"
#include "btrsnap/btrfs.hpp"
#include "klib/log/typed.hpp"
#include <algorithm>
#include <optional>

namespace btrsnap {
namespace {
using namespace std::chrono_literals;

[[nodiscard]] auto to_snapshot(fs::path path) -> std::optional<Snapshot> {
	auto const timestamp = to_timestamp(path.filename().string());
	if (!timestamp) { return {}; }
	return Snapshot{.path = std::move(path), .timestamp = *timestamp};
}

[[nodiscard]] auto delete_snapshot(IBtrfs const& btrfs, Snapshot snapshot) -> Result<Snapshot> {
	return btrfs.delete_subvolume(snapshot.path.string()).transform([&] { return std::move(snapshot); });
}

[[nodiscard]] auto clamped_limit(int const in) { return std::max(in, 0); }
[[nodiscard]] auto clamped_period(std::chrono::days const in) { return std::max(in, std::chrono::days{1}); }

auto trim_front(std::span<Snapshot> list, std::uint32_t const keep) -> std::span<Snapshot> {
	if (list.size() < std::size_t(keep)) { return {}; }
	return list.subspan(std::size_t(keep));
}
auto const log = klib::log::Typed<Subvolume>{};
} // namespace

Subvolume::Subvolume(gsl::not_null<IBtrfs const*> btrfs, fs::path path, fs::path snapshots, Config const& config)
	: m_btrfs(btrfs), m_path(std::move(path)), m_snapshots_directory(std::move(snapshots)), m_snapshot_limit(clamped_limit(config.snapshot_limit)),
	  m_archive_period(clamped_period(config.archive_period)), m_archive_limit(clamped_limit(config.archive_limit)) {
	if (!config.archive_subdirectory.empty()) { m_archive_directory = m_snapshots_directory / config.archive_subdirectory; }
}

auto Subvolume::create(gsl::not_null<IBtrfs const*> btrfs, Config const& config) -> Result<Subvolume> {
	auto result = btrfs->is_subvolume(config.subvolume);
	if (!result) { return std::unexpected{std::move(result.error())}; }

	auto snapshot_directory = fs::path{config.subvolume} / config.snapshots_subdirectory;
	if (!fs::exists(snapshot_directory)) {
		result = btrfs->create_subvolume(snapshot_directory.string());
		if (!result) { return std::unexpected{std::move(result.error())}; }
		log.info("Created snapshot subvolume: {}", snapshot_directory.string());
	}

	result = btrfs->is_subvolume(snapshot_directory.string());
	if (!result) { return std::unexpected{std::move(result.error())}; }

	return Subvolume{btrfs, config.subvolume, std::move(snapshot_directory), config};
}

auto Subvolume::take_snapshot(Timestamp const timestamp) -> Result<Snapshot> {
	auto subdirectory = m_snapshots_directory / to_pathname(timestamp);
	return m_btrfs->create_snapshot(m_path.string(), subdirectory.string()).transform([&] {
		return Snapshot{.path = std::move(subdirectory), .timestamp = timestamp};
	});
}

auto Subvolume::delete_snapshots(std::uint32_t const keep) -> std::vector<Result<Snapshot>> {
	auto manifest = build_manifest();
	auto const to_delete = trim_front(manifest.primary, keep);

	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& snapshot : to_delete) { ret.push_back(delete_snapshot(*m_btrfs, std::move(snapshot))); }

	return ret;
}

auto Subvolume::build_manifest() const -> Manifest {
	auto ret = Manifest{
		.subvolume = m_path,
		.snapshots_limit = get_snapshots_limit(),
		.archive_period = get_archive_period(),
		.archive_limit = get_archive_limit(),
	};
	push_snapshots_to(ret.primary, m_snapshots_directory);
	if (!m_archive_directory.empty()) { push_snapshots_to(ret.archived, m_archive_directory); }
	return ret;
}

void Subvolume::push_snapshots_to(std::vector<Snapshot>& out, fs::path const& path) const {
	auto err = std::error_code{};
	for (auto const& it : fs::directory_iterator{path, err}) {
		if (!it.is_directory()) { continue; }
		auto const& path = it.path();
		if (path == m_snapshots_directory || path == m_archive_directory) { continue; }
		if (!m_btrfs->is_subvolume(path.generic_string())) { continue; }
		auto snapshot = to_snapshot(path);
		if (!snapshot) { continue; }
		out.push_back(std::move(*snapshot));
	}
	std::ranges::sort(out, [](Snapshot const& a, Snapshot const& b) { return a.timestamp > b.timestamp; });
}
} // namespace btrsnap
