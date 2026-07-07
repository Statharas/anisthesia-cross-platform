#pragma once

#include <anisthesia/platform.hpp>

#include <string>
#include <vector>

namespace anisthesia::linux::process {

bool IsVideoPath(const std::string& path);
std::vector<MediaInfo> MediaFromCommandLine(const std::vector<std::string>& arguments);

bool GetResults(const std::vector<Player>& players, media_proc_t media_proc,
                std::vector<Result>& results);

}  // namespace anisthesia::linux::process
