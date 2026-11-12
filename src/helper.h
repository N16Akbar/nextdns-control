#pragma once
#include <KAuth/ActionReply>
#include <QObject>
#include <QVariantMap>
class DnsHelper : public QObject {
    Q_OBJECT
  public Q_SLOTS:
    KAuth::ActionReply setmode(const QVariantMap &arguments);
    KAuth::ActionReply setprofile(const QVariantMap &arguments);
};
