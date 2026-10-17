#pragma once
#include "command/command.hpp"
#include <memory>
#include <vector>

namespace btrsnap::cli {
class Runner {
  public:
	[[nodiscard]] auto parse_args(clap::OptionList options, int argc, char const* const* argv) -> std::optional<ExitCode>;
	[[nodiscard]] auto execute(SubvolumeList& subvolume_list) -> ExitCode;

	std::vector<std::unique_ptr<ICommand>> commands{};

  private:
	std::string_view m_command_identifier{};
};
} // namespace btrsnap::cli
