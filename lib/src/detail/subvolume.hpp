#pragma once
#include "detail/clock.hpp"
#include "detail/config.hpp"
#include <filesystem>

namespace btrsnap::detail {
namespace fs = std::filesystem;

struct SnapshotInfo {
	[[nodiscard]] static auto create(fs::path path) -> SnapshotInfo;

	fs::path path{};
	std::optional<Clock::time_point> timestamp{};
};

class Subvolume {
  public:
	[[nodiscard]] static auto create(Config const& config) -> std::optional<Subvolume>;

	[[nodiscard]] auto get_path() const -> fs::path const& { return m_path; }
	[[nodiscard]] auto get_snapshot_limit() const -> int { return m_snapshot_limit; }

	void list_snapshots();
	auto take_snapshot() -> bool;
	auto delete_snapshots(int keep) -> bool;

  private:
	explicit Subvolume(Config const& config);

	void populate_snapshots();
	auto create_save_directory() -> bool;

	fs::path m_path{};
	fs::path m_save_directory{};
	std::vector<SnapshotInfo> m_snapshots{};
	int m_snapshot_limit{};
};
} // namespace btrsnap::detail
