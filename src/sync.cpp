#include "sync.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <utility>

namespace catbox {

SyncPipe::SyncPipe() {
    if (::pipe2(fds_, O_CLOEXEC) < 0) {
        throw std::runtime_error("Не удалось создать канал синхронизации pipe2");
    }
}

SyncPipe::~SyncPipe() {
    close_read();
    close_write();
}

SyncPipe::SyncPipe(SyncPipe&& other) noexcept {
    fds_[0] = other.fds_[0];
    fds_[1] = other.fds_[1];
    other.fds_[0] = -1;
    other.fds_[1] = -1;
}

SyncPipe& SyncPipe::operator=(SyncPipe&& other) noexcept {
    if (this != &other) {
        close_read();
        close_write();
        fds_[0] = other.fds_[0];
        fds_[1] = other.fds_[1];
        other.fds_[0] = -1;
        other.fds_[1] = -1;
    }
    return *this;
}

void SyncPipe::wait_proceed() {
    char buf = 0;
    ssize_t bytes = ::read(fds_[0], &buf, sizeof(buf));
    if (bytes <= 0) {
        throw std::runtime_error("Канал синхронизации разорван до получения сигнала");
    }
}

void SyncPipe::notify_proceed() {
    char buf = 'G';
    ssize_t bytes = ::write(fds_[1], &buf, sizeof(buf));
    if (bytes != 1) {
        throw std::runtime_error("Ошибка отправки сигнала подтверждения в pipe");
    }
}

void SyncPipe::close_read() {
    if (fds_[0] != -1) {
        ::close(fds_[0]);
        fds_[0] = -1;
    }
}

void SyncPipe::close_write() {
    if (fds_[1] != -1) {
        ::close(fds_[1]);
        fds_[1] = -1;
    }
}

} // namespace catbox
