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
	return btrfs.delete_subvolume(snapshot.path.generic_string()).transform([&] { return std::move(snapshot); });
}

constexpr void clamp_limit(int& out) { out = std::max(out, 0); }
constexpr void clamp_period(std::chrono::days& out) { out = std::max(std::chrono::days{1}, out); }

auto trim_front(std::span<Snapshot> list, std::uint32_t const keep) -> std::span<Snapshot> {
	if (list.size() < std::size_t(keep)) { return {}; }
	return list.subspan(std::size_t(keep));
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

auto Subvolume::take_snapshot(Timestamp const timestamp) -> Result<Snapshot> {
	auto subdirectory = m_config.get_snapshots_path() / to_pathname(timestamp);
	return m_btrfs->create_snapshot(m_config.get_subvolume_path().generic_string(), subdirectory.generic_string()).transform([&] {
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
		.subvolume = m_config.get_subvolume_path(),
		.snapshots_limit = m_config.snapshot_limit,
		.archive_period = m_config.archive_period,
		.archive_limit = m_config.archive_limit,
	};
	push_snapshots_to(ret.primary, m_config.get_snapshots_path());
	if (auto const archive_path = m_config.get_archive_path(); !archive_path.empty()) { push_snapshots_to(ret.archived, archive_path); }
	return ret;
}

void Subvolume::push_snapshots_to(std::vector<Snapshot>& out, fs::path const& path) const {
	auto err = std::error_code{};
	for (auto const& it : fs::directory_iterator{path, err}) {
		if (!it.is_directory()) { continue; }
		auto const& path = it.path();
		if (!m_btrfs->is_subvolume(path.generic_string())) { continue; }
		auto snapshot = to_snapshot(path);
		if (!snapshot) { continue; }
		out.push_back(std::move(*snapshot));
	}
	std::ranges::sort(out, [](Snapshot const& a, Snapshot const& b) { return a.timestamp > b.timestamp; });
}
} // namespace btrsnap
