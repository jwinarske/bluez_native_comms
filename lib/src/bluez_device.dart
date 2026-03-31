// bluez_device.dart

import 'dart:async';
import 'dart:ffi';
import 'dart:isolate';
import 'dart:typed_data';

import 'bluez_gatt_characteristic.dart';
import 'bluez_gatt_descriptor.dart';
import 'bluez_gatt_service.dart';
import 'bluez_manufacturer_data.dart';
import 'bluez_uuid.dart';
import 'exceptions.dart';
import 'ffi/bindings.dart';
import 'ffi/codec.dart';
import 'ffi/types.dart';

/// A Bluetooth device discovered or connected via BlueZ.
class BlueZDevice {
  final Object _clientHandle;
  BlueZDeviceProps _props;

  final _propertiesChangedCtrl = StreamController<List<String>>.broadcast();

  /// Fires with the list of changed property names when any device
  /// property changes (Connected, RSSI, ServicesResolved, etc.)
  Stream<List<String>> get propertiesChanged =>
      _propertiesChangedCtrl.stream;

  final _services = <String, BlueZGattService>{};
  final _characteristics = <String, BlueZGattCharacteristic>{};
  final _descriptors = <String, BlueZGattDescriptor>{};

  /// @nodoc — internal constructor, not part of public API.
  BlueZDevice.internal(this._clientHandle, BlueZDeviceProps props)
      : _props = props;

  /// D-Bus object path.
  String get objectPath => _props.objectPath;

  /// Bluetooth address.
  String get address => _props.address;

  /// Address type: "public" or "random".
  String get addressType => _props.addressType;

  /// Device name as reported by the remote device.
  String get name => _props.name;

  /// User-friendly alias.
  String get alias => _props.alias;

  /// RSSI of the last advertisement (dBm).
  int get rssi => _props.rssi;

  /// Transmit power level (dBm).
  int get txPower => _props.txPower;

  /// GAP appearance value.
  int get appearance => _props.appearance;

  /// Bluetooth class of device.
  int get deviceClass => _props.deviceClass;

  /// Whether the device is paired.
  bool get paired => _props.paired;

  /// Whether the device is trusted.
  bool get trusted => _props.trusted;

  /// Whether the device is blocked.
  bool get blocked => _props.blocked;

  /// Whether the device is currently connected.
  bool get connected => _props.connected;

  /// Whether GATT services have been resolved after connection.
  bool get servicesResolved => _props.servicesResolved;

  /// UUIDs of services advertised by the device.
  List<BlueZUUID> get uuids => _props.uuids.map(BlueZUUID.new).toList();

  /// Manufacturer-specific data from advertisements.
  List<BlueZManufacturerData> get manufacturerData =>
      _props.manufacturerData
          .map((e) => BlueZManufacturerData(e.companyId, e.data))
          .toList();

  /// GATT services discovered after connection.
  List<BlueZGattService> get gattServices =>
      List.unmodifiable(_services.values);

  /// All characteristics across all services.
  Iterable<BlueZGattCharacteristic> get gattCharacteristics =>
      _characteristics.values;

  // ── Operations ────────────────────────────────────────────────────────────

  /// Connect to this device.
  Future<void> connect() async {
    final port = ReceivePort();
    BlueZBindings.deviceConnect(
        _clientHandle, objectPath, port.sendPort.nativePort);
    await _awaitResult(port);
  }

  /// Disconnect from this device.
  Future<void> disconnect() async {
    final port = ReceivePort();
    BlueZBindings.deviceDisconnect(
        _clientHandle, objectPath, port.sendPort.nativePort);
    await _awaitResult(port);
  }

  /// Initiate pairing with this device.
  Future<void> pair() async {
    final port = ReceivePort();
    BlueZBindings.devicePair(
        _clientHandle, objectPath, port.sendPort.nativePort);
    await _awaitResult(port);
  }

  /// Cancel an in-progress pairing.
  Future<void> cancelPairing() async {
    BlueZBindings.deviceCancelPairing(_clientHandle, objectPath);
  }

  /// Wait for ServicesResolved = true after connect().
  Future<void> waitForServicesResolved() async {
    if (servicesResolved) return;
    await propertiesChanged.where((_) => servicesResolved).first;
  }

  // ── Internal ──────────────────────────────────────────────────────────────

  void updateProps(BlueZDeviceProps props) {
    final changed = _diffProps(props);
    _props = props;
    if (changed.isNotEmpty) _propertiesChangedCtrl.add(changed);
  }

  List<String> _diffProps(BlueZDeviceProps next) {
    final changed = <String>[];
    if (_props.connected != next.connected) changed.add('Connected');
    if (_props.rssi != next.rssi) changed.add('RSSI');
    if (_props.paired != next.paired) changed.add('Paired');
    if (_props.servicesResolved != next.servicesResolved) {
      changed.add('ServicesResolved');
    }
    if (_props.name != next.name) changed.add('Name');
    if (_props.trusted != next.trusted) changed.add('Trusted');
    if (_props.blocked != next.blocked) changed.add('Blocked');
    return changed;
  }

  void addService(BlueZGattServiceProps props) =>
      _services[props.objectPath] = BlueZGattService.internal(props);

  void addCharacteristic(BlueZGattCharProps props) {
    final char = BlueZGattCharacteristic.internal(_clientHandle, props);
    _characteristics[props.objectPath] = char;
    _services[props.servicePath]?.addCharacteristic(char);
  }

  void addDescriptor(BlueZGattDescProps props) {
    final desc = BlueZGattDescriptor.internal(_clientHandle, props);
    _descriptors[props.objectPath] = desc;
    _characteristics[props.charPath]?.addDescriptor(desc);
  }

  static Future<void> _awaitResult(ReceivePort port) async {
    final msg = await port.first;
    port.close();
    if (msg is Uint8List && msg[0] == 0x20) {
      final err = GlazeCodec.decode<BlueZError>(msg, 1);
      throw BlueZOperationException(err.message, name: err.name);
    }
  }
}
