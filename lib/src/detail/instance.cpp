#include "detail/instance.hpp"
#include "detail/log.hpp"
#include <filesystem>
#include <print>

namespace btrsnap::detail {
auto Instance::create() -> std::optional<Instance> {
	auto const configs = Config::from_directory();
	if (configs.empty()) {
		std::println("No configs found in {}", Config::directory_v);
		return {};
	}
	return create_impl(configs);
}

auto Instance::create(klib::CString const custom_config_path) -> std::optional<Instance> {
	auto const config = Config::from_file(custom_config_path);
	if (!config) {
		std::println("Failed to load custom config: {}", custom_config_path);
		return {};
	}

	return create_impl({&*config, 1});
}

Instance::Instance(std::span<Config const> configs) {
	for (auto const& config : configs) {
		auto subvolume = Subvolume::create(config);
		if (!subvolume) { continue; }
		m_subvolumes.push_back(std::move(*subvolume));
	}
}

void Instance::list_snapshots() {
	for (auto& subvolume : m_subvolumes) { subvolume.list_snapshots(); }
}

auto Instance::take_snapshots(Flag const flags) -> bool {
	log.info("[Instance] Taking snapshots...");

	auto success = 0;
	for (auto& subvolume : m_subvolumes) {
		if (subvolume.take_snapshot()) { ++success; }
	}
	auto const ret = success > 0;
	auto const failed = int(m_subvolumes.size()) - success;
	if (ret) { log.info("[Instance] {} snapshot(s) successfully saved", success); }
	if (failed > 0) { log.error("[Instance] Failed to take {} snapshot(s)", failed); }

	if ((flags & Flag::NoTrim) == Flag::None) {
		if (success == 0) {
			log.info("[Instance] All snapshots failed, skipping trim");
		} else {
			trim_snapshots();
		}
	}

	return ret;
}

void Instance::trim_snapshots() {
	log.info("[Instance] Trimming snapshots...");
	delete_snapshots([](Subvolume const& subvolume) { return subvolume.get_snapshot_limit(); });
}

auto Instance::clear_snapshots() -> bool {
	log.info("[Instance] Clearing all snapshots...");
	if (delete_snapshots([]([[maybe_unused]] Subvolume const& subvolume) { return 0; })) { return true; }
	log.error("[Instance] Failed to clear snapshots");
	return false;
}

auto Instance::create_impl(std::span<Config const> configs) -> std::optional<Instance> {
	auto ret = Instance{configs};
	if (ret.m_subvolumes.empty()) {
		std::println("No valid subvolumes in loaded configs");
		return {};
	}

	if constexpr (klib::log::debug_enabled_v) {
		auto subvolumes_text = std::string{};
		for (auto const& subvolume : ret.m_subvolumes) {
			if (!subvolumes_text.empty()) { subvolumes_text.append(", "); }
			subvolumes_text.append(subvolume.get_path().string());
		}
		log.info("[Instance] {} Subvolumes loaded: {}", ret.m_subvolumes.size(), subvolumes_text);
	}

	return ret;
}

template <typename F>
auto Instance::delete_snapshots(F get_keep) -> bool {
	auto ret = true;
	for (auto& subvolume : m_subvolumes) {
		auto const keep = get_keep(subvolume);
		ret &= subvolume.delete_snapshots(keep);
	}
	return ret;
}
} // namespace btrsnap::detail
