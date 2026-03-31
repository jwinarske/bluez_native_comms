/// BlueZ enumerations.
library;

/// GATT characteristic flags as reported by BlueZ.
enum BlueZGattCharacteristicFlag {
  broadcast,
  read,
  writeWithoutResponse,
  write,
  notify,
  indicate,
  authenticatedSignedWrites,
  extendedProperties,
  reliableWrite,
  writableAuxiliaries,
  encryptRead,
  encryptWrite,
  encryptAuthenticatedRead,
  encryptAuthenticatedWrite,
  secureRead,
  secureWrite,
  authorize;

  static BlueZGattCharacteristicFlag fromString(String s) {
    return switch (s) {
      'broadcast' => broadcast,
      'read' => read,
      'write-without-response' => writeWithoutResponse,
      'write' => write,
      'notify' => notify,
      'indicate' => indicate,
      'authenticated-signed-writes' => authenticatedSignedWrites,
      'extended-properties' => extendedProperties,
      'reliable-write' => reliableWrite,
      'writable-auxiliaries' => writableAuxiliaries,
      'encrypt-read' => encryptRead,
      'encrypt-write' => encryptWrite,
      'encrypt-authenticated-read' => encryptAuthenticatedRead,
      'encrypt-authenticated-write' => encryptAuthenticatedWrite,
      'secure-read' => secureRead,
      'secure-write' => secureWrite,
      'authorize' => authorize,
      _ => throw ArgumentError('Unknown characteristic flag: $s'),
    };
  }
}

/// Discovery transport filter.
enum BlueZDiscoveryTransport {
  auto_('auto'),
  bredr('bredr'),
  le('le');

  final String value;
  const BlueZDiscoveryTransport(this.value);
}
