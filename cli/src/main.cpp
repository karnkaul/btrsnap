#include "app/app.hpp"
#include <print>

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
