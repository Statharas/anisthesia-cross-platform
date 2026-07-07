#include <anisthesia/linux_process.hpp>

#include <anisthesia/util.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <ranges>
#include <regex>
#include <set>
#include <string_view>

namespace {

namespace fs = std::filesystem;

std::string Basename(const std::string& path) {
  const auto name = fs::path(path).filename().string();
  return name.empty() ? path : name;
}

std::string Lower(std::string value) {
  std::ranges::transform(value, value.begin(),
                         [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

bool PatternMatches(const std::string& pattern, const std::string& value) {
  if (pattern.empty()) return false;

  if (pattern.front() == '^') {
    try {
      return std::regex_match(value, std::regex(pattern, std::regex::ECMAScript | std::regex::icase));
    } catch (const std::regex_error&) {
      return false;
    }
  }

  return anisthesia::detail::util::EqualStrings(pattern, value);
}

bool MatchesExecutable(const anisthesia::Player& player, const std::string& executable) {
  const auto executable_name = Basename(executable);
  return std::ranges::any_of(player.executables, [&](const auto& pattern) {
    return PatternMatches(pattern, executable) || PatternMatches(pattern, executable_name);
  });
}

std::vector<std::string> SplitCommandLine(const std::string& data) {
  std::vector<std::string> arguments;
  std::string current;

  for (const auto c : data) {
    if (c == '\0') {
      if (!current.empty()) arguments.push_back(std::move(current));
      current.clear();
    } else {
      current.push_back(c);
    }
  }

  if (!current.empty()) arguments.push_back(std::move(current));
  return arguments;
}

std::optional<std::vector<std::string>> ReadCommandLine(const fs::path& process_dir) {
  std::ifstream file(process_dir / "cmdline", std::ios::in | std::ios::binary);
  if (!file) {
    return std::nullopt;
  }

  const std::string data{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
  if (data.empty()) return std::nullopt;

  auto arguments = SplitCommandLine(data);
  if (arguments.empty()) return std::nullopt;
  return arguments;
}

bool IsNumericName(const fs::path& path) {
  const auto name = path.filename().string();
  return !name.empty() && std::ranges::all_of(name, [](const unsigned char c) {
    return std::isdigit(c);
  });
}

bool AcceptsMedia(const anisthesia::media_proc_t& media_proc,
                  const anisthesia::MediaInfo& media_info) {
  return !media_proc || media_proc(media_info);
}

}  // namespace

namespace anisthesia::linux::process {

bool IsVideoPath(const std::string& path) {
  const auto extension = Lower(fs::path(path).extension().string());
  static const std::set<std::string> video_extensions = {
      ".avi", ".m2ts", ".m4v", ".mkv", ".mov", ".mp4", ".mpeg",
      ".mpg", ".ogm", ".ogv", ".rmvb", ".ts", ".webm", ".wmv",
  };
  return video_extensions.contains(extension);
}

std::vector<MediaInfo> MediaFromCommandLine(const std::vector<std::string>& arguments) {
  std::vector<MediaInfo> media;

  for (auto it = std::next(arguments.begin()); it != arguments.end(); ++it) {
    if (it->empty() || it->starts_with('-')) continue;
    if (!IsVideoPath(*it)) continue;

    MediaInfo media_info;
    media_info.type = MediaInfoType::File;
    media_info.value = *it;
    media.push_back(std::move(media_info));
  }

  return media;
}

bool GetResults(const std::vector<Player>& players, media_proc_t media_proc,
                std::vector<Result>& results) {
  results.clear();

  std::error_code error;
  for (const auto& entry : fs::directory_iterator("/proc", error)) {
    if (error || !entry.is_directory(error) || !IsNumericName(entry.path())) continue;

    const auto arguments = ReadCommandLine(entry.path());
    if (!arguments || arguments->empty()) continue;

    const auto& executable = arguments->front();
    for (const auto& player : players) {
      if (!MatchesExecutable(player, executable)) continue;

      Media media;
      media.state = MediaState::Playing;

      for (const auto& media_info : MediaFromCommandLine(*arguments)) {
        if (AcceptsMedia(media_proc, media_info)) {
          media.information.push_back(media_info);
        }
      }

      if (!media.information.empty()) {
        results.push_back({.player = player, .media = {std::move(media)}});
      }
      break;
    }
  }

  return !results.empty();
}

}  // namespace anisthesia::linux::process
