#include "helper.h"
#include "networkstate.h"
#include "paths.h"

#include <KAuth/HelperSupport>
#include <QLockFile>
#include <QProcess>
#include <QStringList>

struct CommandResult {
  bool success;
  QString output;
  QString error;
};

static CommandResult run(const QString &program, const QStringList &arguments) {
  QProcess process;
  process.start(program, arguments);
  if (!process.waitForStarted()) {
    return {false, QString(), process.errorString()};
  }
  if (!process.waitForFinished(120000)) {
    process.kill();
    if (!process.waitForFinished(5000)) {
      return {
          false,
          QString(),
          "The command did not stop after timing out.",
      };
    }
    return {false, QString(), "The command timed out."};
  }

  const QString output = QString::fromUtf8(process.readAllStandardOutput());
  const QString error =
      QString::fromUtf8(process.readAllStandardError()).trimmed();
  const bool success =
      process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
  return {success, output, error};
}

static QString commandError(const CommandResult &result,
                            const QString &fallback) {
  if (!result.error.isEmpty()) {
    return result.error;
  }

  const QString output = result.output.trimmed();
  return output.isEmpty() ? fallback : output;
}

static KAuth::ActionReply failure(const QString &description) {
  KAuth::ActionReply reply = KAuth::ActionReply::HelperErrorReply();
  reply.setErrorDescription(description);
  return reply;
}

KAuth::ActionReply DnsHelper::setprofile(const QVariantMap &arguments) {
  QLockFile lock("/run/nextdns-control.lock");
  if (!lock.tryLock()) {
    return failure("Another NextDNS operation is running.");
  }
  const QString profile =
      arguments.value("profile").toString().trimmed().toLower();
  if (!arguments.contains("profile") ||
      !NetworkState::validNextDnsProfile(profile)) {
    return failure("Enter a valid six-character NextDNS profile ID.");
  }
  const CommandResult config = run(NEXTDNS_EXECUTABLE, {"config"});
  const QString previous = NetworkState::nextDnsProfile(config.output);
  if (!config.success || previous.isEmpty()) {
    return failure(
        commandError(config, "The current NextDNS profile could not be read."));
  }
  const CommandResult activeResult =
      run(SYSTEMCTL_EXECUTABLE,
          {"show", "--property=ActiveState", "--value", "nextdns.service"});
  bool active = false;
  if (!activeResult.success ||
      !NetworkState::activeState(activeResult.output, &active)) {
    return failure(commandError(
        activeResult, "The NextDNS service state could not be read."));
  }
  if (!active) {
    return failure("Enable NextDNS before changing its profile.");
  }
  const CommandResult update =
      run(NEXTDNS_EXECUTABLE, {"config", "set", "-profile=" + profile});
  if (!update.success) {
    return failure(
        commandError(update, "The NextDNS profile could not be changed."));
  }
  const CommandResult restart = run(NEXTDNS_EXECUTABLE, {"restart"});
  if (restart.success) {
    return KAuth::ActionReply::SuccessReply();
  }
  const CommandResult rollback =
      run(NEXTDNS_EXECUTABLE, {"config", "set", "-profile=" + previous});
  const CommandResult rollbackRestart =
      rollback.success
          ? run(NEXTDNS_EXECUTABLE, {"restart"})
          : CommandResult{false, QString(),
                          "The previous profile could not be restored."};
  QString rollbackError;
  if (!rollback.success || !rollbackRestart.success) {
    rollbackError = " Rollback failed: " +
        commandError(rollback.success ? rollbackRestart : rollback,
                     "The previous profile could not be restored.");
  } else {
    const CommandResult verifyConfig = run(NEXTDNS_EXECUTABLE, {"config"});
    const CommandResult verifyStatus = run(NEXTDNS_EXECUTABLE, {"status"});
    if (!verifyConfig.success ||
        NetworkState::nextDnsProfile(verifyConfig.output) != previous ||
        !verifyStatus.success || verifyStatus.output.trimmed() != "running") {
      rollbackError = " Rollback failed: the previous profile is not active.";
    }
  }
  return failure(commandError(restart, "NextDNS could not be restarted.") +
                 rollbackError);
}

KAuth::ActionReply DnsHelper::setmode(const QVariantMap &arguments) {
  QLockFile lock("/run/nextdns-control.lock");
  if (!lock.tryLock()) {
    return failure("Another NextDNS operation is running.");
  }
  if (!arguments.contains("enabled") ||
      arguments.value("enabled").metaType().id() != QMetaType::Bool) {
    return failure("The requested DNS mode is invalid.");
  }

  const bool enabled = arguments.value("enabled").toBool();
  if (enabled) {
    const CommandResult start = run(NEXTDNS_EXECUTABLE, {"start"});
    if (!start.success) {
      return failure(commandError(start, "NextDNS start failed."));
    }
    const CommandResult activate = run(NEXTDNS_EXECUTABLE, {"activate"});
    if (activate.success) {
      return KAuth::ActionReply::SuccessReply();
    }
    const CommandResult cleanup = run(NEXTDNS_EXECUTABLE, {"stop"});
    const QString cleanupError = cleanup.success
        ? QString()
        : " Cleanup failed: " + commandError(cleanup, "NextDNS stop failed.");
    return failure(commandError(activate, "NextDNS activate failed.") +
                   cleanupError);
  }

  const CommandResult deactivate = run(NEXTDNS_EXECUTABLE, {"deactivate"});
  if (!deactivate.success) {
    return failure(commandError(deactivate, "NextDNS deactivate failed."));
  }
  const CommandResult route =
      run(IP_EXECUTABLE, {"-o", "route", "get", "1.1.1.1"});
  const QString device = NetworkState::defaultDevice(route.output);
  if (!route.success || device.isEmpty()) {
    return failure(commandError(
        route, "The default network device could not be determined."));
  }
  const CommandResult revert = run(RESOLVECTL_EXECUTABLE, {"revert", device});
  if (!revert.success) {
    return failure(commandError(revert, "The resolver could not be restored."));
  }
  const CommandResult uuidResult = run(
      NMCLI_EXECUTABLE, {"-g", "GENERAL.CON-UUID", "device", "show", device});
  const QString uuid = uuidResult.output.trimmed();
  if (!uuidResult.success || uuid.isEmpty()) {
    return failure(commandError(
        uuidResult, "The active network connection could not be determined."));
  }
  const CommandResult reconnect =
      run(NMCLI_EXECUTABLE, {"connection", "up", "uuid", uuid});
  if (!reconnect.success) {
    return failure(commandError(
        reconnect, "The native DNS connection could not be reactivated."));
  }
  const CommandResult stop = run(NEXTDNS_EXECUTABLE, {"stop"});
  if (!stop.success) {
    return failure(commandError(stop, "NextDNS stop failed."));
  }

  return KAuth::ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("io.github.n16akbar.nextdnscontrol", DnsHelper)
