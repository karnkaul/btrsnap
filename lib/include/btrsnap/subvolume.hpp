#pragma once
#include "btrsnap/result.hpp"
#include "btrsnap/snapshot.hpp"
#include <span>
#include <vector>

namespace btrsnap {
class Subvolume {
  public:
	class Snapshots;

	[[nodiscard]] static auto create(fs::path path, std::string_view snapshot_subdirectory) -> Result<Subvolume>;

	[[nodiscard]] auto get_path() const -> fs::path const& { return m_path; }
	[[nodiscard]] auto get_snapshot_directory() const -> fs::path const& { return m_snapshot_directory; }

	[[nodiscard]] auto get_all_snapshots() const -> std::vector<Snapshot>;
	[[nodiscard]] auto take_snapshot(Clock::time_point timestamp) -> Result<Snapshot>;

  private:
	explicit Subvolume(fs::path path, fs::path snapshot_directory) : m_path(std::move(path)), m_snapshot_directory(std::move(snapshot_directory)) {}

	fs::path m_path{};
	fs::path m_snapshot_directory{};
};

class Subvolume::Snapshots {
  public:
	explicit Snapshots(Subvolume const& subvolume);

	[[nodiscard]] auto get_snapshots() const -> std::span<Snapshot const> { return m_snapshots; }
	[[nodiscard]] auto trim_snapshots(std::uint32_t keep) const -> std::vector<Result<Snapshot>>;

  private:
	std::vector<Snapshot> m_snapshots{};
};
} // namespace btrsnap
