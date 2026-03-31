import 'package:bluez_native_comms/bluez_native_comms.dart';
import 'package:flutter/material.dart';

import 'characteristic_screen.dart';

class DeviceScreen extends StatefulWidget {
  final BlueZClient client;
  final BlueZDevice device;

  const DeviceScreen({
    super.key,
    required this.client,
    required this.device,
  });

  @override
  State<DeviceScreen> createState() => _DeviceScreenState();
}

class _DeviceScreenState extends State<DeviceScreen> {
  bool _connecting = false;

  Future<void> _connect() async {
    setState(() => _connecting = true);
    try {
      await widget.device.connect();
      await widget.device.waitForServicesResolved();
    } on BlueZOperationException catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Connection failed: ${e.message}')),
        );
      }
    }
    if (mounted) setState(() => _connecting = false);
  }

  Future<void> _disconnect() async {
    await widget.device.disconnect();
    if (mounted) setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    final device = widget.device;
    final services = device.gattServices;

    return Scaffold(
      appBar: AppBar(
        title: Text(device.name.isNotEmpty ? device.name : device.address),
        actions: [
          if (_connecting)
            const Padding(
              padding: EdgeInsets.all(16),
              child: SizedBox(
                width: 20,
                height: 20,
                child: CircularProgressIndicator(strokeWidth: 2),
              ),
            )
          else
            TextButton(
              onPressed: device.connected ? _disconnect : _connect,
              child: Text(device.connected ? 'Disconnect' : 'Connect'),
            ),
        ],
      ),
      body: !device.connected
          ? const Center(child: Text('Not connected. Tap Connect.'))
          : services.isEmpty
              ? const Center(child: Text('No services discovered.'))
              : ListView.builder(
                  itemCount: services.length,
                  itemBuilder: (context, index) {
                    final service = services[index];
                    return ExpansionTile(
                      title: Text(service.uuid.toString()),
                      subtitle: Text(service.primary ? 'Primary' : 'Secondary'),
                      children: service.characteristics.map((char) {
                        return ListTile(
                          title: Text(char.uuid.toString()),
                          subtitle: Text(
                              'Flags: ${char.flags.map((f) => f.name).join(', ')}'),
                          trailing: const Icon(Icons.chevron_right),
                          onTap: () => Navigator.push(
                            context,
                            MaterialPageRoute<void>(
                              builder: (_) =>
                                  CharacteristicScreen(characteristic: char),
                            ),
                          ),
                        );
                      }).toList(),
                    );
                  },
                ),
    );
  }
}
