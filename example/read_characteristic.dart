// example/read_characteristic.dart — read a GATT characteristic value.

import 'package:bluez_native_comms/bluez_native_comms.dart';

import 'example_utils.dart';

Future<void> main(List<String> args) async {
  if (args.length < 2) {
    print('Usage: dart run example/read_characteristic.dart '
        '<device_address> <characteristic_uuid> [--timeout <seconds>]');
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

  // Find the characteristic across all services.
  final char =
      target.gattCharacteristics.where((c) => c.uuid == charUuid).firstOrNull;

  if (char == null) {
    print('Characteristic $charUuid not found.');
    await target.disconnect();
    await client.close();
    return;
  }

  final value = await char.readValue();
  print('Value: $value');
  print(
      'Hex: ${value.map((b) => b.toRadixString(16).padLeft(2, '0')).join(' ')}');

  await target.disconnect();
  await client.close();
  print('Done.');
}
