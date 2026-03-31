// device_bridge.h — Device1 D-Bus proxy wrapper.
//
// Wraps the generated Device1_proxy with error handling and
// Dart_PostCObject_DL posting for asynchronous results.
// Connect/Disconnect/Pair post results to a per-call result_port.

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <string>

#include "bluez_types.h"
#include "dart_api_dl.h"

class DeviceBridge {
 public:
  DeviceBridge(sdbus::IConnection& conn, std::string device_path);

  // ── Async operations (post result to result_port) ───────────────────────

  // Connect to the remote device.
  // Posts 0xFF sentinel on success, 0x20 BlueZError on failure.
  void connect(Dart_Port_DL result_port);

  // Disconnect from the remote device.
  void disconnect(Dart_Port_DL result_port);

  // Initiate pairing.
  void pair(Dart_Port_DL result_port);

  // Cancel an in-progress pairing (fire-and-forget).
  void cancel_pairing();

  // ── Properties ──────────────────────────────────────────────────────────

  void set_trusted(bool value);
  void set_blocked(bool value);
  void set_alias(const std::string& value);

  // Generic property setter via D-Bus Properties interface.
  void set_property_bool(const std::string& name, bool value);
  void set_property_string(const std::string& name, const std::string& value);

  [[nodiscard]] const std::string& object_path() const {
    return device_path_;
  }

 private:
  static constexpr auto kBluezService = "org.bluez";
  static constexpr auto kDeviceIface = "org.bluez.Device1";

  sdbus::IConnection& conn_;
  std::string device_path_;
  std::unique_ptr<sdbus::IProxy> proxy_;

  // Post a success sentinel (0xFF) to result_port.
  static void post_success(Dart_Port_DL result_port);

  // Post a BlueZError (0x20) to result_port.
  static void post_error(Dart_Port_DL result_port,
                         const std::string& object_path,
                         const std::string& error_name,
                         const std::string& error_message);
};
