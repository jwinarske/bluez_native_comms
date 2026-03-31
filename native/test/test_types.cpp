// test_types.cpp — glaze encode/decode roundtrip tests for every BlueZ struct.

#include "bluez_types.h"

#include <gtest/gtest.h>

// ── ManufacturerDataEntry ───────────────────────────────────────────────────

TEST(BlueZTypes, ManufacturerDataEntryRoundtrip) {
  ManufacturerDataEntry orig;
  orig.companyId = 0x004C;  // Apple
  orig.data = {0x02, 0x15, 0xAA, 0xBB};

  auto buf = glz::encode(orig);
  ManufacturerDataEntry decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.companyId, orig.companyId);
  EXPECT_EQ(decoded.data, orig.data);
}

// ── BlueZAdapterProps ───────────────────────────────────────────────────────

TEST(BlueZTypes, AdapterPropsRoundtrip) {
  BlueZAdapterProps orig;
  orig.objectPath = "/org/bluez/hci0";
  orig.address = "00:11:22:33:44:55";
  orig.name = "hci0";
  orig.alias = "My Adapter";
  orig.powered = true;
  orig.discoverable = false;
  orig.pairable = true;
  orig.discovering = true;
  orig.discoverableTimeout = 180;
  orig.uuids = {"0000110a-0000-1000-8000-00805f9b34fb",
                "0000110b-0000-1000-8000-00805f9b34fb"};

  auto buf = glz::encode(orig);
  BlueZAdapterProps decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, orig.objectPath);
  EXPECT_EQ(decoded.address, orig.address);
  EXPECT_EQ(decoded.name, orig.name);
  EXPECT_EQ(decoded.alias, orig.alias);
  EXPECT_EQ(decoded.powered, orig.powered);
  EXPECT_EQ(decoded.discoverable, orig.discoverable);
  EXPECT_EQ(decoded.pairable, orig.pairable);
  EXPECT_EQ(decoded.discovering, orig.discovering);
  EXPECT_EQ(decoded.discoverableTimeout, orig.discoverableTimeout);
  EXPECT_EQ(decoded.uuids, orig.uuids);
}

TEST(BlueZTypes, AdapterPropsDefaultsRoundtrip) {
  BlueZAdapterProps orig;  // all defaults

  auto buf = glz::encode(orig);
  BlueZAdapterProps decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_TRUE(decoded.objectPath.empty());
  EXPECT_FALSE(decoded.powered);
  EXPECT_EQ(decoded.discoverableTimeout, 0u);
  EXPECT_TRUE(decoded.uuids.empty());
}

// ── BlueZDeviceProps ────────────────────────────────────────────────────────

TEST(BlueZTypes, DevicePropsRoundtrip) {
  BlueZDeviceProps orig;
  orig.objectPath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
  orig.adapterPath = "/org/bluez/hci0";
  orig.address = "AA:BB:CC:DD:EE:FF";
  orig.addressType = "public";
  orig.name = "Heart Rate Sensor";
  orig.alias = "HR Sensor";
  orig.rssi = -65;
  orig.txPower = 4;
  orig.appearance = 0x0340;
  orig.deviceClass = 0;
  orig.paired = false;
  orig.trusted = false;
  orig.blocked = false;
  orig.connected = true;
  orig.servicesResolved = true;
  orig.legacyPairing = false;
  orig.uuids = {"0000180d-0000-1000-8000-00805f9b34fb"};

  ManufacturerDataEntry mfr;
  mfr.companyId = 0x004C;
  mfr.data = {0x01, 0x02, 0x03};
  orig.manufacturerData = {mfr};

  orig.serviceData["0000180d-0000-1000-8000-00805f9b34fb"] = {0x06, 0x50};

  auto buf = glz::encode(orig);
  BlueZDeviceProps decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, orig.objectPath);
  EXPECT_EQ(decoded.adapterPath, orig.adapterPath);
  EXPECT_EQ(decoded.address, orig.address);
  EXPECT_EQ(decoded.addressType, orig.addressType);
  EXPECT_EQ(decoded.name, orig.name);
  EXPECT_EQ(decoded.alias, orig.alias);
  EXPECT_EQ(decoded.rssi, orig.rssi);
  EXPECT_EQ(decoded.txPower, orig.txPower);
  EXPECT_EQ(decoded.appearance, orig.appearance);
  EXPECT_EQ(decoded.deviceClass, orig.deviceClass);
  EXPECT_EQ(decoded.paired, orig.paired);
  EXPECT_EQ(decoded.trusted, orig.trusted);
  EXPECT_EQ(decoded.blocked, orig.blocked);
  EXPECT_EQ(decoded.connected, orig.connected);
  EXPECT_EQ(decoded.servicesResolved, orig.servicesResolved);
  EXPECT_EQ(decoded.legacyPairing, orig.legacyPairing);
  EXPECT_EQ(decoded.uuids, orig.uuids);

  ASSERT_EQ(decoded.manufacturerData.size(), 1u);
  EXPECT_EQ(decoded.manufacturerData[0].companyId, mfr.companyId);
  EXPECT_EQ(decoded.manufacturerData[0].data, mfr.data);

  EXPECT_EQ(decoded.serviceData, orig.serviceData);
}

