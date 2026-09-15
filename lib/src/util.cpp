#include "btrsnap/util.hpp"

namespace btrsnap {
auto util::to_snapshot(IBtrfs const& btrfs, fs::path path) -> std::optional<Snapshot> {
	if (path.empty()) { return {}; }
	auto const timestamp = to_timestamp(path.filename().string());
	if (!timestamp || !btrfs.is_subvolume(path.generic_string())) { return {}; }
	return Snapshot{.path = std::move(path), .timestamp = *timestamp};
}

auto util::list_snapshots(IBtrfs const& btrfs, fs::path const& parent) -> std::vector<Snapshot> {
	if (parent.empty() || !fs::is_directory(parent)) { return {}; }

	auto ret = std::vector<Snapshot>{};
	auto err = std::error_code{};
	for (auto const& it : fs::directory_iterator{parent, err}) {
		if (!it.is_directory()) { continue; }
		auto snapshot = to_snapshot(btrfs, it.path());
		if (!snapshot) { continue; }
		ret.push_back(std::move(*snapshot));
	}
	return ret;
}
} // namespace btrsnap
