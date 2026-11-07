#include "networkstate.h"

#include <QRegularExpression>

QString NetworkState::defaultDevice(const QString &routeOutput) {
  const QStringList fields = routeOutput.split(' ', Qt::SkipEmptyParts);
  const qsizetype index = fields.indexOf("dev");
  return index >= 0 && index + 1 < fields.size() ? fields.at(index + 1)
                                                 : QString();
}

bool NetworkState::usesNextDns(const QString &dnsOutput) {
  const QStringList servers =
      dnsOutput.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  if (servers.isEmpty()) {
    return false;
  }

  for (const QString &server : servers) {
    if (server != "127.0.0.1" && server != "::1") {
      return false;
    }
  }

  return true;
}

QString NetworkState::nextDnsProfile(const QString &output) {
  const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
  for (const QString &line : lines) {
    if (line.startsWith("profile ")) {
      const QString profile = line.sliced(8).trimmed().toLower();
      return validNextDnsProfile(profile) ? profile : QString();
    }
  }
  return QString();
}

bool NetworkState::validNextDnsProfile(const QString &profile) {
  static const QRegularExpression expression("^[0-9a-fA-F]{6}$");
  return expression.match(profile).hasMatch();
}

QStringList NetworkState::profileValues(const QString &output) {
  QStringList values = output.split('\n', Qt::KeepEmptyParts);
  if (!values.isEmpty() && values.last().isEmpty()) {
    values.removeLast();
  }
  return values;
}

QStringList NetworkState::liveDnsArguments(const QString &device) {
  return {"--escape", "no",   "-g",  "GENERAL.TYPE,IP4.DNS,IP6.DNS",
          "device",   "show", device};
}

QString NetworkState::deviceType(const QString &deviceOutput) {
  const qsizetype newline = deviceOutput.indexOf('\n');
  return (newline < 0 ? deviceOutput : deviceOutput.left(newline)).trimmed();
}

QString NetworkState::liveDnsServers(const QString &deviceOutput) {
  const qsizetype newline = deviceOutput.indexOf('\n');
  return newline < 0 ? QString() : deviceOutput.mid(newline + 1);
}

QString NetworkState::resolvedDnsServers(const QString &resolvedOutput) {
  const qsizetype separator = resolvedOutput.indexOf(": ");
  return separator < 0 ? QString()
                       : resolvedOutput.mid(separator + 2).trimmed();
}

QString NetworkState::dnsSummary(const QString &dnsOutput) {
  QStringList servers =
      dnsOutput.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  servers.removeAll("|");
  return servers.isEmpty() ? QString("None reported") : servers.join(", ");
}

bool NetworkState::supportedDeviceType(const QString &type) {
  static const QStringList supported = {
      "adsl",     "bluetooth", "bridge",     "bond",   "cdma",    "ethernet",
      "generic",  "gsm",       "infiniband", "ipvlan", "macvlan", "modem",
      "olpc-gsm", "team",      "vlan",       "wifi",   "wimax",
  };
  return supported.contains(type);
}

QString NetworkState::unsupportedDeviceMessage(const QString &type) {
  return QString("The default network device type \"%1\" is not supported.")
      .arg(type);
}

NetworkState::ModeState NetworkState::modeState(bool serviceActive,
                                                bool serviceEnabled,
                                                bool localDns) {
  Q_UNUSED(serviceEnabled)
  const bool enabled = serviceActive && localDns;
  const bool nativeDns = !serviceActive && !localDns;
  return {enabled, serviceActive, !enabled && !nativeDns};
}

bool NetworkState::activeState(const QString &value, bool *state) {
  const QString normalized = value.trimmed();
  if (normalized == "active") {
    *state = true;
    return true;
  }
  if (normalized == "inactive") {
    *state = false;
    return true;
  }
  return false;
}

bool NetworkState::enabledState(const QString &value, bool *state) {
  const QString normalized = value.trimmed();
  if (normalized == "enabled") {
    *state = true;
    return true;
  }
  if (normalized == "disabled" || normalized == "static") {
    *state = false;
    return true;
  }
  return false;
}