TEST(BlueZTypes, DevicePropsNegativeRssi) {
  BlueZDeviceProps orig;
  orig.rssi = -120;
  orig.txPower = -10;

  auto buf = glz::encode(orig);
  BlueZDeviceProps decoded;
  glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(decoded.rssi, -120);
  EXPECT_EQ(decoded.txPower, -10);
}

TEST(BlueZTypes, DevicePropsMultipleManufacturerData) {
  BlueZDeviceProps orig;
  orig.manufacturerData = {
      {0x004C, {0x01, 0x02}},
      {0x0059, {0xAA, 0xBB, 0xCC}},
  };

  auto buf = glz::encode(orig);
  BlueZDeviceProps decoded;
  glz::decode(buf.data(), 0, decoded);

  ASSERT_EQ(decoded.manufacturerData.size(), 2u);
  EXPECT_EQ(decoded.manufacturerData[0].companyId, 0x004C);
  EXPECT_EQ(decoded.manufacturerData[0].data,
            (std::vector<uint8_t>{0x01, 0x02}));
  EXPECT_EQ(decoded.manufacturerData[1].companyId, 0x0059);
  EXPECT_EQ(decoded.manufacturerData[1].data,
            (std::vector<uint8_t>{0xAA, 0xBB, 0xCC}));
}

TEST(BlueZTypes, DevicePropsMultipleServiceData) {
  BlueZDeviceProps orig;
  orig.serviceData["uuid-a"] = {0x01};
  orig.serviceData["uuid-b"] = {0x02, 0x03};

  auto buf = glz::encode(orig);
  BlueZDeviceProps decoded;
  glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(decoded.serviceData.size(), 2u);
  EXPECT_EQ(decoded.serviceData["uuid-a"], (std::vector<uint8_t>{0x01}));
  EXPECT_EQ(decoded.serviceData["uuid-b"],
            (std::vector<uint8_t>{0x02, 0x03}));
}

// ── BlueZGattCharProps ──────────────────────────────────────────────────────

TEST(BlueZTypes, GattCharPropsRoundtrip) {
  BlueZGattCharProps orig;
  orig.objectPath =
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b";
  orig.servicePath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a";
  orig.uuid = "00002a37-0000-1000-8000-00805f9b34fb";
  orig.value = {0x06, 0x50};
  orig.notifying = true;
  orig.writeAcquired = false;
  orig.notifyAcquired = false;
  orig.handle = 0x000b;
  orig.mtu = 512;
  orig.flags = {"read", "notify"};

  auto buf = glz::encode(orig);
  BlueZGattCharProps decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, orig.objectPath);
  EXPECT_EQ(decoded.servicePath, orig.servicePath);
  EXPECT_EQ(decoded.uuid, orig.uuid);
  EXPECT_EQ(decoded.value, orig.value);
  EXPECT_EQ(decoded.notifying, orig.notifying);
  EXPECT_EQ(decoded.writeAcquired, orig.writeAcquired);
  EXPECT_EQ(decoded.notifyAcquired, orig.notifyAcquired);
  EXPECT_EQ(decoded.handle, orig.handle);
  EXPECT_EQ(decoded.mtu, orig.mtu);
  EXPECT_EQ(decoded.flags, orig.flags);
}

