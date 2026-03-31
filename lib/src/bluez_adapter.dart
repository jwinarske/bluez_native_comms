// bluez_adapter.dart

import 'bluez_uuid.dart';
import 'enums.dart';
import 'ffi/bindings.dart';
import 'ffi/types.dart';

/// A Bluetooth adapter (HCI controller) managed by BlueZ.
class BlueZAdapter {
  final Object _clientHandle;
  BlueZAdapterProps _props;

  /// @nodoc — internal constructor, not part of public API.
  BlueZAdapter.internal(this._clientHandle, this._props);

  /// D-Bus object path, e.g. `/org/bluez/hci0`.
  String get objectPath => _props.objectPath;

  /// Bluetooth address of the adapter.
  String get address => _props.address;

  /// Adapter name (kernel).
  String get name => _props.name;

  /// User-friendly alias.
  String get alias => _props.alias;

  /// Whether the adapter is powered on.
  bool get powered => _props.powered;

  /// Whether the adapter is discoverable.
  bool get discoverable => _props.discoverable;

  /// Whether the adapter is pairable.
  bool get pairable => _props.pairable;

  /// Whether the adapter is currently scanning.
  bool get discovering => _props.discovering;

  /// Discoverable timeout in seconds.
  int get discoverableTimeout => _props.discoverableTimeout;

  /// UUIDs of supported profiles.
  List<BlueZUUID> get uuids => _props.uuids.map(BlueZUUID.new).toList();

  /// Power the adapter on or off.
  Future<void> setPowered(bool value) async {
    BlueZBindings.adapterSetPropertyBool(
        _clientHandle, objectPath, 'Powered', value);
  }

  /// Start scanning for nearby Bluetooth devices.
  Future<void> startDiscovery() async {
    BlueZBindings.adapterStartDiscovery(_clientHandle, objectPath);
  }

  /// Stop scanning.
  Future<void> stopDiscovery() async {
    BlueZBindings.adapterStopDiscovery(_clientHandle, objectPath);
  }

  /// Set discovery filter (transport, RSSI threshold, UUIDs).
  Future<void> setDiscoveryFilter({
    BlueZDiscoveryTransport? transport,
    int? rssiThreshold,
    List<String>? uuids,
  }) async {
    BlueZBindings.adapterSetDiscoveryFilter(
      _clientHandle,
      objectPath,
      transport?.value,
      rssiThreshold,
      uuids,
    );
  }

  /// Remove a discovered device from BlueZ's cache.
  Future<void> removeDevice(String devicePath) async {
    BlueZBindings.adapterRemoveDevice(_clientHandle, objectPath, devicePath);
  }

  void updateProps(BlueZAdapterProps props) {
    _props = props;
  }
}
