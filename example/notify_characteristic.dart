// example/notify_characteristic.dart
// Demonstrates zero-copy characteristic notification with bluez_native_comms.
// Mirrors jwinarske/flutter_reactive_ble Linux BLE pattern.

import 'package:bluez_native_comms/bluez_native_comms.dart';

const _heartRateServiceUuid = '0000180d-0000-1000-8000-00805f9b34fb';
const _heartRateMeasurementUuid = '00002a37-0000-1000-8000-00805f9b34fb';

Future<void> main(List<String> args) async {
  if (args.isEmpty) {
    print(
        'Usage: dart run example/notify_characteristic.dart <device_address>');
    return;
  }

  final client = BlueZClient();
  await client.connect();

  final adapter = client.adapters.first;

  if (!adapter.powered) {
    print('Powering on adapter...');
    await adapter.setPowered(true);
    await Future<void>.delayed(const Duration(milliseconds: 500));
  }

  print('Scanning...');
  await adapter.startDiscovery();

  BlueZDevice? target;
  await for (final device in client.deviceAdded) {
    if (device.address.toUpperCase() == args[0].toUpperCase()) {
      target = device;
      break;
    }
  }
  await adapter.stopDiscovery();

  if (target == null) {
    print('Device not found: ${args[0]}');
    await client.close();
    return;
  }

  print('Connecting to ${target.address}...');

  const maxRetries = 3;
  for (var attempt = 1; attempt <= maxRetries; attempt++) {
    try {
      await target.connect();
      break;
    } on BlueZOperationException catch (e) {
      print('Attempt $attempt failed: $e');
      if (attempt == maxRetries) {
        print('Giving up after $maxRetries attempts.');
        await client.close();
        return;
      }
      await Future<void>.delayed(const Duration(seconds: 1));
    }
  }

  await target.waitForServicesResolved();
  print('Connected. ${target.gattServices.length} services resolved.');

  final hrService = target.gattServices
      .where((s) => s.uuid == const BlueZUUID(_heartRateServiceUuid))
      .firstOrNull;

  if (hrService == null) {
    print('Heart Rate service not found');
    await target.disconnect();
    await client.close();
    return;
  }

  final hrChar = hrService.characteristics
      .where((c) => c.uuid == const BlueZUUID(_heartRateMeasurementUuid))
      .firstOrNull;

  if (hrChar == null) {
    print('Heart Rate Measurement characteristic not found');
    await target.disconnect();
    await client.close();
    return;
  }

  print('Starting notifications on ${hrChar.uuid}...');
  await hrChar.startNotify();

  // Zero-copy Stream<List<int>> — bytes arrive directly from
  // kExternalTypedData posted by the sdbus-cpp event loop thread.
  var count = 0;
  await for (final bytes in hrChar.value) {
    final flags = bytes[0];
    final hrValue = (flags & 0x01) == 0
        ? bytes[1] // 8-bit HR value
        : (bytes[2] << 8) | bytes[1]; // 16-bit HR value
    print('HR: $hrValue bpm  (raw: $bytes)');
    if (++count >= 10) break;
  }

  await hrChar.stopNotify();
  await target.disconnect();
  await client.close();
  print('Done.');
}
