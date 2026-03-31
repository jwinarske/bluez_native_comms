/// Bluetooth UUID wrapper matching canonical/bluez.dart API.
class BlueZUUID {
  final String value;

  const BlueZUUID(this.value);

  @override
  bool operator ==(Object other) =>
      other is BlueZUUID &&
      value.toLowerCase() == other.value.toLowerCase();

  @override
  int get hashCode => value.toLowerCase().hashCode;

  @override
  String toString() => value;
}
