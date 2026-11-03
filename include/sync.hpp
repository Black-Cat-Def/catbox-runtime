#pragma once

namespace catbox {

class SyncPipe {
public:
    SyncPipe();
    ~SyncPipe();

    SyncPipe(const SyncPipe&) = delete;
    SyncPipe& operator=(const SyncPipe&) = delete;
    SyncPipe(SyncPipe&& other) noexcept;
    SyncPipe& operator=(SyncPipe&& other) noexcept;

    void wait_proceed();
    void notify_proceed();
    void close_read();
    void close_write();

private:
    int fds_[2]{-1, -1};
};

} // namespace catbox
