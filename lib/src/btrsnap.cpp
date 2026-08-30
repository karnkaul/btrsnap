#include "btrsnap/btrsnap.hpp"
#include "detail/instance.hpp"

namespace btrsnap {
namespace {
template <typename F>
void on_instance(klib::CString const custom_config_path, F func) {
	auto instance = [&] {
		if (custom_config_path.as_view().empty()) { return detail::Instance::create(); }
		return detail::Instance::create(custom_config_path);
	}();
	if (!instance) { return; }
	func(*instance);
}
} // namespace
} // namespace btrsnap

void btrsnap::list_snapshots(klib::CString const custom_config_path) {
	on_instance(custom_config_path, [](detail::Instance& instance) { instance.list_snapshots(); });
}

auto btrsnap::take_snapshots(bool const no_trim, klib::CString const custom_config_path) -> bool {
	auto flags = detail::Flag{};
	if (no_trim) { flags |= detail::Flag::NoTrim; }

	auto ret = false;
	on_instance(custom_config_path, [&](detail::Instance& instance) { ret = instance.take_snapshots(flags); });
	return ret;
}

void btrsnap::trim_snapshots(klib::CString const custom_config_path) {
	on_instance(custom_config_path, [](detail::Instance& instance) { instance.trim_snapshots(); });
}

auto btrsnap::clear_snapshots(klib::CString const custom_config_path) -> bool {
	auto ret = false;
	on_instance(custom_config_path, [&](detail::Instance& instance) { ret = instance.clear_snapshots(); });
	return ret;
}

void btrsnap::print_config_for(std::string_view const subvolume) {
	auto const config = detail::Config{.subvolume = std::string{subvolume}};
	config.print();
}
