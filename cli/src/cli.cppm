export module btrsnap.cli;

export import btrsnap;
export import clap;

export namespace btrsnap::cli {
enum struct ExitCode : std::int8_t {
	Success = 0,
	Failure = 1,
};

class ICommand : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_identifier() const -> std::string_view = 0;
	[[nodiscard]] virtual auto get_help_text() const -> std::string_view { return {}; }
	virtual auto get_parameter_list() -> clap::ParameterList { return {}; }
	virtual auto execute(SubvolumeList& subvolume_list) -> ExitCode = 0;
};

class Runner {
  public:
	[[nodiscard]] auto parse_args(clap::OptionList options, int argc, char const* const* argv)
		-> std::optional<ExitCode>;
	[[nodiscard]] auto execute(SubvolumeList& subvolume_list) -> ExitCode;

	std::vector<std::unique_ptr<ICommand>> commands{};

  private:
	std::string_view m_command_identifier{};
};

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
