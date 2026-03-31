// gatt_bridge.h — GATT Characteristic1 and Descriptor1 D-Bus proxy wrappers.
//
// ReadValue/WriteValue/StartNotify/StopNotify post results to a per-call
// result_port. StartNotify additionally wires PropertiesChanged on the
// characteristic to the ObjectManager's events_port for ongoing Value
// notifications.

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bluez_types.h"
#include "dart_api_dl.h"
#include "object_manager.h"

class GattCharBridge {
 public:
  GattCharBridge(sdbus::IConnection& conn,
                 std::string char_path,
                 ObjectManager& obj_mgr);

  // Read the characteristic value.
  // Posts 0x10 BlueZValueResult on success, 0x20 BlueZError on failure.
  void read_value(Dart_Port_DL result_port);

  // Write bytes to the characteristic.
  // with_response selects Write (true) vs WriteWithoutResponse (false).
  // Posts 0xFF on success, 0x20 BlueZError on failure.
  void write_value(const uint8_t* data,
                   int32_t len,
                   bool with_response,
                   Dart_Port_DL result_port);

  // Subscribe to characteristic notifications.
  // Calls StartNotify on the characteristic, then wires PropertiesChanged
  // through ObjectManager so Value changes post to events_port as 0x03.
  // Posts 0xFF on success, 0x20 BlueZError on failure.
  void start_notify(Dart_Port_DL result_port);

  // Unsubscribe from characteristic notifications.
  // Posts 0xFF on success, 0x20 BlueZError on failure.
  void stop_notify(Dart_Port_DL result_port);

  [[nodiscard]] const std::string& object_path() const { return char_path_; }

 private:
  static constexpr auto kBluezService = "org.bluez";
  static constexpr auto kGattCharIface = "org.bluez.GattCharacteristic1";

  sdbus::IConnection& conn_;
  std::string char_path_;
  ObjectManager& obj_mgr_;
  std::unique_ptr<sdbus::IProxy> proxy_;

  static void post_success(Dart_Port_DL result_port);
  static void post_value_result(Dart_Port_DL result_port,
                                const std::string& object_path,
                                const std::vector<uint8_t>& value);
  static void post_error(Dart_Port_DL result_port,
                         const std::string& object_path,
                         const std::string& error_name,
                         const std::string& error_message);
};

class GattDescBridge {
 public:
  GattDescBridge(sdbus::IConnection& conn, std::string desc_path);

  // Read the descriptor value.
  // Posts 0x11 BlueZDescValue on success, 0x20 BlueZError on failure.
  void read_value(Dart_Port_DL result_port);

  // Write bytes to the descriptor.
  // Posts 0xFF on success, 0x20 BlueZError on failure.
  void write_value(const uint8_t* data, int32_t len, Dart_Port_DL result_port);

  [[nodiscard]] const std::string& object_path() const { return desc_path_; }

 private:
  static constexpr auto kBluezService = "org.bluez";
  static constexpr auto kGattDescIface = "org.bluez.GattDescriptor1";

  sdbus::IConnection& conn_;
  std::string desc_path_;
  std::unique_ptr<sdbus::IProxy> proxy_;

  static void post_success(Dart_Port_DL result_port);
  static void post_value_result(Dart_Port_DL result_port,
                                const std::string& object_path,
                                const std::vector<uint8_t>& value);
  static void post_error(Dart_Port_DL result_port,
                         const std::string& object_path,
                         const std::string& error_name,
                         const std::string& error_message);
};
