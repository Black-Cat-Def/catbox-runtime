#include <iostream>
#include <string_view>
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        catbox::print_help();
        return 0;
    }

    std::string_view command = argv[1];

    if (command == "--help" || command == "-h") {
        catbox::print_help();
        return 0;
    }

    if (command == "--version" || command == "-v") {
        catbox::print_version();
        return 0;
    }

    if (!catbox::has_root_privileges()) {
        std::cerr << "Ошибка: Для запуска рантайма требуются права суперпользователя (root).\n"
                  << "Пожалуйста, используйте: sudo catbox ...\n";
        return 1;
    }

    std::cout << "[catbox] Инициализация рантайма (права root подтверждены)...\n";
    return 0;
}
