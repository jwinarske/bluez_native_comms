// example/connect_device.dart — connect to a device by address.

import 'dart:async';

import 'package:bluez_native_comms/bluez_native_comms.dart';

Future<void> main(List<String> args) async {
  if (args.isEmpty) {
    print('Usage: dart run example/connect_device.dart <device_address>');
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

  print('Scanning for ${args[0]}...');
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
    print('Device not found.');
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

  print('Connected: ${target.connected}');

  await target.waitForServicesResolved();
  print('Services resolved: ${target.gattServices.length} services');

  for (final service in target.gattServices) {
    print('  Service: ${service.uuid}  (primary: ${service.primary})');
    for (final char in service.characteristics) {
      print('    Char: ${char.uuid}  flags: ${char.flags}');
    }
  }

  await target.disconnect();
  await client.close();
  print('Done.');
}
