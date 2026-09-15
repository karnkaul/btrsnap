#pragma once
#include "btrsnap/clock.hpp"
#include <filesystem>

namespace btrsnap {
namespace fs = std::filesystem;

struct Snapshot {
	fs::path path{};
	Timestamp timestamp{};
};
} // namespace btrsnap
