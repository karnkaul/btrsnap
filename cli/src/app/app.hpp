#pragma once
#include "btrsnap/instance.hpp"
#include "command/runner.hpp"
#include "klib/ptr.hpp"

namespace btrsnap::cli {
class App {
  public:
	[[nodiscard]] auto run(int argc, char const* const* argv) -> int;

  private:
	[[nodiscard]] auto load_config() -> bool;
	[[nodiscard]] auto setup_instance() -> bool;
	[[nodiscard]] auto load_subvolumes() -> bool;

	Runner m_runner{};
	std::string_view m_custom_config_path{};

	InstanceConfig m_instance_config{};
	std::optional<Instance> m_instance{};
	SubvolumeList m_subvolume_list{};
};
} // namespace btrsnap::cli
