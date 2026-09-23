#include "btrsnap/instance.hpp"
#include "klib/log/typed.hpp"

namespace btrsnap {
namespace {
auto const log = klib::log::Typed<Instance>{};
} // namespace

auto Instance::load_subvolume(Config config) -> Result<void> {
	auto result = Subvolume::create(m_btrfs, std::move(config));
	if (!result) {
		log.warn("Failed to load subvolume: {}", result.error().message);
		return std::unexpected{std::move(result.error())};
	}

	auto const& cfg = result->get_config();
	log.info("Subvolume loaded: {} ({}, {}, {})", cfg.get_subvolume_path().generic_string(), cfg.snapshot_limit, cfg.archive_period, cfg.archive_limit);
	m_subvolumes.push_back(std::move(*result));
	return {};
}

void Instance::clear_loaded_subvolumes() {
	m_subvolumes.clear();
	log.info("Subvolumes cleared");
}

auto Instance::take_snapshots(Timestamp const timestamp) -> std::vector<Result<Snapshot>> {
	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& subvolume : m_subvolumes) { ret.push_back(subvolume.take_snapshot(timestamp)); }
	return ret;
}

auto Instance::recycle_snapshots() -> RecycleReport {
	auto ret = RecycleReport{};
	for (auto& subvolume : m_subvolumes) { ret.append(subvolume.recycle_snapshots()); }
	return ret;
}

auto Instance::clear_all_snapshots() -> std::vector<Result<Snapshot>> {
	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& subvolume : m_subvolumes) { ret.append_range(subvolume.clear_all_snapshots()); }
	return ret;
}

void Instance::print_snapshots(std::ostream& out) const {
	auto const now = current_timestamp();
	for (auto const& subvolume : m_subvolumes) { subvolume.print_snapshots(out, now); }
}
} // namespace btrsnap
