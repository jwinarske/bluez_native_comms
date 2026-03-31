# bluez_native_comms

High-performance BlueZ Bluetooth client for Linux using native_comms and
sdbus-cpp. Zero-copy characteristic notifications via `Dart_PostCObject_DL`.
Drop-in API replacement for `canonical/bluez.dart` with lower latency.

## Features

- Full BlueZ D-Bus API surface: adapters, devices, GATT services,
  characteristics, and descriptors
- Zero-copy characteristic notifications — bytes arrive directly from the
  sdbus-cpp event loop thread via `kExternalTypedData`
- API compatible with [canonical/bluez.dart](https://github.com/canonical/bluez.dart)
  for straightforward migration
- Discovery filter support (transport, RSSI threshold, UUIDs)

## Platform Support

| Platform | Supported |
|----------|-----------|
| Linux (BlueZ >= 5.50) | Yes |
| macOS | No |
| Windows | No |

## Getting Started

```dart
import 'package:bluez_native_comms/bluez_native_comms.dart';

final client = BlueZClient();
await client.connect();

for (final adapter in client.adapters) {
  print('${adapter.address} (${adapter.name})');
  await adapter.startDiscovery();
}

client.deviceAdded.listen((device) {
  print('Found: ${device.address}  RSSI: ${device.rssi}');
});
```

## Building the Native Library

The native shared library must be built before running Dart code:

```bash
cmake -B build native/ -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Set the `BLUEZ_NC_LIB` environment variable to the path of the built
`libbluez_nc.so`, or place it where `DynamicLibrary.open()` can find it.

## License

Apache 2.0 — see [LICENSE](LICENSE).
