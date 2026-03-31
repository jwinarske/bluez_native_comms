// gatt_bridge.cpp — GATT Characteristic1 and Descriptor1 implementations.

#include "gatt_bridge.h"

#include <utility>

// ═══════════════════════════════════════════════════════════════════════════
// GattCharBridge
// ═══════════════════════════════════════════════════════════════════════════

GattCharBridge::GattCharBridge(sdbus::IConnection& conn,
                               std::string char_path,
                               ObjectManager& obj_mgr)
    : conn_(conn),
      char_path_(std::move(char_path)),
      obj_mgr_(obj_mgr),
      proxy_(sdbus::createProxy(conn_,
                                sdbus::ServiceName{kBluezService},
                                sdbus::ObjectPath{char_path_})) {}

void GattCharBridge::read_value(Dart_Port_DL result_port) {
  try {
    std::map<std::string, sdbus::Variant> options;
    std::vector<uint8_t> value;
    proxy_->callMethod("ReadValue")
        .onInterface(kGattCharIface)
        .withArguments(options)
        .storeResultsTo(value);
    post_value_result(result_port, char_path_, value);
  } catch (const sdbus::Error& e) {
    post_error(result_port, char_path_, e.getName(), e.getMessage());
  }
}

void GattCharBridge::write_value(const uint8_t* data,
                                 int32_t len,
                                 bool with_response,
                                 Dart_Port_DL result_port) {
  try {
    std::vector<uint8_t> value(data, data + static_cast<size_t>(len));  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::map<std::string, sdbus::Variant> options;
    if (!with_response) {
      options["type"] = sdbus::Variant{std::string{"command"}};
    }
    proxy_->callMethod("WriteValue")
        .onInterface(kGattCharIface)
        .withArguments(value, options);
    post_success(result_port);
  } catch (const sdbus::Error& e) {
    post_error(result_port, char_path_, e.getName(), e.getMessage());
  }
}

void GattCharBridge::start_notify(Dart_Port_DL result_port) {
  try {
    proxy_->callMethod("StartNotify").onInterface(kGattCharIface);
    // Wire up PropertiesChanged → events_port for Value changes.
    obj_mgr_.subscribe_char_notify(char_path_);
    post_success(result_port);
  } catch (const sdbus::Error& e) {
    post_error(result_port, char_path_, e.getName(), e.getMessage());
  }
}

void GattCharBridge::stop_notify(Dart_Port_DL result_port) {
  try {
    proxy_->callMethod("StopNotify").onInterface(kGattCharIface);
    obj_mgr_.unsubscribe_char_notify(char_path_);
    post_success(result_port);
  } catch (const sdbus::Error& e) {
    post_error(result_port, char_path_, e.getName(), e.getMessage());
  }
}

// ── Dart posting helpers ────────────────────────────────────────────────────

void GattCharBridge::post_success(Dart_Port_DL result_port) {
  uint8_t sentinel = 0xFF;
  Dart_CObject obj;
  obj.type = Dart_CObject_kTypedData;
  obj.value.as_typed_data.type = Dart_TypedData_kUint8;
  obj.value.as_typed_data.length = 1;
  obj.value.as_typed_data.values = &sentinel;
  Dart_PostCObject_DL(result_port, &obj);
}

void GattCharBridge::post_value_result(Dart_Port_DL result_port,
                                       const std::string& object_path,
                                       const std::vector<uint8_t>& value) {
  BlueZValueResult result;
  result.objectPath = object_path;
  result.value = value;

  auto payload = glz::encode(result);

  std::vector<uint8_t> buf;
  buf.reserve(1 + payload.size());
  buf.push_back(0x10);  // Characteristic value result.
  buf.insert(buf.end(), payload.begin(), payload.end());

  Dart_CObject obj;
  obj.type = Dart_CObject_kTypedData;
  obj.value.as_typed_data.type = Dart_TypedData_kUint8;
  obj.value.as_typed_data.length = static_cast<intptr_t>(buf.size());
  obj.value.as_typed_data.values = buf.data();
  Dart_PostCObject_DL(result_port, &obj);
}

void GattCharBridge::post_error(Dart_Port_DL result_port,
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

// ═══════════════════════════════════════════════════════════════════════════
// GattDescBridge
// ═══════════════════════════════════════════════════════════════════════════

GattDescBridge::GattDescBridge(sdbus::IConnection& conn,
                               std::string desc_path)
    : conn_(conn),
      desc_path_(std::move(desc_path)),
      proxy_(sdbus::createProxy(conn_,
                                sdbus::ServiceName{kBluezService},
                                sdbus::ObjectPath{desc_path_})) {}

void GattDescBridge::read_value(Dart_Port_DL result_port) {
  try {
    std::map<std::string, sdbus::Variant> options;
    std::vector<uint8_t> value;
    proxy_->callMethod("ReadValue")
        .onInterface(kGattDescIface)
        .withArguments(options)
        .storeResultsTo(value);
    post_value_result(result_port, desc_path_, value);
  } catch (const sdbus::Error& e) {
    post_error(result_port, desc_path_, e.getName(), e.getMessage());
  }
}

void GattDescBridge::write_value(const uint8_t* data,
                                 int32_t len,
                                 Dart_Port_DL result_port) {
  try {
    std::vector<uint8_t> value(data, data + static_cast<size_t>(len));  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::map<std::string, sdbus::Variant> options;
    proxy_->callMethod("WriteValue")
        .onInterface(kGattDescIface)
        .withArguments(value, options);
    post_success(result_port);
  } catch (const sdbus::Error& e) {
    post_error(result_port, desc_path_, e.getName(), e.getMessage());
  }
}

// ── Dart posting helpers ────────────────────────────────────────────────────

void GattDescBridge::post_success(Dart_Port_DL result_port) {
  uint8_t sentinel = 0xFF;
  Dart_CObject obj;
  obj.type = Dart_CObject_kTypedData;
  obj.value.as_typed_data.type = Dart_TypedData_kUint8;
  obj.value.as_typed_data.length = 1;
  obj.value.as_typed_data.values = &sentinel;
  Dart_PostCObject_DL(result_port, &obj);
}

void GattDescBridge::post_value_result(Dart_Port_DL result_port,
                                       const std::string& object_path,
                                       const std::vector<uint8_t>& value) {
  BlueZValueResult result;
  result.objectPath = object_path;
  result.value = value;

  auto payload = glz::encode(result);

  std::vector<uint8_t> buf;
  buf.reserve(1 + payload.size());
  buf.push_back(0x11);  // Descriptor value result.
  buf.insert(buf.end(), payload.begin(), payload.end());

  Dart_CObject obj;
  obj.type = Dart_CObject_kTypedData;
  obj.value.as_typed_data.type = Dart_TypedData_kUint8;
  obj.value.as_typed_data.length = static_cast<intptr_t>(buf.size());
  obj.value.as_typed_data.values = buf.data();
  Dart_PostCObject_DL(result_port, &obj);
}

void GattDescBridge::post_error(Dart_Port_DL result_port,
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
