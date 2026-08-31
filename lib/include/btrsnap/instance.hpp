#pragma once
#include "btrsnap/config.hpp"
#include "btrsnap/subvolume.hpp"
#include "klib/log/typed.hpp"
#include <iosfwd>

namespace btrsnap {
class LoadedSubvolume : public Subvolume {
  public:
	explicit LoadedSubvolume(Subvolume subvolume, int snapshot_limit);

	int snapshot_limit;
};

class Instance {
  public:
	auto load_subvolume(Config const& config) -> Result<void>;
	[[nodiscard]] auto get_loaded_subvolumes() const -> std::span<LoadedSubvolume const> { return m_subvolumes; }
	void clear_loaded_subvolumes();

	void print_snapshots(std::ostream& out) const;
	auto take_snapshots(Clock::time_point timestamp = Clock::now()) -> std::vector<Result<Snapshot>>;
	auto trim_snapshots() -> std::vector<Result<Snapshot>>;
	auto clear_snapshots() -> std::vector<Result<Snapshot>>;

  private:
	void on_save(Result<Snapshot> const& result) const;
	void on_delete(std::span<Result<Snapshot> const> results) const;

	auto delete_snapshots(int keep) -> std::vector<Result<Snapshot>>;

	klib::log::Typed<Instance> m_log{};

	std::vector<LoadedSubvolume> m_subvolumes{};
};
} // namespace btrsnap
