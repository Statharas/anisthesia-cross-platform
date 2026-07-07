#include <anisthesia.hpp>
#include <anisthesia/linux_process.hpp>
#include <anisthesia/util.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

void Require(const bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

void ParsePlayersDataReadsAllSections() {
  std::vector<anisthesia::Player> players;
  const auto parsed = anisthesia::ParsePlayersData(R"(
# comments and blank lines are ignored
mpv
	windows
		mpv
	executables
		mpv
	strategies
		window_title
			{title}
	type
		default

Firefox
	windows
		MozillaWindowClass
	executables
		firefox
	strategies
		window_title
			{title}
	type
		web_browser
)",
                                                    players);

  Require(parsed, "players data should parse");
  Require(players.size() == 2, "players data should contain two players");
  Require(players[0].name == "mpv", "first player name changed");
  Require(players[0].windows == std::vector<std::string>{"mpv"},
          "first player windows changed");
  Require(players[0].executables == std::vector<std::string>{"mpv"},
          "first player executables changed");
  Require(players[0].strategies == std::vector<anisthesia::Strategy>{
                                   anisthesia::Strategy::WindowTitle},
          "first player strategies changed");
  Require(players[0].window_title_format == "{title}",
          "window-title strategy format changed");
  Require(players[0].type == anisthesia::PlayerType::Default,
          "first player type changed");
  Require(players[1].type == anisthesia::PlayerType::WebBrowser,
          "web browser player type changed");
}

void ParsePlayersDataRejectsInvalidIndentation() {
  std::vector<anisthesia::Player> players;
  Require(!anisthesia::ParsePlayersData("mpv\n\t\twindows\n", players),
          "invalid indentation should be rejected");
}

void ParsePlayersDataRejectsUnknownSections() {
  std::vector<anisthesia::Player> players;
  Require(!anisthesia::ParsePlayersData("mpv\n\tunknown\n\t\tvalue\n", players),
          "unknown sections should be rejected");
}

void ParseBundledPlayersFile() {
  std::vector<anisthesia::Player> players;
  Require(anisthesia::ParsePlayersFile(ANISTHESIA_PLAYERS_PATH, players),
          "bundled players.anisthesia should parse");
  Require(std::any_of(players.begin(), players.end(),
                      [](const auto& player) { return player.name == "Haruna"; }),
          "bundled players.anisthesia should include Haruna");
}

void UtilitiesTrimAndComparePredictably() {
  std::string value = "\tExample\r\n";
  Require(anisthesia::detail::util::TrimLeft(value, "\t"),
          "TrimLeft should remove leading tab");
  Require(anisthesia::detail::util::TrimRight(value, "\r\n"),
          "TrimRight should remove line ending");
  Require(value == "Example", "trimmed value changed");
  Require(anisthesia::detail::util::EqualStrings("MPV", "mpv"),
          "EqualStrings should ignore ASCII case");
  Require(!anisthesia::detail::util::EqualStrings("mpv", "vlc"),
          "EqualStrings should not accept different strings");
}

void PlatformBridgeHasStableEmptyResultContract() {
  std::vector<anisthesia::Result> results;
  const auto reject_all_media = [](const anisthesia::MediaInfo&) { return false; };

  Require(!anisthesia::GetResults({}, reject_all_media, results),
          "platform bridge should report no results without accepted media");
  Require(results.empty(), "platform bridge should leave no empty-input results");
}

void LinuxProcessCommandLineFindsVideoFiles() {
  const auto media = anisthesia::linux::process::MediaFromCommandLine({
      "/usr/bin/haruna",
      "/home/user/Downloads/[SubsPlease] Fate Strange Fake - 01v2 (1080p) [7708674D].mkv",
      "--fullscreen",
      "/home/user/Downloads/not-video.srt",
  });

  Require(media.size() == 1, "Linux process backend should find one video argument");
  Require(media.front().type == anisthesia::MediaInfoType::File,
          "Linux process backend should report argv media as a file");
  Require(media.front().value.ends_with("[7708674D].mkv"),
          "Linux process backend changed detected video path");
  Require(anisthesia::linux::process::IsVideoPath("episode.webm"),
          "Linux process backend should accept webm files");
  Require(!anisthesia::linux::process::IsVideoPath("subtitle.ass"),
          "Linux process backend should reject subtitle files");
}

void LinuxProcessBackendFindsCurrentProcess(const char* executable, const char* video_path) {
  std::vector<anisthesia::Result> results;
  anisthesia::Player player;
  player.name = "Anisthesia test";
  player.executables.push_back(std::filesystem::path(executable).filename().string());
  player.strategies.push_back(anisthesia::Strategy::OpenFiles);

  const auto accepts_all_media = [](const anisthesia::MediaInfo&) { return true; };
  Require(anisthesia::linux::process::GetResults({player}, accepts_all_media, results),
          "Linux process backend should find the current process video argument");
  Require(!results.empty(), "Linux process backend returned no current-process results");
  Require(results.front().player.name == "Anisthesia test",
          "Linux process backend changed matched player");
  Require(!results.front().media.empty(), "Linux process backend returned no media");
  Require(results.front().media.front().information.front().value == video_path,
          "Linux process backend changed current-process video argument");
}

}  // namespace

int main(int argc, char* argv[]) {
  ParsePlayersDataReadsAllSections();
  ParsePlayersDataRejectsInvalidIndentation();
  ParsePlayersDataRejectsUnknownSections();
  ParseBundledPlayersFile();
  UtilitiesTrimAndComparePredictably();
  PlatformBridgeHasStableEmptyResultContract();
  LinuxProcessCommandLineFindsVideoFiles();
  if (argc > 1) {
    LinuxProcessBackendFindsCurrentProcess(argv[0], argv[1]);
  }
  return EXIT_SUCCESS;
}
