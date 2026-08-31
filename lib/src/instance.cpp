#include "btrsnap/instance.hpp"
#include "klib/log/typed.hpp"
#include <algorithm>
#include <ostream>
#include <print>
#include <ranges>

namespace btrsnap {
namespace {
auto const log = klib::log::Typed<Instance>{};

void on_save(Result<Snapshot> const& result) {
	if (!result) {
		log.error("Failed to take snapshot: {}", result.error().message);
	} else {
		log.info("Snapshot saved: {}", result->path.string());
	}
}

void on_delete(std::span<Result<Snapshot> const> results) {
	for (auto const& result : results) {
		if (!result) {
			log.error("Failed to delete snapshot: {}", result.error().message);
		} else {
			log.info("Snapshot deleted: {}", result->path.string());
		}
	}
}
} // namespace

LoadedSubvolume::LoadedSubvolume(Subvolume subvolume, int const snapshot_limit)
	: Subvolume(std::move(subvolume)), snapshot_limit(std::max(snapshot_limit, 0)) {}

auto Instance::load_subvolume(Config const& config) -> Result<void> {
	auto result = Subvolume::create(config.subvolume, config.subdirectory);
	if (!result) {
		log.warn("Failed to load subvolume: {}", result.error().message);
		return std::unexpected{std::move(result.error())};
	}

	auto const& loaded_subvolume = m_subvolumes.emplace_back(std::move(*result), config.limit);
	log.info("Subvolume loaded: {} (snapshot limit: {})", loaded_subvolume.get_path().string(), loaded_subvolume.snapshot_limit);
	return {};
}

void Instance::clear_loaded_subvolumes() {
	m_subvolumes.clear();
	log.info("Subvolumes cleared");
}

void Instance::print_snapshots(std::ostream& out) const {
	for (auto const& subvolume : m_subvolumes) {
		auto const snapshots = Subvolume::Snapshots{subvolume};
		std::println(out, "{}/  ({}/{}):", subvolume.get_snapshot_directory().string(), snapshots.get_snapshots().size(), subvolume.snapshot_limit);
		for (auto const [index, snapshot] : std::views::enumerate(snapshots.get_snapshots())) {
			auto const number = int(index + 1);
			std::println(out, "{}. {}/", number, snapshot.path.filename().string());
		}
		std::println(out);
	}
}

auto Instance::take_snapshots(Clock::time_point const timestamp) -> std::vector<Result<Snapshot>> {
	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& subvolume : m_subvolumes) {
		auto result = subvolume.take_snapshot(timestamp);
		on_save(result);
		ret.push_back(std::move(result));
	}
	return ret;
}

auto Instance::trim_snapshots() -> std::vector<Result<Snapshot>> { return delete_snapshots(-1); }

auto Instance::clear_snapshots() -> std::vector<Result<Snapshot>> { return delete_snapshots(0); }

auto Instance::delete_snapshots(int const keep) -> std::vector<Result<Snapshot>> {
	auto ret = std::vector<Result<Snapshot>>{};
	for (auto const& subvolume : m_subvolumes) {
		auto const to_keep = keep < 0 ? subvolume.snapshot_limit : keep;
		auto snapshots = Subvolume::Snapshots{subvolume};
		auto results = snapshots.trim_snapshots(std::uint32_t(to_keep));
		on_delete(results);
		ret.append_range(std::move(results));
	}
	return ret;
}

} // namespace btrsnap
