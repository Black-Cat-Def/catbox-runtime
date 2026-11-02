#include "cli.hpp"
#include <iostream>
#include <string_view>
#include <vector>

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv, argv + argc);

    if (args.size() < 2 || args[1] == "--help" || args[1] == "-h") {
        catbox::print_help();
        return 0;
    }

    if (args[1] == "--version" || args[1] == "-v") {
        catbox::print_version();
        return 0;
    }

    if (args[1] == "run") {
        if (!catbox::has_root_privileges()) {
            std::cerr << "Ошибка: Для запуска рантайма требуются права суперпользователя (root).\n"
                      << "Пожалуйста, используйте: sudo catbox ...\n";
            return 1;
        }

        auto config = catbox::parse_run_args(args);
        if (!config.has_value()) {
            return 1;
        }

        std::cout << "[catbox] Конфигурация контейнера сформирована:\n"
                  << "  - Rootfs:   " << config->rootfs_path << "\n"
                  << "  - Hostname: " << config->hostname << "\n"
                  << "  - Команда:  ";
        for (const auto& part : config->command) {
            std::cout << part << " ";
        }
        std::cout << "\n";

        return 0;
    }

    std::cerr << "Ошибка: неизвестная команда '" << args[1] << "'. Используйте --help для справки.\n";
    return 1;
}
