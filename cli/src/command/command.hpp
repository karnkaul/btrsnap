#pragma once
#include "btrsnap/list.hpp"
#include "clap/parameter.hpp"
#include "command/exit_code.hpp"
#include "klib/base_types.hpp"

namespace btrsnap::cli {
class ICommand : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_help_text() const -> std::string_view { return {}; }
	virtual auto execute(SubvolumeList& subvolume_list) -> ExitCode = 0;
};

class ISubcommand : public ICommand {
  public:
	[[nodiscard]] virtual auto get_identifier() const -> std::string_view = 0;
	virtual auto get_parameter_list() -> clap::ParameterList { return {}; }
};

class IRootCommand : public ICommand {
  public:
	virtual auto get_option_list() -> clap::OptionList { return {}; }
};
} // namespace btrsnap::cli
