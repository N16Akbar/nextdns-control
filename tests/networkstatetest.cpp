#include "networkstate.h"
#include <QTest>
class NetworkStateTest : public QObject {
  Q_OBJECT
private Q_SLOTS:
  void nextDnsDetection() {
    QVERIFY(NetworkState::usesNextDns("45.90.28.0"));
    QVERIFY(NetworkState::usesNextDns(
        "45.90.30.0#abcdef.dns.nextdns.io"));
    QVERIFY(NetworkState::usesNextDns("2a07:a8c0::"));
    QVERIFY(NetworkState::usesNextDns("2a07:a8c1::"));
    QVERIFY(NetworkState::usesNextDns(
        "Global: 45.90.28.0 | 45.90.30.0"));
    QVERIFY(!NetworkState::usesNextDns("127.0.0.1\n::1\n"));
    QVERIFY(!NetworkState::usesNextDns(
        "182.9.96.6 | 182.9.96.5"));
    QVERIFY(!NetworkState::usesNextDns(QString()));
  }
  void resolverParsing() {
    QCOMPARE(NetworkState::resolvedDnsServers("Link 3 (wlp2s0): 127.0.0.1"),
             QString("127.0.0.1"));
    QVERIFY(NetworkState::resolvedDnsServers("invalid").isEmpty());
  }
  void profileParsing() {
    QCOMPARE(NetworkState::nextDnsProfile("profile abcdef\n"),
             QString("abcdef"));
    QVERIFY(NetworkState::nextDnsProfile("profile invalid\n").isEmpty());
    QVERIFY(NetworkState::nextDnsProfile("status running\n").isEmpty());
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
  void activeStateParsing() {
    bool active = false;
    QVERIFY(NetworkState::activeState("active\n", &active));
    QVERIFY(active);
    QVERIFY(NetworkState::activeState("inactive\n", &active));
    QVERIFY(!active);
    QVERIFY(!NetworkState::activeState("failed\n", &active));
  }
  void modeState() {
    QVERIFY(NetworkState::modeState(true, true).enabled);
    QVERIFY(NetworkState::modeState(true, false).enabled);
    QVERIFY(!NetworkState::modeState(false, true).enabled);
    QVERIFY(!NetworkState::modeState(false, false).enabled);
    QVERIFY(!NetworkState::modeState(true, false).mismatch);
    QVERIFY(!NetworkState::modeState(false, true).mismatch);
  }
  void mismatchMessage() {
    QVERIFY(NetworkState::mismatchMessage(true, true).isEmpty());
    QVERIFY(NetworkState::mismatchMessage(true, false).isEmpty());
    QVERIFY(NetworkState::mismatchMessage(false, true).isEmpty());
    QVERIFY(NetworkState::mismatchMessage(false, false).isEmpty());
  }
};
QTEST_MAIN(NetworkStateTest)
#include "networkstatetest.moc"
