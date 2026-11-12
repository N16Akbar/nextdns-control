# NextDNS Switcher

KDE Plasma 6 widget for switching between a NextDNS profile and the DNS
servers provided by the active network connection.

## Requirements

- KDE Plasma 6
- Qt 6 and KDE Frameworks 6
- NetworkManager
- systemd-resolved

## Build and install

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr

cmake --build build
sudo cmake --install build
```
