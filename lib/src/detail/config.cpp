#include "detail/config.hpp"
#include "confy/reader.hpp"
#include "confy/writer.hpp"
#include "klib/file_io.hpp"
#include <filesystem>
#include <format>
#include <iostream>

namespace btrsnap::detail {
namespace fs = std::filesystem;

namespace key {
namespace {
constexpr std::string_view subvolume_v{"SUBVOLUME"};
constexpr std::string_view subdirectory_v{"SUBDIRECTORY"};
constexpr std::string_view limit_v{"LIMIT"};
} // namespace
} // namespace key

auto Config::from_file(klib::CString const path) -> std::optional<Config> {
	auto reader = confy::Reader{};
	if (!reader.read_file(path.c_str())) { return {}; }

	auto ret = Config{};
	if (!reader.assign_if(ret.subvolume, key::subvolume_v)) { return {}; }

	reader.assign_if(ret.subdirectory, key::subdirectory_v);
	reader.assign_if(ret.limit, key::limit_v);
	return ret;
}

auto Config::from_directory(std::string_view const directory) -> std::vector<Config> {
	if (!fs::is_directory(directory)) { return {}; }

	auto ret = std::vector<Config>{};
	for (auto const& it : fs::directory_iterator{directory}) {
		if (it.is_directory()) { continue; }

		auto path = it.path();
		if (it.is_symlink()) { path = klib::resolve_symlink(path.string()); }

		auto config = from_file(path.string());
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
	writer.write_commented(key::subdirectory_v, subdirectory, "Subdirectory to save snapshots to");
	writer.write_commented(key::limit_v, limit, "Maximum number of snapshots to keep");

	writer.print_to(std::cout);
}
} // namespace btrsnap::detail
