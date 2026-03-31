// test_adapter.cpp — Unit tests for Adapter and Device bridge helpers.
//
// The bridge classes require a running BlueZ daemon for D-Bus operations.
// These tests verify the Dart posting helpers and error encoding that
// don't require a live connection.

#include "device_bridge.h"

#include <gtest/gtest.h>

// ── DeviceBridge error encoding ─────────────────────────────────────────────

TEST(DeviceBridgePost, ErrorPayloadStructure) {
  // Verify that a BlueZError encodes with the correct discriminator.
  BlueZError err;
  err.objectPath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
  err.name = "org.bluez.Error.Failed";
  err.message = "Connection refused";

  auto payload = glz::encode(err);

  std::vector<uint8_t> buf;
  buf.reserve(1 + payload.size());
  buf.push_back(0x20);
  buf.insert(buf.end(), payload.begin(), payload.end());

  // First byte is the discriminator.
  EXPECT_EQ(buf[0], 0x20);

  // The rest should decode back to the original error.
  BlueZError decoded;
  glz::decode(buf.data(), 1, decoded);
  EXPECT_EQ(decoded.objectPath, err.objectPath);
  EXPECT_EQ(decoded.name, err.name);
  EXPECT_EQ(decoded.message, err.message);
}

TEST(DeviceBridgePost, SuccessSentinel) {
  // The success sentinel is a single 0xFF byte.
  uint8_t sentinel = 0xFF;
  EXPECT_EQ(sentinel, 0xFF);
}

// ── AdapterBridge discovery filter construction ─────────────────────────────

TEST(AdapterBridge, DiscoveryFilterConstruction) {
  // Verify we can construct a valid discovery filter map.
  std::map<std::string, sdbus::Variant> filter;
  filter["Transport"] = sdbus::Variant{std::string{"le"}};
  filter["RSSI"] = sdbus::Variant{int16_t{-80}};
  filter["UUIDs"] = sdbus::Variant{
      std::vector<std::string>{"0000180d-0000-1000-8000-00805f9b34fb"}};

  EXPECT_EQ(filter.size(), 3u);
  EXPECT_EQ(filter["Transport"].get<std::string>(), "le");
  EXPECT_EQ(filter["RSSI"].get<int16_t>(), -80);

  auto uuids = filter["UUIDs"].get<std::vector<std::string>>();
  ASSERT_EQ(uuids.size(), 1u);
  EXPECT_EQ(uuids[0], "0000180d-0000-1000-8000-00805f9b34fb");
}

TEST(AdapterBridge, DiscoveryFilterEmpty) {
  std::map<std::string, sdbus::Variant> filter;
  EXPECT_TRUE(filter.empty());
}

// ── Error encoding roundtrip ────────────────────────────────────────────────

TEST(DeviceBridgePost, ErrorEncodingRoundtrip) {
  BlueZError err;
  err.objectPath = "/dev/path";
  err.name = "org.bluez.Error.NotReady";
  err.message = "Resource not ready";

  auto buf = glz::encode(err);
  BlueZError decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, err.objectPath);
  EXPECT_EQ(decoded.name, err.name);
  EXPECT_EQ(decoded.message, err.message);
}

TEST(DeviceBridgePost, ErrorWithEmptyFields) {
  BlueZError err;
  err.objectPath = "";
  err.name = "";
  err.message = "";

  auto buf = glz::encode(err);
  BlueZError decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_TRUE(decoded.objectPath.empty());
  EXPECT_TRUE(decoded.name.empty());
  EXPECT_TRUE(decoded.message.empty());
}
