#pragma once

#include <string>
#include <vector>

namespace catbox {

struct ContainerConfig {
    std::string rootfs_path;
    std::string hostname = "catbox";
    std::vector<std::string> command;
};

} // namespace catbox
