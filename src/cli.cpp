#include "cli.hpp"
#include <iostream>
#include <unistd.h>

namespace catbox {

void print_help() {
    std::cout << "catbox — легковесный контейнерный рантайм на C++17 для Linux\n\n"
              << "Использование:\n"
              << "  catbox run --rootfs <PATH> <COMMAND> [ARGS...]   Запуск процесса в изолированном контейнере\n"
              << "  catbox --help, -h                                Показать справку\n"
              << "  catbox --version, -v                             Показать версию\n";
}

void print_version() {
    std::cout << "catbox version 0.1.0 (C++17 Linux Container Runtime)\n";
}

bool has_root_privileges() {
    return ::geteuid() == 0;
}

} // namespace catbox
