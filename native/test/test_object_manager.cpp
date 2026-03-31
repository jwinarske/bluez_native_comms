// test_object_manager.cpp — Unit tests for ObjectManager property extraction
// and posting logic.
//
// These tests verify the static property extraction methods that convert
// sdbus::Variant property maps into BlueZ type structs. The actual D-Bus
// signal subscription requires a running BlueZ daemon and is tested via
// integration tests.

#include "object_manager.h"

#include <gtest/gtest.h>

using PropertiesMap = std::map<std::string, sdbus::Variant>;

// ── Adapter property extraction ─────────────────────────────────────────────

TEST(ObjectManagerExtract, AdapterPropsBasic) {
  PropertiesMap props;
  props["Address"] = sdbus::Variant{std::string{"00:11:22:33:44:55"}};
  props["Name"] = sdbus::Variant{std::string{"hci0"}};
  props["Alias"] = sdbus::Variant{std::string{"My Adapter"}};
  props["Powered"] = sdbus::Variant{true};
  props["Discoverable"] = sdbus::Variant{false};
  props["Pairable"] = sdbus::Variant{true};
  props["Discovering"] = sdbus::Variant{true};
  props["DiscoverableTimeout"] = sdbus::Variant{uint32_t{180}};
  props["UUIDs"] = sdbus::Variant{
      std::vector<std::string>{"0000110a-0000-1000-8000-00805f9b34fb"}};

  auto a = ObjectManager::extract_adapter_props("/org/bluez/hci0", props);

  EXPECT_EQ(a.objectPath, "/org/bluez/hci0");
  EXPECT_EQ(a.address, "00:11:22:33:44:55");
  EXPECT_EQ(a.name, "hci0");
  EXPECT_EQ(a.alias, "My Adapter");
  EXPECT_TRUE(a.powered);
  EXPECT_FALSE(a.discoverable);
  EXPECT_TRUE(a.pairable);
  EXPECT_TRUE(a.discovering);
  EXPECT_EQ(a.discoverableTimeout, 180u);
  ASSERT_EQ(a.uuids.size(), 1u);
  EXPECT_EQ(a.uuids[0], "0000110a-0000-1000-8000-00805f9b34fb");
}

TEST(ObjectManagerExtract, AdapterPropsMissing) {
  PropertiesMap props;
  // Only objectPath is set, everything else should be default.
  auto a = ObjectManager::extract_adapter_props("/org/bluez/hci0", props);

  EXPECT_EQ(a.objectPath, "/org/bluez/hci0");
  EXPECT_TRUE(a.address.empty());
  EXPECT_FALSE(a.powered);
  EXPECT_EQ(a.discoverableTimeout, 0u);
  EXPECT_TRUE(a.uuids.empty());
}

// ── Device property extraction ──────────────────────────────────────────────

