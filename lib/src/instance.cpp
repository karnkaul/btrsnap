#include "btrsnap/instance.hpp"
#include "btrsnap/util.hpp"
#include "klib/log/typed.hpp"

namespace btrsnap {
namespace {
auto const log = klib::log::Typed<Instance>{};
} // namespace

auto Instance::create(StorageConfig storage_config, gsl::not_null<IBtrfs const*> btrfs) -> Result<Instance> {
	return util::is_non_empty(storage_config.root)
		.and_then([&] { return util::ensure_directory(storage_config.get_snapshots_path()); })
		.and_then([&] { return util::ensure_directory(storage_config.get_archive_path()); })
		.transform([&] { return Instance{btrfs, std::move(storage_config)}; });
}

Instance::Instance(gsl::not_null<IBtrfs const*> btrfs, StorageConfig storage_config) : m_btrfs(btrfs), m_storage_config(std::move(storage_config)) {}

auto Instance::load_subvolume(SubvolumeConfig subvolume_config) const -> Result<Subvolume> {
	auto const name = subvolume_config.name;

	auto result = Subvolume::create(m_btrfs, get_storage_config(), std::move(subvolume_config));
	if (result) {
		log.info("Subvolume '{}' loaded: {}", name, result->get_config().path.generic_string());
	} else {
		log.warn("Failed to load subvolume '{}': {}", name, result.error().message);
	}
	return result;
}

auto Instance::load_subvolume_list(std::vector<SubvolumeConfig> subvolume_configs) const -> SubvolumeList {
	auto ret = SubvolumeList{};
	for (auto& subvolume_config : subvolume_configs) {
		auto subvolume = load_subvolume(std::move(subvolume_config));
		if (!subvolume) { continue; }
		ret.subvolumes.push_back(std::move(*subvolume));
	}
	return ret;
}
} // namespace btrsnap
