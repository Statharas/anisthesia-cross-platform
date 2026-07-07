#include <anisthesia.hpp>
#include <anisthesia/util.hpp>

#include <cstdlib>
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

}  // namespace

int main() {
  ParsePlayersDataReadsAllSections();
  ParsePlayersDataRejectsInvalidIndentation();
  ParsePlayersDataRejectsUnknownSections();
  UtilitiesTrimAndComparePredictably();
  PlatformBridgeHasStableEmptyResultContract();
  return EXIT_SUCCESS;
}
