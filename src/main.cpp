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

    if (!catbox::has_root_privileges()) {
        std::cerr << "Ошибка: Для запуска рантайма требуются права суперпользователя (root).\n"
                  << "Пожалуйста, используйте: sudo catbox ...\n";
        return 1;
    }

    std::cout << "[catbox] Инициализация рантайма (права root подтверждены)...\n";
    return 0;
}
