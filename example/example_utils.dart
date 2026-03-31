// example/example_utils.dart — shared helpers for CLI examples.

import 'dart:async';

import 'package:bluez_native_comms/bluez_native_comms.dart';

/// Default scan timeout in seconds.
const int kDefaultTimeoutSeconds = 15;

/// Parse `--timeout <seconds>` from [args], returning the Duration.
Duration parseScanTimeout(List<String> args) {
  final idx = args.indexOf('--timeout');
  if (idx != -1 && idx + 1 < args.length) {
    final seconds = int.tryParse(args[idx + 1]);
    if (seconds != null && seconds > 0) return Duration(seconds: seconds);
  }
  return const Duration(seconds: kDefaultTimeoutSeconds);
}

/// Find a device by [addr], checking known devices first then scanning.
///
/// Returns `null` if the device is not found within [timeout].
Future<BlueZDevice?> findDevice(
  BlueZClient client,
  BlueZAdapter adapter,
  String addr, {
  required Duration timeout,
}) async {
  addr = addr.toUpperCase();

  // Check already-known devices.
  final existing =
      client.devices.where((d) => d.address.toUpperCase() == addr).firstOrNull;
  if (existing != null) {
    print('Device $addr already known to BlueZ.');
    return existing;
  }

  // Fall back to scanning.
  print('Scanning for $addr...');
  await adapter.startDiscovery();

  try {
    final device = await client.deviceAdded
        .where((d) => d.address.toUpperCase() == addr)
        .first
        .timeout(timeout);
    await adapter.stopDiscovery();
    return device;
  } on TimeoutException {
    print('Device $addr not found within ${timeout.inSeconds} seconds.');
    await adapter.stopDiscovery();
    return null;
  }
}
