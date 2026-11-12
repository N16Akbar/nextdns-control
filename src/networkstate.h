#pragma once

#include <QString>
#include <QStringList>

namespace NetworkState {
QString defaultDevice(const QString &routeOutput);
QString nextDnsProfile(const QString &output);
bool validNextDnsProfile(const QString &profile);
QStringList liveDnsArguments(const QString &device);
QString deviceType(const QString &deviceOutput);
QString liveDnsServers(const QString &deviceOutput);
QString resolvedDnsServers(const QString &resolvedOutput);
QString dnsSummary(const QString &dnsOutput);
bool supportedDeviceType(const QString &type);
QString unsupportedDeviceMessage(const QString &type);

} // namespace NetworkState
