module;

#include <gsl/pointers>

module btrsnap.subvolume;

import klib.log;

#pragma region SubvolumeStorage

namespace btrsnap {
auto SubvolumeStorage::create(StorageConfig const& config, std::string_view const subvolume_name)
	-> Result<SubvolumeStorage> {
	if (subvolume_name.empty()) {
		return to_error(Error::Type::InvalidArgument, "Subvolume name is empty");
	}
	return SubvolumeStorage{config.get_snapshots_path() / subvolume_name, config.get_archive_path() / subvolume_name};
}

SubvolumeStorage::SubvolumeStorage(fs::path snapshots_directory, fs::path archive_directory)
	: m_snapshots_directory(std::move(snapshots_directory)),
	  m_archive_directory(std::move(archive_directory)) {}
} // namespace btrsnap

#pragma endregion

#pragma region Recycler

namespace btrsnap {
auto Recycler::recycle_snapshots(SubvolumeStorage const& storage, Config const& config) const -> Report {
	auto const get_excess_live_snapshots = [&] {
		return get_excess_snapshots(get_sorted_snapshots_in(storage.get_snapshots_directory()), config.snapshot_limit);
	};

	auto excess = get_excess_live_snapshots();
	if (excess.empty()) {
		m_log.info("No snapshots to recycle");
		return {};
	}

	auto ret = Report{};
	ret.archived = populate_archive(storage, excess, config.archive_period);
	if (!ret.archived.empty()) {
		excess = get_excess_live_snapshots();
	}

	ret.deleted = delete_snapshots(std::move(excess));

	excess = get_excess_snapshots(get_sorted_snapshots_in(storage.get_archive_directory()), config.archive_limit);
	ret.deleted.append_range(delete_snapshots(std::move(excess)));

	return ret;
}

auto Recycler::get_sorted_snapshots_in(fs::path const& path) const -> std::vector<Snapshot> {
	auto ret = util::list_snapshots(*m_btrfs, path);
	// move youngest to back.
	std::ranges::sort(ret, [](Snapshot const& a, Snapshot const& b) {
		return a.timestamp < b.timestamp;
	});
	return ret;
}

auto Recycler::get_excess_snapshots(std::vector<Snapshot> sorted, int const keep) -> std::vector<Snapshot> {
	auto const excess_count = static_cast<int>(sorted.size()) - keep;
	if (excess_count <= 0) {
		return {};
	}

	// pop snapshots to keep.
	sorted.resize(std::size_t(excess_count));
	return sorted;
}

auto Recycler::delete_snapshots(std::vector<Snapshot> snapshots) const -> std::vector<Result<Snapshot>> {
	if (snapshots.empty()) {
		return {};
	}

	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& snapshot : snapshots) {
		auto const result = m_btrfs->delete_subvolume(snapshot.path.generic_string());
		if (!result) {
			m_log.warn("Failed to delete snapshot: {}", result.error().message);
		} else {
			m_log.info("Snapshot deleted: {}", snapshot.path.generic_string());
		}
		ret.push_back(result.transform([&] {
			return std::move(snapshot);
		}));
	}
	return ret;
}

auto Recycler::archive_snapshot(Snapshot const& source, fs::path const& parent_directory) const -> Result<Snapshot> {
	auto ret = util::copy_snapshot(*m_btrfs, source, parent_directory);
	if (!ret) {
		m_log.warn("Failed to archive snapshot: {}, {}", source.path.generic_string(), ret.error().message);
	} else {
		m_log.info("Snapshot archived: {}", ret->path.generic_string());
	}
	return ret;
}

