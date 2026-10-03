#include "btrsnap/subvolume.hpp"
#include "btrsnap/btrfs.hpp"
#include "btrsnap/recycler.hpp"
#include "klib/cli/text_table.hpp"
#include "klib/log/typed.hpp"
#include <algorithm>
#include <iostream>
#include <print>
#include <ranges>

namespace btrsnap {
namespace {
using namespace std::chrono_literals;

constexpr void clamp_limit(int& out) { out = std::max(out, 0); }
constexpr void clamp_period(std::chrono::days& out) { out = std::max(std::chrono::days{1}, out); }

struct Printer {
	void print(SnapshotList const& list) const {
		auto table = klib::TextTable::Builder{}.add_column("id").add_column(std::string{subvolume}).add_column("Age").build();

		for (auto const [index, entry] : std::views::enumerate(list.entries)) {
			std::string_view const suffix = entry.is_archived ? " [a]" : "";
			auto filename = std::format("{}{}", entry.snapshot.path.filename().string(), suffix);
			auto row = std::vector{
				std::format("{:> 2}", static_cast<int>(entry.number)),
				std::move(filename),
				format_delta_time(now - entry.snapshot.timestamp),
			};
			table.push_row(std::move(row));
		}

		std::println(out, "{}", table.serialize());
	}

	std::ostream& out;
	std::string_view subvolume;
	RecycleInfo const& recycle;

	Timestamp now{current_timestamp()};
};

auto const log = klib::log::Typed<Subvolume>{};

void on_action(Result<Snapshot> const& result, std::string_view const action) {
	if (!result) {
		log.error("Failed to {} snapshot: {}", action, result.error().message);
	} else {
		log.info("Snapshot {}d: {}", action, result->path.generic_string());
	}
}
} // namespace

auto Subvolume::create(gsl::not_null<IBtrfs const*> btrfs, StorageInfo const& storage_info, Info info) -> Result<Subvolume> {
	clamp_limit(info.recycle.archive_limit);
	clamp_period(info.recycle.archive_period);
	clamp_limit(info.recycle.archive_limit);

	return btrfs->is_subvolume(info.path.generic_string()).and_then([&] { return Storage::create(storage_info, info.name); }).transform([&](Storage storage) {
		return Subvolume(btrfs, std::move(storage), std::move(info));
	});
}

Subvolume::Subvolume(gsl::not_null<IBtrfs const*> btrfs, Storage storage, Info info) : m_btrfs(btrfs), m_storage(std::move(storage)), m_info(std::move(info)) {}

auto Subvolume::get_live_snapshots() const -> std::vector<Snapshot> {
	return Recycler{m_btrfs}.get_sorted_snapshots_in(get_storage().get_snapshots_directory());
}

auto Subvolume::get_archived_snapshots() const -> std::vector<Snapshot> {
	return Recycler{m_btrfs}.get_sorted_snapshots_in(get_storage().get_archive_directory());
}

auto Subvolume::take_snapshot(Timestamp const timestamp) -> Result<Snapshot> {
	auto subdirectory = get_storage().get_snapshots_directory() / to_pathname(timestamp);
	auto ret = m_btrfs->create_snapshot(m_info.path.generic_string(), subdirectory.generic_string()).transform([&] {
		return Snapshot{.path = std::move(subdirectory), .timestamp = timestamp};
	});
	on_action(ret, "save");
	return ret;
}

auto Subvolume::recycle_snapshots() -> RecycleReport { return Recycler{m_btrfs}.recycle_snapshots(m_storage, m_info.recycle); }

auto Subvolume::clear_all_snapshots() -> std::vector<Result<Snapshot>> {
	auto snapshots = get_live_snapshots();
	snapshots.append_range(get_archived_snapshots());
	return Recycler{m_btrfs}.delete_snapshots(std::move(snapshots));
}

void Subvolume::print_snapshots(std::ostream& out, Timestamp const now) const {
	auto const printer = Printer{.out = out, .subvolume = m_info.name, .recycle = m_info.recycle, .now = now};
	printer.print(Recycler{m_btrfs}.get_snapshot_list(m_storage));
}
} // namespace btrsnap
