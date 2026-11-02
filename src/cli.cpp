#include "cli.hpp"
#include <iostream>
#include <unistd.h>

namespace catbox {

void print_help() {
    std::cout << "catbox — легковесный контейнерный рантайм на C++17 для Linux\n\n"
              << "Использование:\n"
              << "  catbox run --rootfs <PATH> [--hostname <NAME>] <COMMAND> [ARGS...]   Запуск процесса в контейнере\n"
              << "  catbox --help, -h                                                    Показать справку\n"
              << "  catbox --version, -v                                                 Показать версию\n";
}

void print_version() {
    std::cout << "catbox version 0.1.0 (C++17 Linux Container Runtime)\n";
}

bool has_root_privileges() {
    return ::geteuid() == 0;
}

std::optional<ContainerConfig> parse_run_args(const std::vector<std::string_view>& args) {
    ContainerConfig config;
    size_t i = 2;

    while (i < args.size()) {
        if (args[i] == "--rootfs") {
            if (i + 1 >= args.size()) {
                std::cerr << "Ошибка: флаг --rootfs требует указания пути.\n";
                return std::nullopt;
            }
            config.rootfs_path = std::string(args[i + 1]);
            i += 2;
        } else if (args[i] == "--hostname") {
            if (i + 1 >= args.size()) {
                std::cerr << "Ошибка: флаг --hostname требует указания имени хоста.\n";
                return std::nullopt;
            }
            config.hostname = std::string(args[i + 1]);
            i += 2;
        } else if (!args[i].empty() && args[i][0] == '-') {
            std::cerr << "Ошибка: неизвестный флаг '" << args[i] << "'.\n";
            return std::nullopt;
        } else {
            break;
        }
    }

    while (i < args.size()) {
        config.command.emplace_back(args[i]);
        ++i;
    }

    if (config.rootfs_path.empty()) {
        std::cerr << "Ошибка: не указан обязательный параметр --rootfs <PATH>.\n";
        return std::nullopt;
    }

    if (config.command.empty()) {
        std::cerr << "Ошибка: не указана команда для запуска внутри контейнера.\n";
        return std::nullopt;
    }

    return config;
}

} // namespace catbox
