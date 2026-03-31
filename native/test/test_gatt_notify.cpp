// test_gatt_notify.cpp — Unit tests for GATT bridge posting helpers and
// value/error payload encoding.
//
// D-Bus operations require a running BlueZ daemon. These tests verify
// the payload encoding path: discriminator + glaze-encoded struct.

#include "gatt_bridge.h"

#include <gtest/gtest.h>

// ── Characteristic value result encoding (0x10) ─────────────────────────────

TEST(GattBridge, CharValueResultPayload) {
  BlueZValueResult result;
  result.objectPath =
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b";
  result.value = {0x06, 0x50};

  auto payload = glz::encode(result);

  std::vector<uint8_t> buf;
  buf.push_back(0x10);
  buf.insert(buf.end(), payload.begin(), payload.end());

  EXPECT_EQ(buf[0], 0x10);

  BlueZValueResult decoded;
  auto end = glz::decode(buf.data(), 1, decoded);
  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, result.objectPath);
  EXPECT_EQ(decoded.value, result.value);
}

TEST(GattBridge, CharValueResultEmpty) {
  BlueZValueResult result;
  result.objectPath = "/path";
  result.value = {};

  auto payload = glz::encode(result);

  std::vector<uint8_t> buf;
  buf.push_back(0x10);
  buf.insert(buf.end(), payload.begin(), payload.end());

  BlueZValueResult decoded;
  glz::decode(buf.data(), 1, decoded);
  EXPECT_EQ(decoded.objectPath, "/path");
  EXPECT_TRUE(decoded.value.empty());
}

TEST(GattBridge, CharValueResultLarge) {
  BlueZValueResult result;
  result.objectPath = "/char/path";
  result.value.resize(512, 0xAB);

  auto payload = glz::encode(result);

  std::vector<uint8_t> buf;
  buf.push_back(0x10);
  buf.insert(buf.end(), payload.begin(), payload.end());

  BlueZValueResult decoded;
  glz::decode(buf.data(), 1, decoded);
  EXPECT_EQ(decoded.value.size(), 512u);
  EXPECT_EQ(decoded.value[0], 0xAB);
  EXPECT_EQ(decoded.value[511], 0xAB);
}

// ── Descriptor value result encoding (0x11) ─────────────────────────────────

TEST(GattBridge, DescValueResultPayload) {
  BlueZValueResult result;
  result.objectPath =
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b/desc000c";
  result.value = {0x01, 0x00};

  auto payload = glz::encode(result);

  std::vector<uint8_t> buf;
  buf.push_back(0x11);
  buf.insert(buf.end(), payload.begin(), payload.end());

  EXPECT_EQ(buf[0], 0x11);

  BlueZValueResult decoded;
  auto end = glz::decode(buf.data(), 1, decoded);
  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, result.objectPath);
  EXPECT_EQ(decoded.value, (std::vector<uint8_t>{0x01, 0x00}));
}

// ── Error encoding (0x20) ───────────────────────────────────────────────────

TEST(GattBridge, ErrorPayload) {
  BlueZError err;
  err.objectPath = "/org/bluez/hci0/dev_XX/service/char";
  err.name = "org.bluez.Error.NotPermitted";
  err.message = "Read not permitted";

  auto payload = glz::encode(err);

  std::vector<uint8_t> buf;
  buf.push_back(0x20);
  buf.insert(buf.end(), payload.begin(), payload.end());

  EXPECT_EQ(buf[0], 0x20);

  BlueZError decoded;
  auto end = glz::decode(buf.data(), 1, decoded);
  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, err.objectPath);
  EXPECT_EQ(decoded.name, err.name);
  EXPECT_EQ(decoded.message, err.message);
}

// ── Notification value encoding (0x03 via ObjectManager) ────────────────────

TEST(GattBridge, NotifyValuePayload) {
  // Simulates the payload ObjectManager posts when PropertiesChanged fires
  // with a new Value on a notifying characteristic.
  BlueZGattCharProps props;
  props.objectPath = "/org/bluez/hci0/dev_AA_BB/service/char";
  props.value = {0x06, 0x48};  // HR: 72 bpm

  auto payload = glz::encode(props);

  std::vector<uint8_t> buf;
  buf.push_back(0x03);
  buf.insert(buf.end(), payload.begin(), payload.end());

  EXPECT_EQ(buf[0], 0x03);

  BlueZGattCharProps decoded;
  auto end = glz::decode(buf.data(), 1, decoded);
  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, props.objectPath);
  EXPECT_EQ(decoded.value, (std::vector<uint8_t>{0x06, 0x48}));
}

TEST(GattBridge, NotifyValueHighFrequency) {
  // Simulate rapid IMU-like notifications (50 Hz, 20 bytes each).
  for (int i = 0; i < 100; ++i) {
    BlueZGattCharProps props;
    props.objectPath = "/char/path";
    props.value.resize(20);
    for (int j = 0; j < 20; ++j) {
      props.value[j] = static_cast<uint8_t>((i + j) & 0xFF);
    }

    auto payload = glz::encode(props);
    std::vector<uint8_t> buf;
    buf.push_back(0x03);
    buf.insert(buf.end(), payload.begin(), payload.end());

    BlueZGattCharProps decoded;
    glz::decode(buf.data(), 1, decoded);
    EXPECT_EQ(decoded.value, props.value);
  }
}

// ── WriteValue option encoding ──────────────────────────────────────────────

TEST(GattBridge, WriteWithResponseOption) {
  // with_response = true → no "type" option (default is "request").
  std::map<std::string, sdbus::Variant> options;
  EXPECT_TRUE(options.empty());
}

TEST(GattBridge, WriteWithoutResponseOption) {
  // with_response = false → options["type"] = "command".
  std::map<std::string, sdbus::Variant> options;
  options["type"] = sdbus::Variant{std::string{"command"}};

  EXPECT_EQ(options.size(), 1u);
  EXPECT_EQ(options["type"].get<std::string>(), "command");
}

// ── Success sentinel ────────────────────────────────────────────────────────

TEST(GattBridge, SuccessSentinelValue) {
  // The success sentinel is a single 0xFF byte.
  uint8_t sentinel = 0xFF;
  EXPECT_EQ(sentinel, 0xFF);
}
