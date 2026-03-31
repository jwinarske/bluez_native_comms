// example/read_descriptor.dart — read GATT descriptors for a characteristic.

import 'package:bluez_native_comms/bluez_native_comms.dart';

import 'example_utils.dart';

Future<void> main(List<String> args) async {
  if (args.length < 2) {
    print('Usage: dart run example/read_descriptor.dart '
        '<device_address> <characteristic_uuid> [--timeout <seconds>]');
    print('');
    print('Reads all descriptors on the specified characteristic.');
    return;
  }

  final charUuid = BlueZUUID(args[1]);
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
    print('Available characteristics:');
    for (final c in target.gattCharacteristics) {
      print('  ${c.uuid}  flags: ${c.flags}');
    }
    await target.disconnect();
    await client.close();
    return;
  }

  print('Characteristic: ${char.uuid}');
  print('Flags: ${char.flags}');
  print('Descriptors: ${char.descriptors.length}');

  for (final desc in char.descriptors) {
    print('');
    print('  Descriptor: ${desc.uuid}');
    try {
      final value = await desc.readValue();
      final hex =
          value.map((b) => b.toRadixString(16).padLeft(2, '0')).join(' ');
      print('  Value: $value');
      print('  Hex:   $hex');
    } on BlueZOperationException catch (e) {
      print('  Read failed: $e');
    }
  }

  await target.disconnect();
  await client.close();
  print('\nDone.');
}
