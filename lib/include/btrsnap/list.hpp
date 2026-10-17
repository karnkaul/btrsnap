#pragma once
#include "btrsnap/subvolume.hpp"

namespace btrsnap {
class SnapshotList {
  public:
	explicit SnapshotList(Subvolume const& subvolume);

	using Number = SnapshotNumber;

	struct Entry {
		Number number{};
		Snapshot snapshot{};
		bool is_archived{};
	};

	[[nodiscard]] auto get_entries() const -> std::span<Entry const> { return m_entries; }
	[[nodiscard]] auto find_entry(Number number) const -> klib::Ptr<Entry const>;
	[[nodiscard]] auto find_snapshot(Number number) const -> klib::Ptr<Snapshot const>;

	auto delete_snapshot(Number number) -> Result<Snapshot>;

  private:
	gsl::not_null<IBtrfs const*> m_btrfs;
	std::vector<Entry> m_entries{};
};

class SubvolumeList {
  public:
	[[nodiscard]] auto take_snapshots(Timestamp timestamp = current_timestamp()) -> std::vector<Result<Snapshot>>;
	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;
	void print_snapshots(std::ostream& out, Timestamp now = current_timestamp()) const;

	[[nodiscard]] auto find_by_name(std::string_view name) const -> klib::Ptr<Subvolume const>;
	[[nodiscard]] auto find_by_path(fs::path const& path) const -> klib::Ptr<Subvolume const>;

	std::vector<Subvolume> subvolumes{};
};
} // namespace btrsnap
