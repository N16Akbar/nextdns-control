#include "dnscontroller.h"
#include "networkstate.h"
#include "paths.h"

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KJob>
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>
#include <QTimer>
#include <QVariantMap>
#include <QWindow>

#include <memory>

namespace {
constexpr int processTimeoutMs = 30000;
}

DnsController::DnsController(QObject *parent) : QObject(parent) {
  QTimer::singleShot(0, this, &DnsController::refresh);
}

bool DnsController::enabled() const { return m_enabled; }

bool DnsController::busy() const { return m_busy; }

bool DnsController::stateKnown() const { return m_stateKnown; }

QString DnsController::profile() const { return m_profile; }

QString DnsController::dnsSummary() const { return m_dnsSummary; }

QString DnsController::errorMessage() const { return m_errorMessage; }

QString DnsController::warningMessage() const { return m_warning; }

void DnsController::runProcess(const QString &program,
                               const QStringList &arguments,
                               ProcessCallback callback) {
  QProcess *process = new QProcess(this);
  m_process = process;

  auto timedOut = std::make_shared<bool>(false);

  auto *timeout = new QTimer(process);
  timeout->setSingleShot(true);
  timeout->start(processTimeoutMs);

  connect(process, &QProcess::errorOccurred, this,
          [this, process, callback](QProcess::ProcessError error) {
            if (error != QProcess::FailedToStart || m_process != process) {
              return;
            }

            const QString message = process->errorString();
            process->deleteLater();
            m_process = nullptr;
            callback(false, QString(), message);
          });

  connect(
      process, &QProcess::finished, this,
      [this, process, callback, timedOut](int exitCode,
                                          QProcess::ExitStatus exitStatus) {
        if (m_process != process) {
          return;
        }

        const QString output =
            QString::fromUtf8(process->readAllStandardOutput()).trimmed();
        const QString error =
            *timedOut
                ? QString("The operation timed out.")
                : QString::fromUtf8(process->readAllStandardError()).trimmed();
        const bool success =
            !*timedOut && exitStatus == QProcess::NormalExit && exitCode == 0;
        process->deleteLater();
        m_process = nullptr;
        callback(success, output, error);
      });

  connect(timeout, &QTimer::timeout, this, [this, process, timedOut]() {
    if (m_process != process) {
      return;
    }
    *timedOut = true;
    process->kill();
  });

  process->start(program, arguments);
}

void DnsController::refresh() {
  if (m_process || m_busy) {
    return;
  }
  m_busy = true;
  m_errorMessage.clear();
  m_profileError.clear();
  m_warning.clear();
  Q_EMIT stateChanged();
  runProcess(NEXTDNS_EXECUTABLE, {"config"},
             [this](bool success, const QString &output, const QString &error) {
               const QString profile = NetworkState::nextDnsProfile(output);
               if (success && !profile.isEmpty()) {
                 m_profile = profile;
               } else {
                 m_profileError = error.isEmpty()
                                      ? QString()
                                      : error;
               }
               refreshMode();
             });
}

void DnsController::refreshMode() {
  runProcess(
      NEXTDNS_EXECUTABLE, {"status"},
      [this](bool ok, const QString &output, const QString &error) {
        const QString status = output.trimmed();
        const bool running = status == "running";
        const bool stopped = status == "stopped";
        if (!ok || (!running && !stopped)) {
          failRefresh(error.isEmpty() ? "The NextDNS status could not be read."
                                      : error);
          return;
        }
        runProcess(
            IP_EXECUTABLE, {"-o", "route", "get", "1.1.1.1"},
            [this, running](bool ok, const QString &output,
                            const QString &error) {
              const QString device = NetworkState::defaultDevice(output);
              if (!ok || device.isEmpty()) {
                failRefresh(error.isEmpty()
                                ? "The active network device could not be determined."
                                : error);
                return;
              }
              runProcess(
                  RESOLVECTL_EXECUTABLE, {"dns", device},
                  [this, running, device](bool ok, const QString &output,
                                           const QString &error) {
                    const QString dnsServers =
                        ok ? NetworkState::resolvedDnsServers(output)
                           : QString();
                    if (!dnsServers.isEmpty()) {
                      finishRefresh(running, dnsServers);
                      return;
                    }

                    if (running) {
                      const QString profileHost =
                          m_profile + ".dns.nextdns.io";
                      const QString globalServers =
                          "45.90.28.0#" + profileHost + " "
                          "45.90.30.0#" + profileHost + " "
                          "2a07:a8c0::#" + profileHost + " "
                          "2a07:a8c1::#" + profileHost;

                      finishRefresh(true, globalServers);
                      return;
                    }

                    refreshLiveDns(false, device, error);
                  });
            });
      });
}

