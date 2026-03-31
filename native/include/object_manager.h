// object_manager.h — ObjectManager proxy + BlueZ object tree management.
//
// Manages the BlueZ D-Bus object tree by:
//   1. Calling GetManagedObjects() to snapshot the current state
//   2. Subscribing to InterfacesAdded / InterfacesRemoved signals
//   3. Subscribing to PropertiesChanged on adapter/device/characteristic paths
//   4. Posting all changes to Dart via Dart_PostCObject_DL
//
// All sdbus-cpp signal callbacks run on the sdbus event loop thread.
// The object tree maps are protected by a mutex.

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "bluez_types.h"
#include "dart_api_dl.h"

class ObjectManager {
 public:
  using PropertiesMap = std::map<std::string, sdbus::Variant>;
  using InterfacesMap = std::map<std::string, PropertiesMap>;

  ObjectManager(sdbus::IConnection& conn, Dart_Port_DL events_port);
  ~ObjectManager();

  ObjectManager(const ObjectManager&) = delete;
  ObjectManager& operator=(const ObjectManager&) = delete;

  // Snapshot the current BlueZ object tree via GetManagedObjects().
  // Posts initial adapter/device/GATT objects to events_port.
  void get_managed_objects();

  // Subscribe to PropertiesChanged for characteristic notifications.
  // Called after StartNotify() succeeds on a characteristic.
  void subscribe_char_notify(const std::string& char_path);

  // Unsubscribe from PropertiesChanged for a characteristic.
  void unsubscribe_char_notify(const std::string& char_path);

  // Subscribe to PropertiesChanged for a device (Connected, RSSI, etc.)
  void subscribe_device_props(const std::string& device_path);

  // Subscribe to PropertiesChanged for an adapter (Powered, Discovering, etc.)
  void subscribe_adapter_props(const std::string& adapter_path);

  // ── Property extraction from D-Bus Variant maps ─────────────────────────

  static BlueZAdapterProps extract_adapter_props(
      const std::string& object_path,
      const PropertiesMap& props);

  static BlueZDeviceProps extract_device_props(
      const std::string& object_path,
      const PropertiesMap& props);

  static BlueZGattServiceProps extract_service_props(
      const std::string& object_path,
      const PropertiesMap& props);

  static BlueZGattCharProps extract_char_props(
      const std::string& object_path,
      const PropertiesMap& props);

  static BlueZGattDescProps extract_desc_props(
      const std::string& object_path,
      const PropertiesMap& props);

  // Safe property accessors (return default on missing/type-mismatch).
  template <typename T>
  static T get_prop(const PropertiesMap& props,
                    const std::string& key,
                    const T& fallback = {});

 private:
  static constexpr auto kBluezService = "org.bluez";
  static constexpr auto kPropertiesIface =
      "org.freedesktop.DBus.Properties";
  static constexpr auto kObjectManagerIface =
      "org.freedesktop.DBus.ObjectManager";

  static constexpr auto kAdapterIface = "org.bluez.Adapter1";
  static constexpr auto kDeviceIface = "org.bluez.Device1";
  static constexpr auto kGattServiceIface = "org.bluez.GattService1";
  static constexpr auto kGattCharIface = "org.bluez.GattCharacteristic1";
  static constexpr auto kGattDescIface = "org.bluez.GattDescriptor1";

  sdbus::IConnection& conn_;
  Dart_Port_DL events_port_;
  std::unique_ptr<sdbus::IProxy> root_proxy_;

  // PropertiesChanged subscription proxies, keyed by object path.
  std::mutex proxies_mutex_;
  std::map<std::string, std::unique_ptr<sdbus::IProxy>> notify_proxies_;
  std::map<std::string, std::unique_ptr<sdbus::IProxy>> device_proxies_;
  std::map<std::string, std::unique_ptr<sdbus::IProxy>> adapter_proxies_;

  // ── Signal handlers ─────────────────────────────────────────────────────

  void on_interfaces_added(const sdbus::ObjectPath& object_path,
                           const InterfacesMap& interfaces);

  void on_interfaces_removed(const sdbus::ObjectPath& object_path,
                             const std::vector<std::string>& interfaces);

  // ── Dart posting helper ─────────────────────────────────────────────────

  template <typename T>
  void post_glaze(uint8_t discriminator, const T& value);
};
