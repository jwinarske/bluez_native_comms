// bluez_gatt_descriptor.dart

import 'dart:ffi';
import 'dart:isolate';
import 'dart:typed_data';

import 'bluez_uuid.dart';
import 'exceptions.dart';
import 'ffi/bindings.dart';
import 'ffi/codec.dart';
import 'ffi/types.dart';

/// A GATT descriptor on a remote Bluetooth device.
class BlueZGattDescriptor {
  final Object _clientHandle;
  final BlueZGattDescProps _props;

  /// @nodoc — internal constructor, not part of public API.
  BlueZGattDescriptor.internal(this._clientHandle, this._props);

  /// D-Bus object path of this descriptor.
  String get objectPath => _props.objectPath;

  /// Object path of the parent characteristic.
  String get characteristicPath => _props.charPath;

  /// Descriptor UUID.
  BlueZUUID get uuid => BlueZUUID(_props.uuid);

  /// Last known descriptor value.
  List<int> get cachedValue => List.unmodifiable(_props.value);

  /// ATT handle.
  int get handle => _props.handle;

  /// Read the descriptor value from the device.
  Future<List<int>> readValue() async {
    final port = ReceivePort();
    BlueZBindings.descReadValue(
        _clientHandle, objectPath, port.sendPort.nativePort);
    final msg = await port.first as Uint8List;
    port.close();
    if (msg[0] == 0x20) {
      final err = GlazeCodec.decode<BlueZError>(msg, 1);
      throw BlueZOperationException(err.message, name: err.name);
    }
    final result = GlazeCodec.decode<BlueZValueResult>(msg, 1);
    return List.unmodifiable(result.value);
  }

  /// Write [data] to the descriptor.
  Future<void> writeValue(List<int> data) async {
    final bytes = Uint8List.fromList(data);
    final port = ReceivePort();
    BlueZBindings.descWriteValue(
        _clientHandle, objectPath, bytes, port.sendPort.nativePort);
    final msg = await port.first as Uint8List;
    port.close();
    if (msg[0] == 0x20) {
      final err = GlazeCodec.decode<BlueZError>(msg, 1);
      throw BlueZOperationException(err.message, name: err.name);
    }
  }
}
