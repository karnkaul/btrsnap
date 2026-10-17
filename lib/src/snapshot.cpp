#include "btrsnap/snapshot.hpp"
#include <algorithm>

namespace btrsnap {
auto Snapshot::List::find_snapshot(Number const number) const -> klib::Ptr<Snapshot const> {
	auto const it = std::ranges::find_if(entries, [number](Entry const& entry) { return entry.number == number; });
	if (it == entries.end()) { return nullptr; }
	return &it->snapshot;
}
} // namespace btrsnap
