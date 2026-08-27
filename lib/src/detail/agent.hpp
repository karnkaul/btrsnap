#pragma once
#include "detail/config.hpp"
#include "detail/snapshot_info.hpp"
#include <filesystem>

namespace btrsnap::detail {
namespace fs = std::filesystem;

class Agent {
  public:
	[[nodiscard]] static auto create(Config const& config) -> std::optional<Agent>;

	[[nodiscard]] auto get_subvolume_path() const -> fs::path const& { return m_subvolume; }
	[[nodiscard]] auto get_limit() const -> int { return m_limit; }

	void list_snapshots();
	auto take_snapshot() -> bool;
	void fill_snapshots_to_delete(std::vector<SnapshotInfo>& out, int keep);

	static auto delete_snapshots(std::span<SnapshotInfo const> snapshots) -> bool;

  private:
	explicit Agent(Config const& config);

	void populate_snapshots();
	auto create_save_directory() -> bool;

	fs::path m_subvolume{};
	fs::path m_save_directory{};
	std::vector<SnapshotInfo> m_snapshots{};
	int m_limit{};
};
} // namespace btrsnap::detail
