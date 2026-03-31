// example/pair_device.dart — pair with a Bluetooth device.

import 'package:bluez_native_comms/bluez_native_comms.dart';

Future<void> main(List<String> args) async {
  if (args.isEmpty) {
    print('Usage: dart run example/pair_device.dart <device_address>');
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

  print('Found: ${target.name.isNotEmpty ? target.name : target.address}');
  print('Paired: ${target.paired}');

  if (target.paired) {
    print('Already paired.');
    await client.close();
    return;
  }

  print('Pairing...');
  try {
    await target.pair();
    print('Paired: ${target.paired}');
  } on BlueZOperationException catch (e) {
    print('Pairing failed: $e');
  }

  await client.close();
  print('Done.');
}
