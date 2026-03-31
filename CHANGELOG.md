## 0.1.0

- Initial release.
- BlueZClient, BlueZAdapter, BlueZDevice APIs matching canonical/bluez.dart.
- Zero-copy characteristic notifications via native_comms Channel B.
- StartNotify / StopNotify via org.bluez.GattCharacteristic1.
- ReadValue / WriteValue for characteristics and descriptors.
- Discovery filter support (transport, RSSI threshold, UUIDs).
- Flutter BLE scanner example (mirrors jwinarske/flutter_reactive_ble pattern).
- CI: Ubuntu 24.04 + Fedora 41, x86_64 + arm64, ASAN, clang-tidy-19.
