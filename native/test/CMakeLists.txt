find_package(GTest REQUIRED)

add_executable(test_types test_types.cpp)
target_link_libraries(test_types PRIVATE bluez_object_manager GTest::gtest_main)

add_executable(test_object_manager test_object_manager.cpp)
target_link_libraries(test_object_manager PRIVATE bluez_object_manager GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_types)
gtest_discover_tests(test_object_manager)
