#include "networkstate.h"
#include <QTest>
class NetworkStateTest : public QObject {
  Q_OBJECT
private Q_SLOTS:
  void nextDnsDetection() {
    QVERIFY(NetworkState::usesNextDns("127.0.0.1\n::1\n"));
    QVERIFY(!NetworkState::usesNextDns("127.0.0.1\n1.1.1.1\n"));
    QVERIFY(!NetworkState::usesNextDns(QString()));
  }
  void resolverParsing() {
    QCOMPARE(NetworkState::resolvedDnsServers("Link 3 (wlp2s0): 127.0.0.1"),
             QString("127.0.0.1"));
    QVERIFY(NetworkState::resolvedDnsServers("invalid").isEmpty());
  }
  void profileValidation() {
    QVERIFY(NetworkState::validNextDnsProfile("abcdef"));
    QVERIFY(NetworkState::validNextDnsProfile("ABCDEF"));
    QVERIFY(!NetworkState::validNextDnsProfile("abcde"));
    QVERIFY(!NetworkState::validNextDnsProfile("zzzzzz"));
  }
  void deviceSupport() {
    QVERIFY(NetworkState::supportedDeviceType("wifi"));
    QVERIFY(NetworkState::supportedDeviceType("ethernet"));
    QVERIFY(!NetworkState::supportedDeviceType("bridge"));
    QVERIFY(!NetworkState::supportedDeviceType("vlan"));
  }
  void modeState() {
    QVERIFY(NetworkState::modeState(true, true).enabled);
    QVERIFY(!NetworkState::modeState(false, false).mismatch);
    QVERIFY(NetworkState::modeState(true, false).mismatch);
    QVERIFY(NetworkState::modeState(false, true).mismatch);
  }
};
QTEST_MAIN(NetworkStateTest)
#include "networkstatetest.moc"
