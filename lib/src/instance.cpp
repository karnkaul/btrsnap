#include "btrsnap/instance.hpp"
#include "btrsnap/util.hpp"
#include "klib/log/typed.hpp"

namespace btrsnap {
namespace {
auto const log = klib::log::Typed<Instance>{};
} // namespace

auto Instance::create(StorageInfo storage_info, gsl::not_null<IBtrfs const*> btrfs) -> Result<Instance> {
	return util::is_non_empty(storage_info.root)
		.and_then([&] { return util::ensure_directory(storage_info.get_snapshots_path()); })
		.and_then([&] { return util::ensure_directory(storage_info.get_archive_path()); })
		.transform([&] { return Instance{btrfs, std::move(storage_info)}; });
}

Instance::Instance(gsl::not_null<IBtrfs const*> btrfs, StorageInfo storage_info) : m_btrfs(btrfs), m_storage_info(std::move(storage_info)) {}

auto Instance::load_subvolume(SubvolumeInfo subvolume_info) -> Result<void> {
	auto const name = subvolume_info.name;

	auto result = Subvolume::create(m_btrfs, get_storage_info(), std::move(subvolume_info));
	if (!result) {
		log.warn("Failed to load subvolume '{}': {}", name, result.error().message);
		return std::unexpected{std::move(result.error())};
	}

	log.info("Subvolume '{}' loaded: {}", name, result->get_info().path.generic_string());
	m_subvolumes.push_back(std::move(*result));
	return {};
}

void Instance::load_subvolumes(std::vector<SubvolumeInfo> subvolume_infos) {
	for (auto& subvolume_info : subvolume_infos) { auto const _ = load_subvolume(std::move(subvolume_info)); }
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
