#include "btrsnap/util.hpp"
#include "detail/to_error.hpp"
#include "djson/json.hpp"

namespace btrsnap {
namespace {
void to_path(dj::Json const& json, fs::path& out) {
	auto str = std::string{};
	from_json(json, str);
	out = str;
}

void to_days(dj::Json const& json, std::chrono::days& out) {
	auto days = int{};
	from_json(json, days);
	out = std::chrono::days{days};
}
} // namespace

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

auto util::copy_snapshot(IBtrfs const& btrfs, Snapshot const& source, fs::path const& dst_dir) -> Result<Snapshot> {
	return ensure_directory(dst_dir).and_then([&] -> Result<Snapshot> {
		auto const filename = source.path.filename();
		auto destination = dst_dir / filename;
		if (fs::exists(destination)) {
			auto message = std::format("Destination already exists: {}", destination.generic_string());
			return detail::to_error(Error::Type::InvalidArgument, message);
		}
		return btrfs.create_snapshot(source.path.generic_string(), destination.generic_string()).transform([&] {
			return Snapshot{.path = std::move(destination), .timestamp = source.timestamp};
		});
	});
}

auto util::is_non_empty(fs::path const& path) -> Result<void> {
	if (path.empty()) { return detail::to_error(Error::Type::InvalidArgument, "Empty path"); }
	return {};
}

auto util::is_directory(fs::path const& path) -> Result<void> {
	if (auto result = is_non_empty(path); !result) { return std::unexpected{std::move(result.error())}; }

	auto err = std::error_code{};

	if (!fs::exists(path, err)) { return detail::filesystem_error(std::format("Nonexistent path: {}", path.generic_string())); }
	if (!fs::is_directory(path, err)) { return detail::filesystem_error(std::format("Existing path is not a directory: {}", path.generic_string())); }

	return {};
}

auto util::ensure_directory(fs::path const& path) -> Result<void> {
	if (auto result = is_non_empty(path); !result) { return std::unexpected{std::move(result.error())}; }

	auto err = std::error_code{};

	if (!fs::exists(path)) {
		if (!fs::create_directories(path, err)) { return detail::filesystem_error(std::format("Failed to create directory: {}", path.generic_string())); }
		return {};
	}

	return util::is_directory(path);
}

void util::from_json(dj::Json const& json, StorageInfo& out) { to_path(json, out.root); }

void util::from_json(dj::Json const& json, RecycleInfo& out) {
	if (auto const& snapshot_limit = json["snapshot_limit"]) { from_json(snapshot_limit, out.snapshot_limit); }
	if (auto const& archive_period_days = json["archive_period_days"]) { to_days(archive_period_days, out.archive_period); }
	if (auto const& archive_limit = json["archive_limit"]) { from_json(archive_limit, out.archive_limit); }
}

void util::from_json(dj::Json const& json, SubvolumeInfo& out) {
	from_json(json["name"], out.name);
	to_path(json["path"], out.path);
	from_json(json["recycle"], out.recycle);
}

void util::from_json(dj::Json const& json, InstanceInfo& out) {
	if (auto const& storage = json["storage"]) { from_json(storage, out.storage); }
	for (auto const& subvolume : json["subvolumes"].as_array()) { from_json(subvolume, out.subvolumes.emplace_back()); }
}
} // namespace btrsnap
