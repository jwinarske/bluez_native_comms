// example/scan_devices.dart — scan and print discovered devices.

import 'dart:async';

import 'package:bluez_native_comms/bluez_native_comms.dart';

import 'example_utils.dart';

Future<void> main(List<String> args) async {
  final timeout = parseScanTimeout(args);

  final client = BlueZClient();
  await client.connect();

  final adapter = client.adapters.first;
  print('Adapter: ${adapter.address}  (${adapter.name})');

  if (!adapter.powered) {
    print('Powering on adapter...');
    await adapter.setPowered(true);
    // Give BlueZ a moment to bring the adapter up.
    await Future<void>.delayed(const Duration(milliseconds: 500));
  }

  // Discovery filter: LE only, -80 dBm threshold
  await adapter.setDiscoveryFilter(
    transport: BlueZDiscoveryTransport.le,
    rssiThreshold: -80,
  );

  print('Scanning for ${timeout.inSeconds} seconds...\n');
  await adapter.startDiscovery();

  final sub = client.deviceAdded.listen((device) {
    final name = device.name.isNotEmpty ? device.name : '(unknown)';
    final mfr = device.manufacturerData.isNotEmpty
        ? '  mfr: 0x${device.manufacturerData.first.companyId.toRadixString(16)}'
        : '';
    print('${device.address}  RSSI ${device.rssi} dBm  $name$mfr');
  });

  await Future<void>.delayed(timeout);
  await sub.cancel();
  await adapter.stopDiscovery();
  await client.close();
}
