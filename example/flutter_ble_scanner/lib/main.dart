import 'package:flutter/material.dart';

import 'scanner_screen.dart';

void main() {
  runApp(const BLEScannerApp());
}

class BLEScannerApp extends StatelessWidget {
  const BLEScannerApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'BLE Scanner',
      theme: ThemeData(
        colorSchemeSeed: Colors.blue,
        useMaterial3: true,
      ),
      home: const ScannerScreen(),
    );
  }
}
