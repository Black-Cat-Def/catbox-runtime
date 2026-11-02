#pragma once

#include "container.hpp"
#include <optional>
#include <string_view>
#include <vector>

namespace catbox {

void print_help();
void print_version();
bool has_root_privileges();
std::optional<ContainerConfig> parse_run_args(const std::vector<std::string_view>& args);

} // namespace catbox
