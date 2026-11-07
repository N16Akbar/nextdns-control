#pragma once

#include <QString>
#include <QStringList>

namespace NetworkState {
struct ModeState {
  bool enabled;
  bool repairToNextDns;
  bool mismatch;
};

QString defaultDevice(const QString &routeOutput);
bool usesNextDns(const QString &dnsOutput);
QString nextDnsProfile(const QString &output);
bool validNextDnsProfile(const QString &profile);
QStringList profileValues(const QString &output);
QStringList liveDnsArguments(const QString &device);
QString deviceType(const QString &deviceOutput);
QString liveDnsServers(const QString &deviceOutput);
QString resolvedDnsServers(const QString &resolvedOutput);
QString dnsSummary(const QString &dnsOutput);
bool supportedDeviceType(const QString &type);
QString unsupportedDeviceMessage(const QString &type);

bool activeState(const QString &value, bool *state);
bool enabledState(const QString &value, bool *state);
ModeState modeState(bool serviceActive, bool serviceEnabled, bool localDns);
} // namespace NetworkState
