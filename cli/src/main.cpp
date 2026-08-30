#include "btrsnap/btrsnap.hpp"
#include "btrsnap/build_version.hpp"
#include "clap/parser.hpp"
#include <print>

namespace btrsnap::cli {
namespace {
class App {
  public:
	auto run(int const argc, char const* const* argv) -> int {
		auto const parse_result = parse_args(argc, argv);
		if (parse_result.should_early_exit()) { return parse_result.return_code(); }

		if (m_generate) {
			print_config_for(m_generate_for_subvolume);
			return EXIT_SUCCESS;
		}

		if (m_list) {
			list_snapshots(m_custom_config_path);
			return EXIT_SUCCESS;
		}

		if (m_only_trim) {
			trim_snapshots(m_custom_config_path);
			return EXIT_SUCCESS;
		}

		if (m_clear) {
			if (!clear_snapshots(m_custom_config_path)) { return EXIT_FAILURE; }
			return EXIT_SUCCESS;
		}

		if (!take_snapshots(m_no_trim, m_custom_config_path)) { return EXIT_FAILURE; }

		return EXIT_SUCCESS;
	}

  private:
	[[nodiscard]] auto parse_args(int const argc, char const* const* argv) -> clap::Result {
		static auto const version_str = std::format("{}", build_version_v);
		auto spec = clap::spec::Parameters{
			.parameters =
				{
					clap::named_option(m_generate_for_subvolume, "g,generate", "generate config for SUBVOLUME", &m_generate),
					clap::named_option(m_custom_config_path, "c,config", "path to custom config"),
					clap::named_flag(m_list, "l,list", "list snapshots"),
					clap::named_flag(m_no_trim, "n,no-trim", "skip trimming snapshots"),
					clap::named_flag(m_only_trim, "t,trim", "only trim existing snapshots"),
					clap::named_flag(m_clear, "clear", "clear ALL saved snapshots"),
				},
			.program =
				clap::Program{
					.name = "btrsnap",
					.version = version_str,
				},
		};
		auto parser = clap::Parser{std::move(spec)};
		return parser.parse_main(argc, argv);
	}

	std::string_view m_generate_for_subvolume{};
	std::string m_custom_config_path{};
	bool m_generate{};
	bool m_list{};
	bool m_no_trim{};
	bool m_only_trim{};
	bool m_clear{};
};
} // namespace
} // namespace btrsnap::cli

int main(int argc, char** argv) {
	try {
		return btrsnap::cli::App{}.run(argc, argv);
	} catch (std::exception const& e) {
		std::println(stderr, "PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println(stderr, "PANIC!");
		return EXIT_FAILURE;
	}
}
