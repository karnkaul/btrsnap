#pragma once
#include "btrsnap/btrfs.hpp"
#include "btrsnap/config.hpp"
#include "btrsnap/result.hpp"
#include "btrsnap/snapshot.hpp"
#include <gsl/pointers>

namespace btrsnap {
class Subvolume {
  public:
	[[nodiscard]] static auto create(gsl::not_null<IBtrfs const*> btrfs, Config const& config) -> Result<Subvolume>;

	[[nodiscard]] auto get_path() const -> fs::path const& { return m_path; }

	[[nodiscard]] auto get_snapshots_directory() const -> fs::path const& { return m_snapshots_directory; }
	[[nodiscard]] auto get_snapshots_limit() const -> int { return m_snapshot_limit; }

	[[nodiscard]] auto get_archive_directory() const -> fs::path const& { return m_archive_directory; }
	[[nodiscard]] auto get_archive_period() const -> std::chrono::days { return m_archive_period; }
	[[nodiscard]] auto get_archive_limit() const -> int { return m_archive_limit; }

	[[nodiscard]] auto take_snapshot(Timestamp timestamp) -> Result<Snapshot>;
	[[nodiscard]] auto delete_snapshots(std::uint32_t keep) -> std::vector<Result<Snapshot>>;

	[[nodiscard]] auto build_manifest() const -> Manifest;

  private:
	explicit Subvolume(gsl::not_null<IBtrfs const*> btrfs, fs::path path, fs::path snapshots, Config const& config);

	void push_snapshots_to(std::vector<Snapshot>& out, fs::path const& path) const;

	gsl::not_null<IBtrfs const*> m_btrfs;

	fs::path m_path{};
	fs::path m_snapshots_directory{};
	fs::path m_archive_directory{};

	int m_snapshot_limit{};
	std::chrono::days m_archive_period{};
	int m_archive_limit{};
};
} // namespace btrsnap
