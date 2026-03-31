// example/write_characteristic.dart — write a value to a GATT characteristic.

import 'dart:typed_data';

import 'package:bluez_native_comms/bluez_native_comms.dart';

import 'example_utils.dart';

Future<void> main(List<String> args) async {
  if (args.length < 3) {
    print('Usage: dart run example/write_characteristic.dart '
        '<device_address> <characteristic_uuid> <hex_bytes> '
        '[--no-response] [--timeout <seconds>]');
    print('');
    print('  hex_bytes: space-separated hex values, e.g. "01 ff 00"');
    return;
  }

  final deviceAddr = args[0];
  final charUuid = BlueZUUID(args[1]);
  final withResponse = !args.contains('--no-response');
  final timeout = parseScanTimeout(args);

  // Parse hex bytes from remaining args (skip flags).
  final hexArgs = args.skip(2).where((a) => !a.startsWith('--')).where((a) {
    // Skip the value after --timeout.
    final idx = args.indexOf('--timeout');
    return idx == -1 || args.indexOf(a) != idx + 1;
  });
  final data =
      Uint8List.fromList(hexArgs.map((h) => int.parse(h, radix: 16)).toList());

  final client = BlueZClient();
  await client.connect();

  final adapter = client.adapters.first;

  if (!adapter.powered) {
    print('Powering on adapter...');
    await adapter.setPowered(true);
    await Future<void>.delayed(const Duration(milliseconds: 500));
  }

  final target =
      await findDevice(client, adapter, deviceAddr, timeout: timeout);
  if (target == null) {
    await client.close();
    return;
  }

  print('Connecting...');

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

  final char =
      target.gattCharacteristics.where((c) => c.uuid == charUuid).firstOrNull;

  if (char == null) {
    print('Characteristic $charUuid not found.');
    await target.disconnect();
    await client.close();
    return;
  }

  final mode = withResponse ? 'write' : 'write-without-response';
  final hex = data.map((b) => b.toRadixString(16).padLeft(2, '0')).join(' ');
  print('Writing [$hex] to ${char.uuid} ($mode)...');

  try {
    await char.writeValue(data, withResponse: withResponse);
    print('Write succeeded.');
  } on BlueZOperationException catch (e) {
    print('Write failed: $e');
  }

  // Read back to verify if the characteristic supports read.
  if (char.flags.contains(BlueZGattCharacteristicFlag.read)) {
    final readBack = await char.readValue();
    final readHex =
        readBack.map((b) => b.toRadixString(16).padLeft(2, '0')).join(' ');
    print('Read back: [$readHex]');
  }

  await target.disconnect();
  await client.close();
  print('Done.');
}
