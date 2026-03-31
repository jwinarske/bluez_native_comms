import 'package:bluez_native_comms/src/bluez_uuid.dart';
import 'package:bluez_native_comms/src/enums.dart';
import 'package:bluez_native_comms/src/exceptions.dart';
import 'package:test/test.dart';

void main() {
  group('BlueZUUID', () {
    test('equality is case-insensitive', () {
      const a = BlueZUUID('0000180D-0000-1000-8000-00805F9B34FB');
      const b = BlueZUUID('0000180d-0000-1000-8000-00805f9b34fb');
      expect(a, equals(b));
      expect(a.hashCode, equals(b.hashCode));
    });

    test('different UUIDs are not equal', () {
      const a = BlueZUUID('0000180d-0000-1000-8000-00805f9b34fb');
      const b = BlueZUUID('00002a37-0000-1000-8000-00805f9b34fb');
      expect(a, isNot(equals(b)));
    });

    test('toString returns the value', () {
      const uuid = BlueZUUID('0000180d-0000-1000-8000-00805f9b34fb');
      expect(uuid.toString(), '0000180d-0000-1000-8000-00805f9b34fb');
    });
  });

  group('BlueZGattCharacteristicFlag', () {
    test('fromString parses known flags', () {
      expect(BlueZGattCharacteristicFlag.fromString('read'),
          BlueZGattCharacteristicFlag.read);
      expect(BlueZGattCharacteristicFlag.fromString('write'),
          BlueZGattCharacteristicFlag.write);
      expect(BlueZGattCharacteristicFlag.fromString('notify'),
          BlueZGattCharacteristicFlag.notify);
      expect(
          BlueZGattCharacteristicFlag.fromString('write-without-response'),
          BlueZGattCharacteristicFlag.writeWithoutResponse);
    });

    test('fromString throws on unknown flag', () {
      expect(
          () => BlueZGattCharacteristicFlag.fromString('unknown'),
          throwsA(isA<ArgumentError>()));
    });
  });

  group('BlueZDiscoveryTransport', () {
    test('enum values', () {
      expect(BlueZDiscoveryTransport.le.value, 'le');
      expect(BlueZDiscoveryTransport.bredr.value, 'bredr');
      expect(BlueZDiscoveryTransport.auto_.value, 'auto');
    });
  });

  group('BlueZException', () {
    test('base exception', () {
      const e = BlueZException('test');
      expect(e.message, 'test');
      expect(e.toString(), contains('test'));
    });

    test('service unavailable', () {
      const e = BlueZServiceUnavailableException();
      expect(e, isA<BlueZException>());
    });

    test('operation exception', () {
      const e = BlueZOperationException('msg',
          name: 'org.bluez.Error.Failed');
      expect(e.name, 'org.bluez.Error.Failed');
      expect(e.message, 'msg');
      expect(e.toString(), contains('org.bluez.Error.Failed'));
    });
  });
}
