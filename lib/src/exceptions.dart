/// BlueZ exception hierarchy.
library;

/// Base exception for all BlueZ operations.
class BlueZException implements Exception {
  final String message;

  const BlueZException(this.message);

  @override
  String toString() => 'BlueZException: $message';
}

/// Thrown when the BlueZ daemon is not available on the system bus.
class BlueZServiceUnavailableException extends BlueZException {
  const BlueZServiceUnavailableException([
    super.message = 'BlueZ service is not available',
  ]);
}

/// Thrown when a D-Bus method call fails.
class BlueZOperationException extends BlueZException {
  /// The D-Bus error name, e.g. `org.bluez.Error.Failed`.
  final String name;

  const BlueZOperationException(super.message, {required this.name});

  @override
  String toString() => 'BlueZOperationException($name): $message';
}
