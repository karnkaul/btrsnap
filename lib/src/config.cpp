#include "btrsnap/config.hpp"
#include "confy/reader.hpp"
#include "confy/writer.hpp"
#include "klib/file_io.hpp"
#include <filesystem>
#include <format>
#include <iostream>

namespace btrsnap {
namespace fs = std::filesystem;

namespace key {
namespace {
constexpr std::string_view subvolume_v{"SUBVOLUME"};
constexpr std::string_view snapshots_subdirectory_v{"SNAPSHOTS_SUBDIRECTORY"};
constexpr std::string_view snapshot_limit_v{"SNAPSHOT_LIMIT"};
constexpr std::string_view archive_subdirectory_v{"ARCHIVE_SUBDIRECTORY"};
constexpr std::string_view archive_period_v{"ARCHIVE_PERIOD_DAYS"};
constexpr std::string_view archive_limit_v{"ARCHIVE_LIMIT"};
} // namespace
} // namespace key

auto Config::from_file(klib::CString const path) -> std::optional<Config> {
	auto reader = confy::Reader{};
	if (!reader.read_file(path.c_str())) { return {}; }

	auto ret = Config{};
	if (!reader.assign_if(ret.subvolume, key::subvolume_v)) { return {}; }

	reader.assign_if(ret.snapshots_subdirectory, key::snapshots_subdirectory_v);
	reader.assign_if(ret.snapshot_limit, key::snapshot_limit_v);
	reader.assign_if(ret.archive_subdirectory, key::archive_subdirectory_v);
	if (auto period = 0; reader.assign_if(period, key::archive_period_v)) { ret.archive_period = std::chrono::days{period}; }
	reader.assign_if(ret.archive_limit, key::archive_limit_v);

	return ret;
}

auto Config::from_directory(std::string_view const directory) -> std::vector<Config> {
	if (!fs::is_directory(directory)) { return {}; }

	auto ret = std::vector<Config>{};
	for (auto const& it : fs::directory_iterator{directory}) {
		if (it.is_directory()) { continue; }

		auto path = it.path();
		if (it.is_symlink()) { path = klib::resolve_symlink(path.generic_string()); }

		auto config = from_file(path.generic_string());
		if (!config) { continue; }

		ret.push_back(std::move(*config));
	}
	return ret;
}

void Config::print(std::string_view const save_directory) const {
	if (subvolume.empty()) { return; }

	auto writer = confy::Writer{};
	if (!save_directory.empty()) { writer.text = std::format("## Save to {}/<filename>.conf\n", save_directory); }

	writer.write_uncommented(key::subvolume_v, subvolume, "Path to subvolume");
	writer.write_commented(key::snapshots_subdirectory_v, snapshots_subdirectory, "Subdirectory to save snapshots to");
	writer.write_commented(key::snapshot_limit_v, snapshot_limit, "Maximum number of snapshots to keep");
	writer.write_commented(key::archive_subdirectory_v, archive_subdirectory, "Subdirectory to store archived snapshots in");
	writer.write_commented(key::archive_period_v, archive_period.count(), "Minimum duration between archived snapshots (in days)");
	writer.write_commented(key::archive_limit_v, archive_limit, "Maximum number of archived snapshots to keep");

	writer.print_to(std::cout);
}
} // namespace btrsnap
