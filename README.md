# NextDNS Control

A Plasma 6 tray widget for switching between the NextDNS CLI and native network DNS.

This is an unofficial personal project. It is not affiliated with NextDNS or KDE.

## Supported setup

- Plasma 6
- Qt 6 and KDE Frameworks 6
- NetworkManager
- systemd-resolved
- NextDNS CLI
- Ethernet or Wi-Fi as the active default route

Bridge, bond, VLAN, modem, Bluetooth, VPN, split-DNS, and stacked network setups are not supported.

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
ctest --test-dir build --output-on-failure
qmllint-qt6 package/contents/config/config.qml package/contents/ui/configGeneral.qml package/contents/ui/main.qml
```

## Install

```bash
sudo cmake --install build
systemctl --user restart plasma-plasmashell.service
```

Add **NextDNS Control** to the Plasma system tray. Open its configuration page and enter a six-character NextDNS profile ID.

## Recovery

The helper delegates activation to `nextdns start` and `nextdns activate`. Native restoration uses `nextdns deactivate`, `resolvectl revert`, NetworkManager connection reactivation, and `nextdns stop`.

If an operation fails, verify the real state before retrying:

```bash
nextdns status
nextdns config
ip -o route get 1.1.1.1
resolvectl status
```

## Uninstall

```bash
sudo cmake --build build --target uninstall
systemctl --user restart plasma-plasmashell.service
```

## License

See `LICENSE`.
