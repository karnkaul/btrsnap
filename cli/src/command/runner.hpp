#pragma once
#include "command/command.hpp"
#include <memory>
#include <vector>

namespace btrsnap::cli {
class Runner {
  public:
	[[nodiscard]] auto parse_args(int argc, char const* const* argv) -> std::optional<ExitCode>;
	[[nodiscard]] auto execute(SubvolumeList& subvolume_list) -> ExitCode;

	std::unique_ptr<IRootCommand> root_command{};
	std::vector<std::unique_ptr<ISubcommand>> subcommands{};

  private:
	[[nodiscard]] auto get_command(std::string_view identifier) const -> ICommand&;

	std::string_view m_command_identifier{};
};
} // namespace btrsnap::cli
