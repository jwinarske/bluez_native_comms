// adapter_bridge.cpp — Adapter1 D-Bus proxy implementation.

#include "adapter_bridge.h"

#include <utility>

AdapterBridge::AdapterBridge(sdbus::IConnection& conn,
                             std::string adapter_path)
    : conn_(conn),
      adapter_path_(std::move(adapter_path)),
      proxy_(sdbus::createProxy(conn_,
                                sdbus::ServiceName{kBluezService},
                                sdbus::ObjectPath{adapter_path_})) {}

// ── Discovery ───────────────────────────────────────────────────────────────

void AdapterBridge::start_discovery() {
  proxy_->callMethod("StartDiscovery").onInterface(kAdapterIface);
}

void AdapterBridge::stop_discovery() {
  proxy_->callMethod("StopDiscovery").onInterface(kAdapterIface);
}

void AdapterBridge::set_discovery_filter(
    const std::map<std::string, sdbus::Variant>& filter) {
  proxy_->callMethod("SetDiscoveryFilter")
      .onInterface(kAdapterIface)
      .withArguments(filter);
}

void AdapterBridge::remove_device(const std::string& device_path) {
  proxy_->callMethod("RemoveDevice")
      .onInterface(kAdapterIface)
      .withArguments(sdbus::ObjectPath{device_path});
}

// ── Properties ──────────────────────────────────────────────────────────────

void AdapterBridge::set_powered(bool value) {
  set_property_bool("Powered", value);
}

void AdapterBridge::set_discoverable(bool value) {
  set_property_bool("Discoverable", value);
}

void AdapterBridge::set_pairable(bool value) {
  set_property_bool("Pairable", value);
}

void AdapterBridge::set_alias(const std::string& value) {
  set_property_string("Alias", value);
}

void AdapterBridge::set_discoverable_timeout(uint32_t value) {
  set_property_uint32("DiscoverableTimeout", value);
}

void AdapterBridge::set_pairable_timeout(uint32_t value) {
  set_property_uint32("PairableTimeout", value);
}

void AdapterBridge::set_property_bool(const std::string& name, bool value) {
  proxy_->setProperty(name).onInterface(kAdapterIface).toValue(value);
}

void AdapterBridge::set_property_string(const std::string& name,
                                        const std::string& value) {
  proxy_->setProperty(name).onInterface(kAdapterIface).toValue(value);
}

void AdapterBridge::set_property_uint32(const std::string& name,
                                        uint32_t value) {
  proxy_->setProperty(name).onInterface(kAdapterIface).toValue(value);
}
