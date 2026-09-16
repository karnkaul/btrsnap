#pragma once
#include "btrsnap/result.hpp"
#include "btrsnap/snapshot.hpp"
#include <vector>

namespace btrsnap {
struct RecycleReport {
	void append(RecycleReport other) {
		archived.append_range(std::move(other.archived));
		deleted.append_range(std::move(other.deleted));
	}

	std::vector<Snapshot> archived{};
	std::vector<Result<Snapshot>> deleted{};
};
} // namespace btrsnap
