// example/device_properties.dart — monitor live property changes on a device.

import 'dart:async';

import 'package:bluez_native_comms/bluez_native_comms.dart';

import 'example_utils.dart';

Future<void> main(List<String> args) async {
  if (args.isEmpty) {
    print('Usage: dart run example/device_properties.dart <device_address> '
        '[--timeout <seconds>]');
    print('');
    print('Monitors property changes (RSSI, Connected, etc.) for 30 seconds.');
    return;
  }

  final timeout = parseScanTimeout(args);
  final client = BlueZClient();
  await client.connect();

  final adapter = client.adapters.first;

  if (!adapter.powered) {
    print('Powering on adapter...');
    await adapter.setPowered(true);
    await Future<void>.delayed(const Duration(milliseconds: 500));
  }

  final target = await findDevice(client, adapter, args[0], timeout: timeout);
  if (target == null) {
    await client.close();
    return;
  }

  // Start discovery to receive RSSI updates.
  await adapter.startDiscovery();

  print('Found: ${target.name.isNotEmpty ? target.name : target.address}');
  print('  Address:    ${target.address} (${target.addressType})');
  print('  RSSI:       ${target.rssi} dBm');
  print('  Paired:     ${target.paired}');
  print('  Connected:  ${target.connected}');
  print('  Trusted:    ${target.trusted}');
  print('  UUIDs:      ${target.uuids}');
  print('');
  print('Monitoring property changes for 30 seconds...');

  final dev = target;
  final sub = dev.propertiesChanged.listen((changed) {
    final timestamp = DateTime.now().toIso8601String().substring(11, 23);
    print('[$timestamp] Changed: $changed');
    if (changed.contains('RSSI')) print('  RSSI: ${dev.rssi} dBm');
    if (changed.contains('Connected')) {
      print('  Connected: ${dev.connected}');
    }
    if (changed.contains('Paired')) print('  Paired: ${dev.paired}');
    if (changed.contains('ServicesResolved')) {
      print('  ServicesResolved: ${dev.servicesResolved}');
    }
  });

  await Future<void>.delayed(const Duration(seconds: 30));
  await sub.cancel();
  await adapter.stopDiscovery();
  await client.close();
  print('Done.');
}