void DnsController::refreshLiveDns(bool serviceActive, const QString &device,
                                   const QString &resolvedError) {
  runProcess(
      NMCLI_EXECUTABLE, NetworkState::liveDnsArguments(device),
      [this, serviceActive, resolvedError](bool ok, const QString &output,
                                           const QString &error) {
        if (!ok) {
          failRefresh(error.isEmpty()
                          ? (resolvedError.isEmpty()
                                 ? "The active DNS configuration is unavailable."
                                 : resolvedError)
                          : error);
          return;
        }
        const QString type = NetworkState::deviceType(output);
        if (!NetworkState::supportedDeviceType(type)) {
          failRefresh(NetworkState::unsupportedDeviceMessage(type));
          return;
        }
        const QString dnsServers = NetworkState::liveDnsServers(output);
        if (dnsServers.isEmpty()) {
          failRefresh("No DNS servers were reported for the active link.");
          return;
        }
        finishRefresh(serviceActive, dnsServers);
      });
}

void DnsController::finishRefresh(bool serviceActive,
                                  const QString &dnsServers) {
  const bool localDns = NetworkState::usesNextDns(dnsServers);
  const NetworkState::ModeState state =
      NetworkState::modeState(serviceActive, localDns);
  m_dnsSummary = NetworkState::dnsSummary(dnsServers);
  m_enabled = state.enabled;
  m_stateKnown = true;
  m_warning = NetworkState::mismatchMessage(serviceActive, localDns);
  m_errorMessage = m_profileError;
  m_busy = false;
  Q_EMIT stateChanged();
}

void DnsController::failRefresh(const QString &errorMessage) {
  m_errorMessage = errorMessage;
  m_warning.clear();
  m_stateKnown = false;
  m_busy = false;
  Q_EMIT stateChanged();
}

void DnsController::runAction(const QString &name,
                              const QVariantMap &arguments,
                              bool profileAction) {
  m_busy = true;
  m_errorMessage.clear();
  m_warning.clear();
  Q_EMIT stateChanged();
  QScreen *screen = QGuiApplication::primaryScreen();
  auto *authParent = new QWindow();
  if (screen) {
    const QPoint center = screen->availableGeometry().center();
    authParent->setScreen(screen);
    authParent->setGeometry(center.x(), center.y(), 1, 1);
  }
  authParent->setFlags(Qt::Tool | Qt::FramelessWindowHint |
                       Qt::WindowDoesNotAcceptFocus);
  authParent->setOpacity(0);
  authParent->show();
  KAuth::Action action(name);
  action.setHelperId("io.github.n16akbar.nextdnscontrol");
  action.setArguments(arguments);
  action.setParentWindow(authParent);
  KAuth::ExecuteJob *job = action.execute();
  connect(job, &KJob::result, this,
          [this, authParent, arguments, profileAction](KJob *completedJob) {
            authParent->hide();
            authParent->deleteLater();
            if (completedJob->error()) {
              m_busy = false;
              m_errorMessage = completedJob->errorString();
              Q_EMIT stateChanged();
              Q_EMIT notificationRequested("NextDNS change failed",
                                           m_errorMessage);
              refresh();
              return;
            }
            m_busy = false;
            if (profileAction) {
              Q_EMIT profileSaved();
            } else {
              const bool enabled = arguments.value("enabled").toBool();
              Q_EMIT notificationRequested(
                  enabled ? "NextDNS enabled" : "NextDNS disabled",
                  enabled ? "Your DNS requests are now using NextDNS."
                          : "The default network DNS settings have been restored.");
            }
            refresh();
          });
  job->start();
}

void DnsController::setEnabled(bool enabled) {
  if (m_busy || !m_stateKnown) {
    return;
  }

  if (enabled &&
      !NetworkState::validNextDnsProfile(m_profile)) {
    m_errorMessage =
        "Enter and save your six-character NextDNS profile ID first.";
    Q_EMIT stateChanged();
    return;
  }

  QVariantMap arguments;
  arguments.insert("enabled", enabled);

  runAction(
      "io.github.n16akbar.nextdnscontrol.setmode",
      arguments,
      false);
}

void DnsController::setProfile(
    const QString &profile) {
  if (m_busy) {
    return;
  }

  if (!m_stateKnown) {
    m_errorMessage =
        "The resolver state is not known. Refresh and try again.";
    Q_EMIT stateChanged();
    return;
  }

  const QString normalized =
      profile.trimmed().toLower();

  if (!NetworkState::validNextDnsProfile(normalized)) {
    m_errorMessage =
        "Enter a six-character NextDNS profile ID using 0-9 or A-F.";
    Q_EMIT stateChanged();
    return;
  }

  QVariantMap arguments;
  arguments.insert("profile", normalized);

  runAction(
      "io.github.n16akbar.nextdnscontrol.setprofile",
      arguments,
      true);
}
