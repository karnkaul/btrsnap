#include "detail/snapshot_info.hpp"

namespace btrsnap::detail {
auto SnapshotInfo::create(fs::path path) -> SnapshotInfo {
	auto const timestamp = to_timestamp(path.filename().string());
	return SnapshotInfo{
		.path = std::move(path),
		.timestamp = timestamp,
	};
}
} // namespace btrsnap::detail
