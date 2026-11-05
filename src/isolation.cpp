#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "isolation.hpp"
#include "sync.hpp"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef SYS_pivot_root
#if defined(__NR_pivot_root)
#define SYS_pivot_root __NR_pivot_root
#else
#define SYS_pivot_root 217
#endif
#endif

namespace catbox {

namespace {

constexpr size_t STACK_SIZE = 1024 * 1024 * 4;

struct ChildContext {
    const ContainerConfig& config;
    SyncPipe& sync_pipe;
};

void setup_uts_namespace(const std::string& hostname) {
    if (!hostname.empty()) {
        if (::sethostname(hostname.data(), hostname.size()) < 0) {
            throw std::runtime_error("Не удалось установить имя хоста (" + hostname + "): " + std::string(std::strerror(errno)));
        }
    }
}

void setup_mount_namespace(const std::string& rootfs_path_str) {
    std::filesystem::path rootfs_path(rootfs_path_str);

    if (::mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) < 0) {
        throw std::runtime_error("Ошибка настройки MS_PRIVATE для монтирования: " + std::string(std::strerror(errno)));
    }

    if (::mount(rootfs_path.c_str(), rootfs_path.c_str(), nullptr, MS_BIND | MS_REC, nullptr) < 0) {
        throw std::runtime_error("Ошибка bind mount rootfs (" + rootfs_path.string() + "): " + std::string(std::strerror(errno)));
    }

    std::filesystem::path old_root_dir = rootfs_path / ".pivot_old";
    std::error_code ec;
    std::filesystem::create_directories(old_root_dir, ec);
    if (ec) {
        throw std::runtime_error("Не удалось создать директорию " + old_root_dir.string() + ": " + ec.message());
    }

    if (::syscall(SYS_pivot_root, rootfs_path.c_str(), old_root_dir.c_str()) < 0) {
        throw std::runtime_error("Системный вызов pivot_root завершился ошибкой: " + std::string(std::strerror(errno)));
    }

    if (::chdir("/") < 0) {
        throw std::runtime_error("Не удалось перейти в новый корень (chdir /): " + std::string(std::strerror(errno)));
    }

    if (::umount2("/.pivot_old", MNT_DETACH) < 0) {
        throw std::runtime_error("Не удалось отмонтировать старый корень (umount2 /.pivot_old): " + std::string(std::strerror(errno)));
    }

    std::filesystem::remove("/.pivot_old", ec);

    std::filesystem::create_directories("/proc", ec);
    if (::mount("proc", "/proc", "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC, nullptr) < 0) {
        throw std::runtime_error("Не удалось смонтировать /proc: " + std::string(std::strerror(errno)));
    }
}

[[noreturn]] void execute_command(const std::vector<std::string>& command) {
    if (command.empty()) {
        std::cerr << "[catbox] Ошибка: команда для запуска пуста\n";
        ::_exit(1);
    }

    std::vector<char*> argv;
    argv.reserve(command.size() + 1);
    for (const auto& arg : command) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    if (::getenv("PATH") == nullptr) {
        ::setenv("PATH", "/bin:/usr/bin:/sbin:/usr/sbin", 1);
    }

    ::execvp(argv[0], argv.data());

    std::cerr << "[catbox] Ошибка при запуске '" << argv[0] << "': "
              << std::strerror(errno) << "\n";
    ::_exit(127);
}

int child_entrypoint(void* arg) {
    auto* ctx = static_cast<ChildContext*>(arg);

    ctx->sync_pipe.close_write();
    try {
        ctx->sync_pipe.wait_proceed();

        setup_uts_namespace(ctx->config.hostname);
        setup_mount_namespace(ctx->config.rootfs_path);

        execute_command(ctx->config.command);
    } catch (const std::exception& e) {
        std::cerr << "[catbox] Ошибка внутри контейнера: " << e.what() << "\n";
        ::_exit(1);
    }

    return 0;
}

} // namespace

int spawn_container_process(const ContainerConfig& config) {
    std::error_code ec;
    if (!std::filesystem::exists(config.rootfs_path, ec)) {
        throw std::runtime_error("Указанный путь rootfs не существует: " + config.rootfs_path);
    }

    std::filesystem::path canonical_rootfs = std::filesystem::canonical(config.rootfs_path, ec);
    if (ec) {
        throw std::runtime_error("Не удалось разрешить путь rootfs (" + config.rootfs_path + "): " + ec.message());
    }

    if (!std::filesystem::is_directory(canonical_rootfs, ec)) {
        throw std::runtime_error("Указанный rootfs не является директорией: " + canonical_rootfs.string());
    }

    ContainerConfig resolved_config = config;
    resolved_config.rootfs_path = canonical_rootfs.string();

    SyncPipe sync_pipe;

    void* stack = ::mmap(
        nullptr,
        STACK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
        -1,
        0
    );

    if (stack == MAP_FAILED) {
        throw std::runtime_error("Не удалось выделить память под стек процесса");
    }

    ChildContext ctx{resolved_config, sync_pipe};
    void* stack_top = static_cast<char*>(stack) + STACK_SIZE;

    int clone_flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | SIGCHLD;
    pid_t child_pid = ::clone(child_entrypoint, stack_top, clone_flags, &ctx);

    if (child_pid < 0) {
        ::munmap(stack, STACK_SIZE);
        throw std::runtime_error("Системный вызов clone завершился ошибкой: " + std::string(std::strerror(errno)));
    }

    std::cout << "[catbox] Процесс контейнера успешно порожден (хостовый PID=" << child_pid << ")\n";

    sync_pipe.close_read();
    sync_pipe.notify_proceed();

    int status = 0;
    if (::waitpid(child_pid, &status, 0) < 0) {
        ::munmap(stack, STACK_SIZE);
        throw std::runtime_error("Ошибка при ожидании дочернего процесса: " + std::string(std::strerror(errno)));
    }

    ::munmap(stack, STACK_SIZE);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }

    return 1;
}

} // namespace catbox
