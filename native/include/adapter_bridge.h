// adapter_bridge.h — Adapter1 D-Bus proxy wrapper.
//
// Wraps the generated Adapter1_proxy with error handling and
// Dart_PostCObject_DL posting for asynchronous results.

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <map>
#include <string>
#include <vector>

#include "bluez_types.h"
#include "dart_api_dl.h"

class AdapterBridge {
 public:
  AdapterBridge(sdbus::IConnection& conn, std::string adapter_path);

  // ── Discovery ───────────────────────────────────────────────────────────

  void start_discovery();
  void stop_discovery();

  // filter: a{sv} with optional keys: Transport(s), UUIDs(as), RSSI(n)
  void set_discovery_filter(
      const std::map<std::string, sdbus::Variant>& filter);

  // Remove a discovered device.
  void remove_device(const std::string& device_path);

  // ── Properties ──────────────────────────────────────────────────────────

  void set_powered(bool value);
  void set_discoverable(bool value);
  void set_pairable(bool value);
  void set_alias(const std::string& value);
  void set_discoverable_timeout(uint32_t value);
  void set_pairable_timeout(uint32_t value);

  // Generic property setter via D-Bus Properties interface.
  void set_property_bool(const std::string& name, bool value);
  void set_property_string(const std::string& name, const std::string& value);
  void set_property_uint32(const std::string& name, uint32_t value);

  [[nodiscard]] const std::string& object_path() const { return adapter_path_; }

 private:
  static constexpr auto kBluezService = "org.bluez";
  static constexpr auto kAdapterIface = "org.bluez.Adapter1";

  sdbus::IConnection& conn_;
  std::string adapter_path_;
  std::unique_ptr<sdbus::IProxy> proxy_;
};
