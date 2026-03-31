// bluez_gatt_service.dart

import 'bluez_gatt_characteristic.dart';
import 'bluez_uuid.dart';
import 'ffi/types.dart';

/// A GATT service on a remote Bluetooth device.
class BlueZGattService {
  final BlueZGattServiceProps _props;
  final _characteristics = <String, BlueZGattCharacteristic>{};

  /// @nodoc — internal constructor, not part of public API.
  BlueZGattService.internal(this._props);

  /// D-Bus object path of this service.
  String get objectPath => _props.objectPath;

  /// Object path of the parent device.
  String get devicePath => _props.devicePath;

  /// Service UUID.
  BlueZUUID get uuid => BlueZUUID(_props.uuid);

  /// Whether this is a primary service.
  bool get primary => _props.primary;

  /// ATT handle.
  int get handle => _props.handle;

  /// Characteristics belonging to this service.
  List<BlueZGattCharacteristic> get characteristics =>
      List.unmodifiable(_characteristics.values);

  /// @nodoc
  void addCharacteristic(BlueZGattCharacteristic c) =>
      _characteristics[c.objectPath] = c;
}
