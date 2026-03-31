// bluez_bridge.cpp — C ABI entry points wrapping all bridges.
//
// The BridgeContext owns one sdbus connection running its event loop on a
// dedicated thread. All bridge objects share this connection.

#include "bluez_bridge.h"

#include <memory>
#include <string>
#include <thread>

#include "adapter_bridge.h"
#include "device_bridge.h"
#include "gatt_bridge.h"
#include "object_manager.h"

struct BridgeContext {
  std::unique_ptr<sdbus::IConnection> conn;
  std::unique_ptr<ObjectManager> obj_mgr;
  Dart_Port_DL events_port;
  std::thread event_loop;
};

extern "C" {

void bluez_bridge_init(void* dart_api_dl_data) {
  Dart_InitializeApiDL(dart_api_dl_data);
}

void* bluez_client_create(int64_t events_port) {
  auto ctx = std::make_unique<BridgeContext>();
  ctx->events_port = events_port;

  ctx->conn = sdbus::createSystemBusConnection();
  ctx->obj_mgr =
      std::make_unique<ObjectManager>(*ctx->conn, ctx->events_port);

  // Snapshot the current BlueZ object tree.
  ctx->obj_mgr->get_managed_objects();

  // Run the sdbus event loop on a dedicated thread.
  ctx->event_loop = std::thread([&conn = *ctx->conn]() {
    conn.enterEventLoop();
  });

  return ctx.release();
}

void bluez_client_destroy(void* handle) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  ctx->conn->leaveEventLoop();
  if (ctx->event_loop.joinable()) {
    ctx->event_loop.join();
  }
  delete ctx;  // NOLINT(cppcoreguidelines-owning-memory)
}

// ── Adapter operations ──────────────────────────────────────────────────────

void bluez_adapter_start_discovery(void* handle, const char* adapter_path) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  AdapterBridge adapter(*ctx->conn, adapter_path);
  adapter.start_discovery();
}

void bluez_adapter_stop_discovery(void* handle, const char* adapter_path) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  AdapterBridge adapter(*ctx->conn, adapter_path);
  adapter.stop_discovery();
}

void bluez_adapter_set_discovery_filter(void* handle,
                                        const char* adapter_path,
                                        const uint8_t* filter_json,
                                        int32_t len) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  AdapterBridge adapter(*ctx->conn, adapter_path);

  // Decode the filter from glaze binary: Transport(s), UUIDs(as), RSSI(n).
  std::map<std::string, sdbus::Variant> filter;
  if (filter_json != nullptr && len > 0) {
    // For now, pass empty filter — full decode added with Dart FFI layer.
    (void)filter_json;
    (void)len;
  }
  adapter.set_discovery_filter(filter);
}

void bluez_adapter_remove_device(void* handle,
                                 const char* adapter_path,
                                 const char* device_path) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  AdapterBridge adapter(*ctx->conn, adapter_path);
  adapter.remove_device(device_path);
}

void bluez_adapter_set_property(void* handle,
                                const char* adapter_path,
                                const char* prop_name,
                                const uint8_t* value_json,
                                int32_t len) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  AdapterBridge adapter(*ctx->conn, adapter_path);

  // Decode property value from glaze binary.
  // For bool properties, the first byte is 0/1.
  if (len == 1) {
    adapter.set_property_bool(prop_name, value_json[0] != 0);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }
  // For string properties, read length-prefixed string.
  // For uint32 properties, read 4 bytes little-endian.
  // Full type dispatch deferred to Dart-side encoding.
}

// ── Device operations ───────────────────────────────────────────────────────

void bluez_device_connect(void* handle,
                          const char* device_path,
                          int64_t result_port) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  DeviceBridge device(*ctx->conn, device_path);
  device.connect(result_port);
}

void bluez_device_disconnect(void* handle,
                             const char* device_path,
                             int64_t result_port) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  DeviceBridge device(*ctx->conn, device_path);
  device.disconnect(result_port);
}

void bluez_device_pair(void* handle,
                       const char* device_path,
                       int64_t result_port) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  DeviceBridge device(*ctx->conn, device_path);
  device.pair(result_port);
}

void bluez_device_cancel_pairing(void* handle, const char* device_path) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  DeviceBridge device(*ctx->conn, device_path);
  device.cancel_pairing();
}

void bluez_device_set_property(void* handle,
                               const char* device_path,
                               const char* prop_name,
                               const uint8_t* value_json,
                               int32_t len) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  DeviceBridge device(*ctx->conn, device_path);

  if (len == 1) {
    device.set_property_bool(prop_name, value_json[0] != 0);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }
}

// ── GATT characteristic operations ──────────────────────────────────────────

void bluez_char_read_value(void* handle,
                           const char* char_path,
                           int64_t result_port) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  GattCharBridge gatt(*ctx->conn, char_path, *ctx->obj_mgr);
  gatt.read_value(result_port);
}

void bluez_char_write_value(void* handle,
                            const char* char_path,
                            const uint8_t* value_buf,
                            int32_t len,
                            bool with_response,
                            int64_t result_port) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  GattCharBridge gatt(*ctx->conn, char_path, *ctx->obj_mgr);
  gatt.write_value(value_buf, len, with_response, result_port);
}

void bluez_char_start_notify(void* handle,
                             const char* char_path,
                             int64_t result_port) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  GattCharBridge gatt(*ctx->conn, char_path, *ctx->obj_mgr);
  gatt.start_notify(result_port);
}

void bluez_char_stop_notify(void* handle,
                            const char* char_path,
                            int64_t result_port) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  GattCharBridge gatt(*ctx->conn, char_path, *ctx->obj_mgr);
  gatt.stop_notify(result_port);
}

// ── GATT descriptor operations ──────────────────────────────────────────────

void bluez_desc_read_value(void* handle,
                           const char* desc_path,
                           int64_t result_port) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  GattDescBridge desc(*ctx->conn, desc_path);
  desc.read_value(result_port);
}

void bluez_desc_write_value(void* handle,
                            const char* desc_path,
                            const uint8_t* value_buf,
                            int32_t len,
                            int64_t result_port) {
  auto* ctx = static_cast<BridgeContext*>(handle);
  GattDescBridge desc(*ctx->conn, desc_path);
  desc.write_value(value_buf, len, result_port);
}

}  // extern "C"
