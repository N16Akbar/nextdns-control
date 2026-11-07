#pragma once

#include <QList>
#include <QString>
#include <QStringList>

struct NetworkCommand {
  QString program;
  QStringList arguments;
};
QStringList dnsFields();
QStringList dnsArguments(const QString &uuid, const QStringList &values);
bool dnsMatchesNextDns(const QStringList &values);
QList<NetworkCommand> networkCommands(bool enabled, const QString &uuid,
                                      const QStringList &restoreValues);
