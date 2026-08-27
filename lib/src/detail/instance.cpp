#include "detail/instance.hpp"
#include "detail/agent.hpp"
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

	auto ret = Instance{configs};
	if (ret.m_agents.empty()) {
		std::println("No valid subvolumes in loaded configs");
		return {};
	}

	if constexpr (klib::log::debug_enabled_v) {
		auto agents_text = std::string{};
		for (auto const& agent : ret.m_agents) {
			if (!agents_text.empty()) { agents_text.append(", "); }
			agents_text.append(agent.get_subvolume_path().string());
		}
		log.debug("[Instance] {} Agents created: {}", ret.m_agents.size(), agents_text);
	}

	return ret;
}

Instance::Instance(std::span<Config const> configs) {
	for (auto const& config : configs) {
		auto agent = Agent::create(config);
		if (!agent) { continue; }
		m_agents.push_back(std::move(*agent));
	}
}

void Instance::list_snapshots() {
	for (auto& agent : m_agents) { agent.list_snapshots(); }
}

auto Instance::take_snapshots(Flag const flags) -> bool {
	auto ret = true;
	for (auto& agent : m_agents) { ret &= agent.take_snapshot(); }

	if ((flags & Flag::NoTrim) == Flag::None) { trim_snapshots(); }

	return ret;
}

void Instance::trim_snapshots() {
	auto to_delete = std::vector<SnapshotInfo>{};
	for (auto& agent : m_agents) { agent.fill_snapshots_to_delete(to_delete, agent.get_limit()); }
	Agent::delete_snapshots(to_delete);
}

auto Instance::clear_snapshots() -> bool {
	auto to_delete = std::vector<SnapshotInfo>{};
	for (auto& agent : m_agents) { agent.fill_snapshots_to_delete(to_delete, 0); }
	return Agent::delete_snapshots(to_delete);
}
} // namespace btrsnap::detail
