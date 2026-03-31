import 'dart:async';

import 'package:bluez_native_comms/bluez_native_comms.dart';
import 'package:flutter/material.dart';

import 'device_screen.dart';

class ScannerScreen extends StatefulWidget {
  const ScannerScreen({super.key});

  @override
  State<ScannerScreen> createState() => _ScannerScreenState();
}

class _ScannerScreenState extends State<ScannerScreen> {
  final _client = BlueZClient();
  final _devices = <String, BlueZDevice>{};
  StreamSubscription<BlueZDevice>? _sub;
  bool _scanning = false;
  bool _connected = false;
  String? _error;

  @override
  void initState() {
    super.initState();
    _init();
  }

  Future<void> _init() async {
    try {
      await _client.connect();
      if (!mounted) return;
      setState(() => _connected = true);
      _sub = _client.deviceAdded.listen((device) {
        setState(() => _devices[device.address] = device);
      });
    } catch (e) {
      if (!mounted) return;
      setState(() => _error = e.toString());
    }
  }

  Future<void> _toggleScan() async {
    if (!_connected || _client.adapters.isEmpty) return;
    final adapter = _client.adapters.first;
    if (_scanning) {
      await adapter.stopDiscovery();
    } else {
      _devices.clear();
      await adapter.startDiscovery();
    }
    setState(() => _scanning = !_scanning);
  }

  @override
  void dispose() {
    _sub?.cancel();
    _client.close();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    if (_error != null) {
      return Scaffold(
        appBar: AppBar(title: const Text('BLE Scanner')),
        body: Center(
          child: Padding(
            padding: const EdgeInsets.all(32),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                const Icon(Icons.error_outline, size: 48, color: Colors.red),
                const SizedBox(height: 16),
                Text(
                  'Failed to initialize BlueZ',
                  style: Theme.of(context).textTheme.titleLarge,
                ),
                const SizedBox(height: 8),
                Text(
                  _error!,
                  textAlign: TextAlign.center,
                  style: Theme.of(context).textTheme.bodySmall,
                ),
                const SizedBox(height: 16),
                const Text(
                  'Make sure libbluez_nc.so is built and either '
                  'BLUEZ_NC_LIB is set or the library is on the '
                  'system library path.',
                  textAlign: TextAlign.center,
                ),
              ],
            ),
          ),
        ),
      );
    }

    final sorted = _devices.values.toList()
      ..sort((a, b) => b.rssi.compareTo(a.rssi));

    return Scaffold(
      appBar: AppBar(
        title: const Text('BLE Scanner'),
        actions: [
          IconButton(
            icon: Icon(_scanning ? Icons.stop : Icons.bluetooth_searching),
            onPressed: _connected ? _toggleScan : null,
          ),
        ],
      ),
      body: sorted.isEmpty
          ? const Center(child: Text('No devices found. Tap scan to start.'))
          : ListView.builder(
              itemCount: sorted.length,
              itemBuilder: (context, index) {
                final device = sorted[index];
                final name = device.name.isNotEmpty ? device.name : '(unknown)';
                return ListTile(
                  title: Text(name),
                  subtitle: Text(device.address),
                  trailing: Text('${device.rssi} dBm'),
                  onTap: () => Navigator.push(
                    context,
                    MaterialPageRoute<void>(
                      builder: (_) =>
                          DeviceScreen(client: _client, device: device),
                    ),
                  ),
                );
              },
            ),
    );
  }
}