TEST(BlueZTypes, GattCharPropsEmptyValue) {
  BlueZGattCharProps orig;
  orig.objectPath = "/some/path";
  orig.value = {};
  orig.flags = {};

  auto buf = glz::encode(orig);
  BlueZGattCharProps decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_TRUE(decoded.value.empty());
  EXPECT_TRUE(decoded.flags.empty());
}

// ── BlueZGattServiceProps ───────────────────────────────────────────────────

TEST(BlueZTypes, GattServicePropsRoundtrip) {
  BlueZGattServiceProps orig;
  orig.objectPath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a";
  orig.devicePath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
  orig.uuid = "0000180d-0000-1000-8000-00805f9b34fb";
  orig.primary = true;
  orig.handle = 0x000a;

  auto buf = glz::encode(orig);
  BlueZGattServiceProps decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, orig.objectPath);
  EXPECT_EQ(decoded.devicePath, orig.devicePath);
  EXPECT_EQ(decoded.uuid, orig.uuid);
  EXPECT_EQ(decoded.primary, orig.primary);
  EXPECT_EQ(decoded.handle, orig.handle);
}

// ── BlueZGattDescProps ──────────────────────────────────────────────────────

TEST(BlueZTypes, GattDescPropsRoundtrip) {
  BlueZGattDescProps orig;
  orig.objectPath =
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b/desc000c";
  orig.charPath =
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b";
  orig.uuid = "00002902-0000-1000-8000-00805f9b34fb";
  orig.value = {0x01, 0x00};
  orig.handle = 0x000c;

  auto buf = glz::encode(orig);
  BlueZGattDescProps decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, orig.objectPath);
  EXPECT_EQ(decoded.charPath, orig.charPath);
  EXPECT_EQ(decoded.uuid, orig.uuid);
  EXPECT_EQ(decoded.value, orig.value);
  EXPECT_EQ(decoded.handle, orig.handle);
}

// ── BlueZValueResult ────────────────────────────────────────────────────────

TEST(BlueZTypes, ValueResultRoundtrip) {
  BlueZValueResult orig;
  orig.objectPath =
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b";
  orig.value = {0x06, 0x50, 0x00, 0xFF};

  auto buf = glz::encode(orig);
  BlueZValueResult decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, orig.objectPath);
  EXPECT_EQ(decoded.value, orig.value);
}

TEST(BlueZTypes, ValueResultLargePayload) {
  BlueZValueResult orig;
  orig.objectPath = "/path";
  orig.value.resize(512, 0xAB);

  auto buf = glz::encode(orig);
  BlueZValueResult decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.value.size(), 512u);
  EXPECT_EQ(decoded.value, orig.value);
}

// ── BlueZError ──────────────────────────────────────────────────────────────

TEST(BlueZTypes, ErrorRoundtrip) {
  BlueZError orig;
  orig.objectPath = "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF";
  orig.name = "org.bluez.Error.Failed";
  orig.message = "Connection refused";

  auto buf = glz::encode(orig);
  BlueZError decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, orig.objectPath);
  EXPECT_EQ(decoded.name, orig.name);
  EXPECT_EQ(decoded.message, orig.message);
}

TEST(BlueZTypes, ErrorEmptyMessage) {
  BlueZError orig;
  orig.objectPath = "/path";
  orig.name = "org.bluez.Error.NotReady";
  orig.message = "";

  auto buf = glz::encode(orig);
  BlueZError decoded;
  auto end = glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(end, buf.size());
  EXPECT_EQ(decoded.objectPath, orig.objectPath);
  EXPECT_EQ(decoded.name, orig.name);
  EXPECT_TRUE(decoded.message.empty());
}
