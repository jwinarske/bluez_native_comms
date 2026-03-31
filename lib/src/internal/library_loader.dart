// library_loader.dart — DynamicLibrary.open() resolution for libbluez_nc.so.

import 'dart:ffi';
import 'dart:io';

import 'package:path/path.dart' as p;

DynamicLibrary loadBluezNc() {
  // 1. Environment variable override.
  final envPath = Platform.environment['BLUEZ_NC_LIB'];
  if (envPath != null && envPath.isNotEmpty) {
    return DynamicLibrary.open(envPath);
  }

  // 2. Next to the running executable.
  final exeDir = p.dirname(Platform.resolvedExecutable);
  final candidates = [
    p.join(exeDir, 'libbluez_nc.so'),
    p.join(exeDir, 'lib', 'libbluez_nc.so'),
  ];

  for (final path in candidates) {
    if (File(path).existsSync()) {
      return DynamicLibrary.open(path);
    }
  }

  // 3. System library path fallback.
  return DynamicLibrary.open('libbluez_nc.so');
}
