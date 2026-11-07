#include "networkcommands.h"
#include "paths.h"

static const QStringList nextDnsValues = {"yes", "127.0.0.1", "-1",
                                          "yes", "::1",       "-1"};

QStringList dnsFields() {
  return {"ipv4.ignore-auto-dns", "ipv4.dns", "ipv4.dns-priority",
          "ipv6.ignore-auto-dns", "ipv6.dns", "ipv6.dns-priority"};
}

QStringList dnsArguments(const QString &uuid, const QStringList &values) {
  return {"connection",
          "modify",
          "uuid",
          uuid,
          "ipv4.ignore-auto-dns",
          values.at(0),
          "ipv4.dns",
          values.at(1),
          "ipv4.dns-priority",
          values.at(2),
          "ipv6.ignore-auto-dns",
          values.at(3),
          "ipv6.dns",
          values.at(4),
          "ipv6.dns-priority",
          values.at(5)};
}

bool dnsMatchesNextDns(const QStringList &values) {
  return values == nextDnsValues;
}

QList<NetworkCommand> networkCommands(bool enabled, const QString &uuid,
                                      const QStringList &restoreValues) {
  if (enabled) {
    return {
        {SYSTEMCTL_EXECUTABLE, {"enable", "--now", "nextdns.service"}},
        {NMCLI_EXECUTABLE, dnsArguments(uuid, nextDnsValues)},
        {NMCLI_EXECUTABLE, {"connection", "up", "uuid", uuid}},
    };
  }

  const QStringList values = restoreValues.size() == 6
                                 ? restoreValues
                                 : QStringList({"no", "", "0", "no", "", "0"});
  return {
      {NMCLI_EXECUTABLE, dnsArguments(uuid, values)},
      {NMCLI_EXECUTABLE, {"connection", "up", "uuid", uuid}},
      {SYSTEMCTL_EXECUTABLE, {"disable", "--now", "nextdns.service"}},
  };
}
