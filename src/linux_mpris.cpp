#include <anisthesia/linux_mpris.hpp>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>
#include <QUrl>

#include <algorithm>
#include <optional>
#include <ranges>
#include <utility>

namespace {

QVariant GetMprisProperty(const QString& service, const QString& property) {
  QDBusInterface properties{service, "/org/mpris/MediaPlayer2",
                            "org.freedesktop.DBus.Properties",
                            QDBusConnection::sessionBus()};
  const QDBusReply<QDBusVariant> reply =
      properties.call("Get", "org.mpris.MediaPlayer2.Player", property);
  return reply.isValid() ? reply.value().variant() : QVariant{};
}

std::optional<anisthesia::MediaInfo> MediaInfoFromMetadata(const QVariantMap& metadata) {
  if (const auto value = metadata.value("xesam:url").toString(); !value.isEmpty()) {
    const QUrl url{value};
    if (url.isLocalFile()) {
      return anisthesia::MediaInfo{
          .type = anisthesia::MediaInfoType::File,
          .value = url.toLocalFile().toStdString(),
      };
    }
  }

  if (const auto value = metadata.value("xesam:title").toString(); !value.isEmpty()) {
    return anisthesia::MediaInfo{
        .type = anisthesia::MediaInfoType::Title,
        .value = value.toStdString(),
    };
  }

  return std::nullopt;
}

bool AcceptsMedia(const anisthesia::media_proc_t& media_proc,
                  const anisthesia::MediaInfo& media_info) {
  return !media_proc || media_proc(media_info);
}

}  // namespace

namespace anisthesia::linux::mpris {

bool GetResults(const std::vector<Player>& players, media_proc_t media_proc,
                std::vector<Result>& results) {
  results.clear();

  const auto bus = QDBusConnection::sessionBus();
  const auto* interface = bus.interface();
  if (!interface) return false;

  const QDBusReply<QStringList> services_reply = interface->registeredServiceNames();
  if (!services_reply.isValid()) return false;

  for (const auto& service : services_reply.value()) {
    if (!service.startsWith("org.mpris.MediaPlayer2.")) continue;

    const auto playback_status = GetMprisProperty(service, "PlaybackStatus").toString();
    if (playback_status != "Playing") continue;

    const auto metadata = GetMprisProperty(service, "Metadata").toMap();
    const auto media_info = MediaInfoFromMetadata(metadata);
    if (!media_info || !AcceptsMedia(media_proc, *media_info)) continue;

    const auto player_name =
        service.mid(QStringLiteral("org.mpris.MediaPlayer2.").size()).toStdString();

    Player player;
    player.name = player_name;
    for (const auto& candidate : players) {
      const auto matches_name = candidate.name == player_name;
      const auto matches_executable =
          std::ranges::find(candidate.executables, player_name) != candidate.executables.end();
      if (matches_name || matches_executable) {
        player = candidate;
        break;
      }
    }

    Media media;
    media.state = MediaState::Playing;
    media.information = {*media_info};

    results.push_back({.player = std::move(player), .media = {std::move(media)}});
  }

  return !results.empty();
}

}  // namespace anisthesia::linux::mpris
