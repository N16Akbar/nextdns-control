#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QtCore/QStringList>
#include <QtQml/qqmlregistration.h>
#include <functional>

class QProcess;

class DnsController : public QObject {
  Q_OBJECT
  QML_ELEMENT
  Q_PROPERTY(bool enabled READ enabled NOTIFY stateChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY stateChanged)
  Q_PROPERTY(bool stateKnown READ stateKnown NOTIFY stateChanged)
  Q_PROPERTY(QString profile READ profile NOTIFY stateChanged)
  Q_PROPERTY(QString dnsSummary READ dnsSummary NOTIFY stateChanged)
  Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY stateChanged)

public:
  explicit DnsController(QObject *parent = nullptr);

  bool enabled() const;
  bool busy() const;
  bool stateKnown() const;
  QString profile() const;
  QString dnsSummary() const;
  QString errorMessage() const;

  Q_INVOKABLE void refresh();
  Q_INVOKABLE void setEnabled(bool enabled);
  Q_INVOKABLE void setProfile(const QString &profile);

Q_SIGNALS:
  void stateChanged();
  void profileSaved();

private:
  using ProcessCallback =
      std::function<void(bool, const QString &, const QString &)>;

  void runProcess(const QString &program, const QStringList &arguments,
                  ProcessCallback callback);
  void runAction(const QString &name, const QVariantMap &arguments,
                 bool profileAction);
  void refreshMode();
  void finishRefresh(bool serviceActive, const QString &dnsServers);
  void failRefresh(const QString &errorMessage);

  bool m_enabled = false;
  bool m_busy = false;
  bool m_stateKnown = false;
  QString m_profile;
  QString m_dnsSummary;
  QString m_profileError;
  QString m_errorMessage;
  QProcess *m_process = nullptr;
};
