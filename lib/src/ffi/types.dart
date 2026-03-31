// types.dart — Dart-side struct mirrors for glaze-decoded BlueZ payloads.
// These match the C++ structs in native/include/bluez_types.h.

/// Manufacturer data entry (companyId + raw bytes).
class ManufacturerDataEntry {
  final int companyId;
  final List<int> data;
  const ManufacturerDataEntry(this.companyId, this.data);
}

/// Adapter properties from BlueZ.
class BlueZAdapterProps {
  final String objectPath;
  final String address;
  final String name;
  final String alias;
  final bool powered;
  final bool discoverable;
  final bool pairable;
  final bool discovering;
  final int discoverableTimeout;
  final List<String> uuids;

  const BlueZAdapterProps({
    required this.objectPath,
    this.address = '',
    this.name = '',
    this.alias = '',
    this.powered = false,
    this.discoverable = false,
    this.pairable = false,
    this.discovering = false,
    this.discoverableTimeout = 0,
    this.uuids = const [],
  });
}

/// Device properties from BlueZ.
class BlueZDeviceProps {
  final String objectPath;
  final String adapterPath;
  final String address;
  final String addressType;
  final String name;
  final String alias;
  final int rssi;
  final int txPower;
  final int appearance;
  final int deviceClass;
  final bool paired;
  final bool trusted;
  final bool blocked;
  final bool connected;
  final bool servicesResolved;
  final bool legacyPairing;
  final List<String> uuids;
  final List<ManufacturerDataEntry> manufacturerData;
  final Map<String, List<int>> serviceData;

  const BlueZDeviceProps({
    required this.objectPath,
    this.adapterPath = '',
    this.address = '',
    this.addressType = '',
    this.name = '',
    this.alias = '',
    this.rssi = 0,
    this.txPower = 0,
    this.appearance = 0,
    this.deviceClass = 0,
    this.paired = false,
    this.trusted = false,
    this.blocked = false,
    this.connected = false,
    this.servicesResolved = false,
    this.legacyPairing = false,
    this.uuids = const [],
    this.manufacturerData = const [],
    this.serviceData = const {},
  });
}

/// GATT characteristic properties.
class BlueZGattCharProps {
  final String objectPath;
  final String servicePath;
  final String uuid;
  final List<int> value;
  final bool notifying;
  final bool writeAcquired;
  final bool notifyAcquired;
  final int handle;
  final int mtu;
  final List<String> flags;

  const BlueZGattCharProps({
    required this.objectPath,
    this.servicePath = '',
    this.uuid = '',
    this.value = const [],
    this.notifying = false,
    this.writeAcquired = false,
    this.notifyAcquired = false,
    this.handle = 0,
    this.mtu = 0,
    this.flags = const [],
  });
}

/// GATT service properties.
class BlueZGattServiceProps {
  final String objectPath;
  final String devicePath;
  final String uuid;
  final bool primary;
  final int handle;

  const BlueZGattServiceProps({
    required this.objectPath,
    this.devicePath = '',
    this.uuid = '',
    this.primary = false,
    this.handle = 0,
  });
}

/// GATT descriptor properties.
class BlueZGattDescProps {
  final String objectPath;
  final String charPath;
  final String uuid;
  final List<int> value;
  final int handle;

  const BlueZGattDescProps({
    required this.objectPath,
    this.charPath = '',
    this.uuid = '',
    this.value = const [],
    this.handle = 0,
  });
}

/// Value result from ReadValue / StartNotify notification.
class BlueZValueResult {
  final String objectPath;
  final List<int> value;
  const BlueZValueResult({required this.objectPath, required this.value});
}

/// D-Bus error from a failed method call.
class BlueZError {
  final String objectPath;
  final String name;
  final String message;
  const BlueZError({
    required this.objectPath,
    required this.name,
    required this.message,
  });
}
