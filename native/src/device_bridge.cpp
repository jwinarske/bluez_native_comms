// device_bridge.cpp — Device1 D-Bus proxy implementation.

#include "device_bridge.h"

DeviceBridge::DeviceBridge(sdbus::IConnection& conn,
                           const std::string& device_path)
    : conn_(conn),
      device_path_(device_path),
      proxy_(sdbus::createProxy(conn_,
                                sdbus::ServiceName{kBluezService},
                                sdbus::ObjectPath{device_path_})) {}

// ── Async operations ────────────────────────────────────────────────────────

void DeviceBridge::connect(Dart_Port_DL result_port) {
  try {
    proxy_->callMethod("Connect").onInterface(kDeviceIface);
    post_success(result_port);
  } catch (const sdbus::Error& e) {
    post_error(result_port, device_path_, e.getName(), e.getMessage());
  }
}

void DeviceBridge::disconnect(Dart_Port_DL result_port) {
  try {
    proxy_->callMethod("Disconnect").onInterface(kDeviceIface);
    post_success(result_port);
  } catch (const sdbus::Error& e) {
    post_error(result_port, device_path_, e.getName(), e.getMessage());
  }
}

void DeviceBridge::pair(Dart_Port_DL result_port) {
  try {
    proxy_->callMethod("Pair").onInterface(kDeviceIface);
    post_success(result_port);
  } catch (const sdbus::Error& e) {
    post_error(result_port, device_path_, e.getName(), e.getMessage());
  }
}

void DeviceBridge::cancel_pairing() {
  try {
    proxy_->callMethod("CancelPairing").onInterface(kDeviceIface);
  } catch (const sdbus::Error&) {
    // Best-effort: ignore errors on cancel.
  }
}

// ── Properties ──────────────────────────────────────────────────────────────

void DeviceBridge::set_trusted(bool value) {
  set_property_bool("Trusted", value);
}

void DeviceBridge::set_blocked(bool value) {
  set_property_bool("Blocked", value);
}

void DeviceBridge::set_alias(const std::string& value) {
  set_property_string("Alias", value);
}

void DeviceBridge::set_property_bool(const std::string& name, bool value) {
  proxy_->setProperty(name).onInterface(kDeviceIface).toValue(value);
}

void DeviceBridge::set_property_string(const std::string& name,
                                       const std::string& value) {
  proxy_->setProperty(name).onInterface(kDeviceIface).toValue(value);
}

// ── Dart posting helpers ────────────────────────────────────────────────────

void DeviceBridge::post_success(Dart_Port_DL result_port) {
  uint8_t sentinel = 0xFF;
  Dart_CObject obj;
  obj.type = Dart_CObject_kTypedData;
  obj.value.as_typed_data.type = Dart_TypedData_kUint8;
  obj.value.as_typed_data.length = 1;
  obj.value.as_typed_data.values = &sentinel;
  Dart_PostCObject_DL(result_port, &obj);
}

void DeviceBridge::post_error(Dart_Port_DL result_port,
                              const std::string& object_path,
                              const std::string& error_name,
                              const std::string& error_message) {
  BlueZError err;
  err.objectPath = object_path;
  err.name = error_name;
  err.message = error_message;

  auto payload = glz::encode(err);

  std::vector<uint8_t> buf;
  buf.reserve(1 + payload.size());
  buf.push_back(0x20);
  buf.insert(buf.end(), payload.begin(), payload.end());

  Dart_CObject obj;
  obj.type = Dart_CObject_kTypedData;
  obj.value.as_typed_data.type = Dart_TypedData_kUint8;
  obj.value.as_typed_data.length = static_cast<intptr_t>(buf.size());
  obj.value.as_typed_data.values = buf.data();
  Dart_PostCObject_DL(result_port, &obj);
}
