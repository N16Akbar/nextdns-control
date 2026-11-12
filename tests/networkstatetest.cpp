#include "networkstate.h"
#include <QTest>
class NetworkStateTest : public QObject {
    Q_OBJECT
  private Q_SLOTS:
    void resolverParsing() {
        QCOMPARE(NetworkState::resolvedDnsServers("Link 3 (wlp2s0): 127.0.0.1"),
                 QString("127.0.0.1"));
        QVERIFY(NetworkState::resolvedDnsServers("invalid").isEmpty());
    }
    void profileParsing() {
        QCOMPARE(NetworkState::nextDnsProfile("profile abcdef\n"), QString("abcdef"));
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
};
QTEST_MAIN(NetworkStateTest)
#include "networkstatetest.moc"
