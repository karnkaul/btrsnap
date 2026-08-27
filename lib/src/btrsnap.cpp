#include "btrsnap/btrsnap.hpp"
#include "detail/instance.hpp"

namespace btrsnap {
namespace {
template <typename F>
void on_instance(F func) {
	auto instance = detail::Instance::create();
	if (!instance) { return; }
	func(*instance);
}
} // namespace
} // namespace btrsnap

void btrsnap::list_snapshots() {
	on_instance([](detail::Instance& instance) { instance.list_snapshots(); });
}

auto btrsnap::take_snapshots(bool const no_trim) -> bool {
	auto flags = detail::Flag{};
	if (no_trim) { flags |= detail::Flag::NoTrim; }

	auto ret = false;
	on_instance([&](detail::Instance& instance) { ret = instance.take_snapshots(flags); });
	return ret;
}

void btrsnap::trim_snapshots() {
	on_instance([](detail::Instance& instance) { instance.trim_snapshots(); });
}

auto btrsnap::clear_snapshots() -> bool {
	auto ret = false;
	on_instance([&](detail::Instance& instance) { ret = instance.clear_snapshots(); });
	return ret;
}

void btrsnap::print_config_for(std::string_view const subvolume) {
	auto const config = detail::Config{.subvolume = std::string{subvolume}};
	config.print();
}
