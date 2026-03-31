find_package(GTest REQUIRED)

add_executable(test_types test_types.cpp)
target_link_libraries(test_types PRIVATE bluez_native GTest::gtest_main)

add_executable(test_object_manager test_object_manager.cpp)
target_link_libraries(test_object_manager PRIVATE bluez_native GTest::gtest_main)

add_executable(test_adapter test_adapter.cpp)
target_link_libraries(test_adapter PRIVATE bluez_native GTest::gtest_main)

add_executable(test_gatt_notify test_gatt_notify.cpp)
target_link_libraries(test_gatt_notify PRIVATE bluez_native GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_types)
gtest_discover_tests(test_object_manager)
gtest_discover_tests(test_adapter)
gtest_discover_tests(test_gatt_notify)
