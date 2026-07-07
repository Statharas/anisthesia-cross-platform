#include <anisthesia/platform.hpp>

#ifdef _WIN32
#include <anisthesia/win_platform.hpp>
#elif defined(ANISTHESIA_HAS_LINUX_MPRIS)
#include <anisthesia/linux_mpris.hpp>
#endif

namespace anisthesia {

bool GetResults(const std::vector<Player>& players, media_proc_t media_proc,
                std::vector<Result>& results) {
#ifdef _WIN32
  std::vector<win::Result> win_results;
  if (!win::GetResults(players, std::move(media_proc), win_results)) {
    return false;
  }

  results.clear();
  results.reserve(win_results.size());
  for (auto& result : win_results) {
    results.push_back({
        .player = std::move(result.player),
        .media = std::move(result.media),
    });
  }
  return true;
#elif defined(ANISTHESIA_HAS_LINUX_MPRIS)
  return linux::mpris::GetResults(players, std::move(media_proc), results);
#else
  (void)players;
  (void)media_proc;
  results.clear();
  return false;
#endif
}

}  // namespace anisthesia
