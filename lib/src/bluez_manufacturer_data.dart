/// Manufacturer-specific data from a BLE advertisement.
class BlueZManufacturerData {
  /// Bluetooth SIG assigned company identifier.
  final int companyId;

  /// Raw manufacturer data bytes.
  final List<int> data;

  const BlueZManufacturerData(this.companyId, this.data);

  @override
  String toString() =>
      'BlueZManufacturerData(0x${companyId.toRadixString(16)}, $data)';
}
