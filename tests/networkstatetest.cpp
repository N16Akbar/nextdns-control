#include "networkstate.h"
#include "networkcommands.h"
#include "stash.h"

#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>

#include <cassert>

int main(int argc, char **argv) {
  QCoreApplication application(argc, argv);

  assert(NetworkState::usesNextDns("127.0.0.1\n::1\n"));
  assert(!NetworkState::usesNextDns("127.0.0.1\n1.1.1.1\n"));
  assert(!NetworkState::usesNextDns(""));

  assert(NetworkState::dnsSummary("127.0.0.1\n::1\n") == "127.0.0.1, ::1");
  assert(NetworkState::dnsSummary("1.1.1.1\n1.0.0.1\n") == "1.1.1.1, 1.0.0.1");
  assert(NetworkState::dnsSummary("1.1.1.1 | 1.0.0.1") == "1.1.1.1, 1.0.0.1");
  assert(NetworkState::dnsSummary("") == "None reported");

  assert(NetworkState::resolvedDnsServers("Link 3 (wlp2s0): 127.0.0.1") ==
         "127.0.0.1");
  assert(NetworkState::resolvedDnsServers(
             "Link 3 (wlp2s0): 182.9.96.6 182.9.96.5") ==
         "182.9.96.6 182.9.96.5");
  assert(NetworkState::resolvedDnsServers(
             "Link 3 (wlp2s0): 2606:4700:4700::1111") ==
         "2606:4700:4700::1111");
  assert(NetworkState::resolvedDnsServers("invalid").isEmpty());

  assert(NetworkState::defaultDevice(
             "1.1.1.1 via 192.168.1.1 dev wlp2s0 src 192.168.1.2") == "wlp2s0");

  assert(NetworkState::validNextDnsProfile("abcdef"));
  assert(!NetworkState::validNextDnsProfile("abcde"));

  assert(dnsMatchesNextDns({"yes", "127.0.0.1", "-1", "yes", "::1", "-1"}));

  const NetworkState::ModeState nextDns =
      NetworkState::modeState(true, true, true);
  assert(nextDns.enabled);
  assert(!nextDns.mismatch);

  const NetworkState::ModeState nativeDns =
      NetworkState::modeState(false, true, false);
  assert(!nativeDns.enabled);
  assert(!nativeDns.mismatch);

  assert(NetworkState::modeState(true, true, false).mismatch);
  assert(NetworkState::modeState(false, true, true).mismatch);

  QTemporaryDir directory;
  assert(directory.isValid());

  const QString uuid = "e5b31608-6a3c-41eb-add2-d2b695711625";
  QFile legacyFile(directory.path() + '/' + uuid);
  assert(legacyFile.open(QIODevice::WriteOnly));
  assert(legacyFile.write("no\n\n0\nno\n\n0") == 11);
  legacyFile.close();

  const Stash::Result legacy = Stash::read(directory.path(), uuid);
  assert(legacy.status == Stash::Status::Valid);
  assert(legacy.values == QStringList({"no", "", "0", "no", "", "0"}));
  assert(Stash::remove(directory.path(), uuid).isEmpty());
  assert(!legacyFile.exists());

  return 0;
}
