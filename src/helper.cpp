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
                "The operation could not be stopped after it timed out.",
            };
        }
        return {false, QString(), "The operation timed out."};
    }

    const QString output = QString::fromUtf8(process.readAllStandardOutput());
    const QString error = QString::fromUtf8(process.readAllStandardError()).trimmed();
    const bool success = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    return {success, output, error};
}

static QString commandError(const CommandResult &result, const QString &fallback) {
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
        return failure("Another NextDNS change is already in progress.");
    }

    const QString profile = arguments.value("profile").toString().trimmed().toLower();

    if (!arguments.contains("profile") || !NetworkState::validNextDnsProfile(profile)) {
        return failure("Enter a six-character NextDNS profile ID using 0-9 or A-F.");
    }

    const CommandResult update = run(NEXTDNS_EXECUTABLE, {"config", "set", "-profile=" + profile});

    if (!update.success) {
        return failure(commandError(update, "The NextDNS profile ID could not be saved."));
    }

    return KAuth::ActionReply::SuccessReply();
}

KAuth::ActionReply DnsHelper::setmode(const QVariantMap &arguments) {
    QLockFile lock("/run/nextdns-control.lock");
    if (!lock.tryLock()) {
        return failure("Another NextDNS change is already in progress.");
    }
    if (!arguments.contains("enabled") ||
        arguments.value("enabled").metaType().id() != QMetaType::Bool) {
        return failure("The requested DNS mode is not valid.");
    }

    const bool enabled = arguments.value("enabled").toBool();
    if (enabled) {
        const CommandResult start = run(NEXTDNS_EXECUTABLE, {"start"});
        if (!start.success) {
            return failure(commandError(start, "NextDNS could not be started."));
        }
        const CommandResult activate = run(NEXTDNS_EXECUTABLE, {"activate"});
        if (activate.success) {
            return KAuth::ActionReply::SuccessReply();
        }
        const CommandResult cleanup = run(NEXTDNS_EXECUTABLE, {"stop"});
        const QString cleanupError =
            cleanup.success
                ? QString()
                : " NextDNS also could not be stopped: " +
                      commandError(cleanup, "No additional error details were provided.");
        return failure(commandError(activate, "NextDNS could not be activated.") + cleanupError);
    }

    const CommandResult deactivate = run(NEXTDNS_EXECUTABLE, {"deactivate"});
    if (!deactivate.success) {
        return failure(commandError(deactivate, "NextDNS could not be deactivated."));
    }
    return KAuth::ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("io.github.n16akbar.nextdnscontrol", DnsHelper)
