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
  StreamSubscription<BlueZDevice>? _deviceSub;
  StreamSubscription<BlueZAdapter>? _adapterSub;
  bool _scanning = false;
  bool _connected = false;
  String? _error;

  BlueZAdapter? get _adapter =>
      _client.adapters.isNotEmpty ? _client.adapters.first : null;

  bool get _powered => _adapter?.powered ?? false;

  @override
  void initState() {
    super.initState();
    _init();
  }

  Future<void> _init() async {
    try {
      await _client.connect();
      if (!mounted) return;
      setState(() {
        _connected = true;
        // Seed with devices already known to BlueZ from previous scans.
        for (final device in _client.devices) {
          _devices[device.address] = device;
        }
      });
      _deviceSub = _client.deviceAdded.listen((device) {
        if (mounted) setState(() => _devices[device.address] = device);
      });
      // Monitor adapter power state changes.
      _adapterSub = _client.adapterChanged.listen((adapter) {
        if (!mounted) return;
        setState(() {
          if (adapter.powered) {
            // Adapter just powered on — seed with known devices.
            for (final device in _client.devices) {
              _devices[device.address] = device;
            }
          } else {
            if (_scanning) _scanning = false;
            _devices.clear();
          }
        });
      });
    } catch (e) {
      if (!mounted) return;
      setState(() => _error = e.toString());
    }
  }

  Future<void> _toggleScan() async {
    final adapter = _adapter;
    if (!_connected || adapter == null) return;

    if (!_powered) {
      await adapter.setPowered(true);
      // Give BlueZ a moment to bring the adapter up.
      await Future<void>.delayed(const Duration(milliseconds: 500));
      if (!mounted) return;
      setState(() {});
    }

    if (_scanning) {
      await adapter.stopDiscovery();
    } else {
      _devices.clear();
      await adapter.startDiscovery();
    }
    if (mounted) setState(() => _scanning = !_scanning);
  }

  @override
  void dispose() {
    _deviceSub?.cancel();
    _adapterSub?.cancel();
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
          // Adapter power indicator.
          if (_connected)
            Icon(
              _powered ? Icons.bluetooth : Icons.bluetooth_disabled,
              color: _powered ? null : Colors.red,
            ),
          const SizedBox(width: 8),
          IconButton(
            icon: Icon(_scanning ? Icons.stop : Icons.bluetooth_searching),
            onPressed: _connected ? _toggleScan : null,
          ),
        ],
      ),
      body: !_powered && _connected
          ? Center(
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  const Icon(Icons.bluetooth_disabled,
                      size: 48, color: Colors.grey),
                  const SizedBox(height: 16),
                  const Text('Bluetooth adapter is powered off.'),
                  const SizedBox(height: 16),
                  ElevatedButton(
                    onPressed: () => _adapter?.setPowered(true),
                    child: const Text('Power On'),
                  ),
                ],
              ),
            )
          : sorted.isEmpty
              ? const Center(
                  child: Text('No devices found. Tap scan to start.'))
              : ListView.builder(
                  itemCount: sorted.length,
                  itemBuilder: (context, index) {
                    final device = sorted[index];
                    final name =
                        device.name.isNotEmpty ? device.name : '(unknown)';
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
