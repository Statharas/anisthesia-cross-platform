#pragma once

#include <string_view>
#include <vector>

#include <anisthesia/platform.hpp>

namespace anisthesia::linux::mpris {

MediaState MediaStateFromPlaybackStatus(std::string_view playback_status);

bool GetResults(const std::vector<Player>& players, media_proc_t media_proc,
                std::vector<Result>& results);

}  // namespace anisthesia::linux::mpris
