#include "isolation.hpp"
#include "sync.hpp"
#include <iostream>
#include <sched.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdexcept>
#include <cstdint>

namespace catbox {

namespace {

constexpr size_t STACK_SIZE = 1024 * 1024 * 4;

struct ChildContext {
    const ContainerConfig& config;
    SyncPipe& sync_pipe;
};

int child_entrypoint(void* arg) {
    auto* ctx = static_cast<ChildContext*>(arg);

    ctx->sync_pipe.close_write();
    ctx->sync_pipe.wait_proceed();

    std::cout << "[catbox] Дочерний процесс запущен в изолированных пространствах имён (PID=" << ::getpid() << ")\n";

    return 0;
}

} // namespace

int spawn_container_process(const ContainerConfig& config) {
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

    ChildContext ctx{config, sync_pipe};
    void* stack_top = static_cast<char*>(stack) + STACK_SIZE;

    int clone_flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | SIGCHLD;
    pid_t child_pid = ::clone(child_entrypoint, stack_top, clone_flags, &ctx);

    if (child_pid < 0) {
        ::munmap(stack, STACK_SIZE);
        throw std::runtime_error("Системный вызов clone завершился ошибкой");
    }

    std::cout << "[catbox] Процесс контейнера успешно порожден (хостовый PID=" << child_pid << ")\n";

    sync_pipe.close_read();
    sync_pipe.notify_proceed();

    int status = 0;
    if (::waitpid(child_pid, &status, 0) < 0) {
        ::munmap(stack, STACK_SIZE);
        throw std::runtime_error("Ошибка при ожидании дочернего процесса");
    }

    ::munmap(stack, STACK_SIZE);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return 1;
}

} // namespace catbox
