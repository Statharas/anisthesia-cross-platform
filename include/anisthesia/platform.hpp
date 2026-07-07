#pragma once

#include <vector>

#include <anisthesia/media.hpp>
#include <anisthesia/player.hpp>

namespace anisthesia {

struct Result {
  Player player;
  std::vector<Media> media;
};

bool GetResults(const std::vector<Player>& players, media_proc_t media_proc,
                std::vector<Result>& results);

}  // namespace anisthesia
