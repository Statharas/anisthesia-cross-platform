#pragma once

#include <vector>

#include <anisthesia/platform.hpp>

namespace anisthesia::linux::mpris {

bool GetResults(const std::vector<Player>& players, media_proc_t media_proc,
                std::vector<Result>& results);

}  // namespace anisthesia::linux::mpris