TEST(ObjectManagerExtract, DevicePropsBasic) {
  PropertiesMap props;
  props["Address"] = sdbus::Variant{std::string{"AA:BB:CC:DD:EE:FF"}};
  props["AddressType"] = sdbus::Variant{std::string{"public"}};
  props["Name"] = sdbus::Variant{std::string{"Heart Rate Sensor"}};
  props["Alias"] = sdbus::Variant{std::string{"HR Sensor"}};
  props["RSSI"] = sdbus::Variant{int16_t{-65}};
  props["TxPower"] = sdbus::Variant{int16_t{4}};
  props["Appearance"] = sdbus::Variant{uint16_t{0x0340}};
  props["Class"] = sdbus::Variant{uint32_t{0}};
  props["Paired"] = sdbus::Variant{false};
  props["Trusted"] = sdbus::Variant{false};
  props["Blocked"] = sdbus::Variant{false};
  props["Connected"] = sdbus::Variant{true};
  props["ServicesResolved"] = sdbus::Variant{true};
  props["LegacyPairing"] = sdbus::Variant{false};
  props["UUIDs"] = sdbus::Variant{
      std::vector<std::string>{"0000180d-0000-1000-8000-00805f9b34fb"}};
  props["Adapter"] =
      sdbus::Variant{sdbus::ObjectPath{"/org/bluez/hci0"}};

  auto d = ObjectManager::extract_device_props(
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF", props);

  EXPECT_EQ(d.objectPath, "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
  EXPECT_EQ(d.address, "AA:BB:CC:DD:EE:FF");
  EXPECT_EQ(d.addressType, "public");
  EXPECT_EQ(d.name, "Heart Rate Sensor");
  EXPECT_EQ(d.rssi, -65);
  EXPECT_EQ(d.txPower, 4);
  EXPECT_EQ(d.appearance, 0x0340);
  EXPECT_TRUE(d.connected);
  EXPECT_TRUE(d.servicesResolved);
  EXPECT_EQ(d.adapterPath, "/org/bluez/hci0");
  ASSERT_EQ(d.uuids.size(), 1u);
}

TEST(ObjectManagerExtract, DevicePropsManufacturerData) {
  PropertiesMap props;
  props["Address"] = sdbus::Variant{std::string{"AA:BB:CC:DD:EE:FF"}};

  // ManufacturerData: a{qv} where v is ay
  std::map<uint16_t, sdbus::Variant> mfr;
  mfr[0x004C] = sdbus::Variant{std::vector<uint8_t>{0x02, 0x15, 0xAA}};
  props["ManufacturerData"] = sdbus::Variant{mfr};

  auto d = ObjectManager::extract_device_props("/path", props);

  ASSERT_EQ(d.manufacturerData.size(), 1u);
  EXPECT_EQ(d.manufacturerData[0].companyId, 0x004C);
  EXPECT_EQ(d.manufacturerData[0].data,
            (std::vector<uint8_t>{0x02, 0x15, 0xAA}));
}

TEST(ObjectManagerExtract, DevicePropsServiceData) {
  PropertiesMap props;
  props["Address"] = sdbus::Variant{std::string{"AA:BB:CC:DD:EE:FF"}};

  // ServiceData: a{sv} where v is ay
  std::map<std::string, sdbus::Variant> svc;
  svc["0000180d-0000-1000-8000-00805f9b34fb"] =
      sdbus::Variant{std::vector<uint8_t>{0x06, 0x50}};
  props["ServiceData"] = sdbus::Variant{svc};

  auto d = ObjectManager::extract_device_props("/path", props);

  ASSERT_EQ(d.serviceData.size(), 1u);
  auto it = d.serviceData.find("0000180d-0000-1000-8000-00805f9b34fb");
  ASSERT_NE(it, d.serviceData.end());
  EXPECT_EQ(it->second, (std::vector<uint8_t>{0x06, 0x50}));
}

TEST(ObjectManagerExtract, DevicePropsMissing) {
  PropertiesMap props;
  auto d = ObjectManager::extract_device_props("/path", props);

  EXPECT_EQ(d.objectPath, "/path");
  EXPECT_TRUE(d.address.empty());
  EXPECT_EQ(d.rssi, 0);
  EXPECT_FALSE(d.connected);
  EXPECT_TRUE(d.manufacturerData.empty());
  EXPECT_TRUE(d.serviceData.empty());
}

// ── GATT service property extraction ────────────────────────────────────────

TEST(ObjectManagerExtract, GattServicePropsBasic) {
  PropertiesMap props;
  props["UUID"] =
      sdbus::Variant{std::string{"0000180d-0000-1000-8000-00805f9b34fb"}};
  props["Primary"] = sdbus::Variant{true};
  props["Device"] = sdbus::Variant{
      sdbus::ObjectPath{"/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF"}};
  props["Handle"] = sdbus::Variant{uint16_t{0x000a}};

  auto s = ObjectManager::extract_service_props(
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a", props);

  EXPECT_EQ(s.objectPath,
            "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a");
  EXPECT_EQ(s.uuid, "0000180d-0000-1000-8000-00805f9b34fb");
  EXPECT_TRUE(s.primary);
  EXPECT_EQ(s.devicePath, "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF");
  EXPECT_EQ(s.handle, 0x000a);
}

// ── GATT characteristic property extraction ─────────────────────────────────

TEST(ObjectManagerExtract, GattCharPropsBasic) {
  PropertiesMap props;
  props["UUID"] =
      sdbus::Variant{std::string{"00002a37-0000-1000-8000-00805f9b34fb"}};
  props["Service"] = sdbus::Variant{sdbus::ObjectPath{
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a"}};
  props["Value"] = sdbus::Variant{std::vector<uint8_t>{0x06, 0x50}};
  props["Notifying"] = sdbus::Variant{true};
  props["WriteAcquired"] = sdbus::Variant{false};
  props["NotifyAcquired"] = sdbus::Variant{false};
  props["Handle"] = sdbus::Variant{uint16_t{0x000b}};
  props["MTU"] = sdbus::Variant{uint16_t{512}};
  props["Flags"] = sdbus::Variant{std::vector<std::string>{"read", "notify"}};

  auto c = ObjectManager::extract_char_props(
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b", props);

  EXPECT_EQ(c.uuid, "00002a37-0000-1000-8000-00805f9b34fb");
  EXPECT_EQ(c.servicePath,
            "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a");
  EXPECT_EQ(c.value, (std::vector<uint8_t>{0x06, 0x50}));
  EXPECT_TRUE(c.notifying);
  EXPECT_FALSE(c.writeAcquired);
  EXPECT_EQ(c.handle, 0x000b);
  EXPECT_EQ(c.mtu, 512);
  ASSERT_EQ(c.flags.size(), 2u);
  EXPECT_EQ(c.flags[0], "read");
  EXPECT_EQ(c.flags[1], "notify");
}

// ── GATT descriptor property extraction ─────────────────────────────────────

TEST(ObjectManagerExtract, GattDescPropsBasic) {
  PropertiesMap props;
  props["UUID"] =
      sdbus::Variant{std::string{"00002902-0000-1000-8000-00805f9b34fb"}};
  props["Characteristic"] = sdbus::Variant{sdbus::ObjectPath{
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b"}};
  props["Value"] = sdbus::Variant{std::vector<uint8_t>{0x01, 0x00}};
  props["Handle"] = sdbus::Variant{uint16_t{0x000c}};

  auto desc = ObjectManager::extract_desc_props(
      "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b/desc000c",
      props);

  EXPECT_EQ(desc.uuid, "00002902-0000-1000-8000-00805f9b34fb");
  EXPECT_EQ(desc.charPath,
            "/org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service000a/char000b");
  EXPECT_EQ(desc.value, (std::vector<uint8_t>{0x01, 0x00}));
  EXPECT_EQ(desc.handle, 0x000c);
}

// ── Roundtrip: extract → encode → decode ────────────────────────────────────

TEST(ObjectManagerExtract, AdapterExtractEncodeDecodeRoundtrip) {
  PropertiesMap props;
  props["Address"] = sdbus::Variant{std::string{"00:11:22:33:44:55"}};
  props["Name"] = sdbus::Variant{std::string{"hci0"}};
  props["Powered"] = sdbus::Variant{true};
  props["Discovering"] = sdbus::Variant{true};

  auto a = ObjectManager::extract_adapter_props("/org/bluez/hci0", props);
  auto buf = glz::encode(a);
  BlueZAdapterProps decoded;
  glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(decoded.objectPath, "/org/bluez/hci0");
  EXPECT_EQ(decoded.address, "00:11:22:33:44:55");
  EXPECT_EQ(decoded.name, "hci0");
  EXPECT_TRUE(decoded.powered);
  EXPECT_TRUE(decoded.discovering);
}

TEST(ObjectManagerExtract, DeviceExtractEncodeDecodeRoundtrip) {
  PropertiesMap props;
  props["Address"] = sdbus::Variant{std::string{"AA:BB:CC:DD:EE:FF"}};
  props["RSSI"] = sdbus::Variant{int16_t{-72}};
  props["Connected"] = sdbus::Variant{true};
  props["Adapter"] =
      sdbus::Variant{sdbus::ObjectPath{"/org/bluez/hci0"}};

  std::map<uint16_t, sdbus::Variant> mfr;
  mfr[0x0059] = sdbus::Variant{std::vector<uint8_t>{0xAA, 0xBB}};
  props["ManufacturerData"] = sdbus::Variant{mfr};

  auto d = ObjectManager::extract_device_props("/dev/path", props);
  auto buf = glz::encode(d);
  BlueZDeviceProps decoded;
  glz::decode(buf.data(), 0, decoded);

  EXPECT_EQ(decoded.address, "AA:BB:CC:DD:EE:FF");
  EXPECT_EQ(decoded.rssi, -72);
  EXPECT_TRUE(decoded.connected);
  EXPECT_EQ(decoded.adapterPath, "/org/bluez/hci0");
  ASSERT_EQ(decoded.manufacturerData.size(), 1u);
  EXPECT_EQ(decoded.manufacturerData[0].companyId, 0x0059);
}