auto Recycler::populate_archive(SubvolumeStorage const& storage,
								std::span<Snapshot const> excess,
								std::chrono::days const period) const -> std::vector<Snapshot> {
	if (excess.empty()) {
		return {};
	}

	auto latest_timestamp = [&] -> std::optional<Timestamp> {
		auto const existing = get_sorted_snapshots_in(storage.get_archive_directory());
		if (!existing.empty()) {
			return existing.back().timestamp;
		}
		return {};
	}();

	auto result = util::ensure_directory(storage.get_archive_directory());
	if (!result) {
		m_log.warn("Failed to create archive directory: {}", result.error().message);
		return {};
	}

	auto ret = std::vector<Snapshot>{};
	for (auto const& src : excess) {
		auto const delta_time = [&] -> std::optional<Timestamp> {
			if (latest_timestamp) {
				return src.timestamp - *latest_timestamp;
			}
			return {};
		}();
		if (delta_time && *delta_time < period) {
			continue;
		}

		if (auto result = archive_snapshot(src, storage.get_archive_directory())) {
			latest_timestamp = src.timestamp;
			ret.push_back(std::move(*result));
		}
	}
	return ret;
}
} // namespace btrsnap

#pragma endregion

#pragma region Subvolume

namespace btrsnap {
namespace {
constexpr void clamp_limit(int& out) { out = std::max(out, 0); }
constexpr void clamp_period(std::chrono::days& out) { out = std::max(std::chrono::days{1}, out); }

struct Printer {
	void print(std::span<SnapshotList::Entry const> entries) const {
		auto table =
			klib::TextTable::Builder{}.add_column(" #").add_column(std::string{subvolume}).add_column("Age").build();

		for (auto const [index, entry] : std::views::enumerate(entries)) {
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
	RecycleConfig const& recycle;

	Timestamp now{current_timestamp()};
};
} // namespace

auto Subvolume::create(gsl::not_null<IBtrfs const*> btrfs, StorageConfig const& storage_config, Config config)
	-> Result<Subvolume> {
	clamp_limit(config.recycle.archive_limit);
	clamp_period(config.recycle.archive_period);
	clamp_limit(config.recycle.archive_limit);

	return btrfs->is_subvolume(config.path.generic_string())
		.and_then([&] {
			return Storage::create(storage_config, config.name);
		})
		.transform([&](Storage storage) {
			return Subvolume(btrfs, std::move(storage), std::move(config));
		});
}

Subvolume::Subvolume(gsl::not_null<IBtrfs const*> btrfs, Storage storage, Config config)
	: m_btrfs(btrfs),
	  m_storage(std::move(storage)),
	  m_config(std::move(config)) {}

auto Subvolume::get_live_snapshots() const -> std::vector<Snapshot> {
	return Recycler{m_btrfs}.get_sorted_snapshots_in(get_storage().get_snapshots_directory());
}

auto Subvolume::get_archived_snapshots() const -> std::vector<Snapshot> {
	return Recycler{m_btrfs}.get_sorted_snapshots_in(get_storage().get_archive_directory());
}

auto Subvolume::take_snapshot(Timestamp const timestamp) -> Result<Snapshot> {
	auto subdirectory = get_storage().get_snapshots_directory() / to_pathname(timestamp);
	auto ret = save_snapshot_to(subdirectory).transform([&] {
		return Snapshot{.path = std::move(subdirectory), .timestamp = timestamp};
	});
	on_action(ret, "save");
	return ret;
}

auto Subvolume::recycle_snapshots() -> RecycleReport {
	return Recycler{m_btrfs}.recycle_snapshots(m_storage, m_config.recycle);
}

auto Subvolume::clear_all_snapshots() -> std::vector<Result<Snapshot>> {
	auto snapshots = get_live_snapshots();
	snapshots.append_range(get_archived_snapshots());
	return Recycler{m_btrfs}.delete_snapshots(std::move(snapshots));
}

void Subvolume::print_snapshots(std::ostream& out, Timestamp const now) const {
	auto const printer = Printer{.out = out, .subvolume = m_config.name, .recycle = m_config.recycle, .now = now};
	printer.print(SnapshotList{*this}.get_entries());
}

auto Subvolume::save_snapshot_to(fs::path const& destination) const -> Result<void> {
	if (m_config.readonly_snapshots) {
		return m_btrfs->create_readonly_snapshot(m_config.path.generic_string(), destination.generic_string());
	}
	return m_btrfs->create_snapshot(m_config.path.generic_string(), destination.generic_string());
}

void Subvolume::on_action(Result<Snapshot> const& result, std::string_view const action) const {
	if (!result) {
		m_log.error("Failed to {} snapshot: {}", action, result.error().message);
	} else {
		m_log.info("Snapshot {}d: {}", action, result->path.generic_string());
	}
}

SnapshotList::SnapshotList(Subvolume const& subvolume) : m_btrfs(&subvolume.get_btrfs()) {
	auto const transfer = [this](std::span<Snapshot> snapshots, bool const is_archived) {
		for (auto& snapshot : snapshots) {
			m_entries.push_back(Entry{.snapshot = std::move(snapshot), .is_archived = is_archived});
		}
	};

	auto const& storage = subvolume.get_storage();
	auto snapshots = util::list_snapshots(*m_btrfs, storage.get_snapshots_directory());
	transfer(snapshots, false);
	snapshots = util::list_snapshots(*m_btrfs, storage.get_archive_directory());
	transfer(snapshots, true);

	std::ranges::sort(m_entries, [](Entry const& a, Entry const& b) {
		return a.snapshot.timestamp < b.snapshot.timestamp;
	});

	auto number = 1;
	for (auto& entry : m_entries) {
		entry.number = SnapshotNumber{number++};
	}
}

auto SnapshotList::find_entry(Number const number) const -> klib::Ptr<Entry const> {
	auto const it = std::ranges::find_if(m_entries, [number](Entry const& entry) {
		return entry.number == number;
	});
	if (it == m_entries.end()) {
		return nullptr;
	}
	return &*it;
}

auto SnapshotList::find_snapshot(Number const number) const -> klib::Ptr<Snapshot const> {
	auto const entry = find_entry(number);
	if (!entry) {
		return nullptr;
	}
	return &entry->snapshot;
}

auto SnapshotList::delete_snapshot(Number const number) -> Result<Snapshot> {
	auto const it = std::ranges::find_if(m_entries, [number](Entry const& e) {
		return e.number == number;
	});
	if (it == m_entries.end()) {
		return to_error(Error::Type::InvalidArgument,
						std::format("Invalid Snapshot Number: {}", std::to_underlying(number)));
	}

	auto result = m_btrfs->delete_subvolume(it->snapshot.path.generic_string());
	if (!result) {
		return std::unexpected(std::move(result.error()));
	}

	auto ret = std::move(it->snapshot);
	m_entries.erase(it);
	return ret;
}

auto SubvolumeList::take_snapshots(Timestamp const timestamp) -> std::vector<Result<Snapshot>> {
	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& subvolume : subvolumes) {
		ret.push_back(subvolume.take_snapshot(timestamp));
	}
	return ret;
}

auto SubvolumeList::recycle_snapshots() -> RecycleReport {
	auto ret = RecycleReport{};
	for (auto& subvolume : subvolumes) {
		ret.append(subvolume.recycle_snapshots());
	}
	return ret;
}

auto SubvolumeList::clear_all_snapshots() -> std::vector<Result<Snapshot>> {
	auto ret = std::vector<Result<Snapshot>>{};
	for (auto& subvolume : subvolumes) {
		ret.append_range(subvolume.clear_all_snapshots());
	}
	return ret;
}

void SubvolumeList::print_snapshots(std::ostream& out, Timestamp const now) const {
	for (auto const& subvolume : subvolumes) {
		subvolume.print_snapshots(out, now);
	}
}

auto SubvolumeList::find_by_name(std::string_view const name) const -> klib::Ptr<Subvolume const> {
	auto const it = std::ranges::find_if(subvolumes, [name](Subvolume const& subvolume) {
		return subvolume.get_config().name == name;
	});
	if (it == subvolumes.end()) {
		return nullptr;
	}
	return &*it;
}

auto SubvolumeList::find_by_path(fs::path const& path) const -> klib::Ptr<Subvolume const> {
	auto const it = std::ranges::find_if(subvolumes, [&path](Subvolume const& subvolume) {
		return subvolume.get_config().path == path;
	});
	if (it == subvolumes.end()) {
		return nullptr;
	}
	return &*it;
}
} // namespace btrsnap

#pragma endregion
