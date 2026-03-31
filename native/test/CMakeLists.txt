find_package(GTest REQUIRED)

add_executable(test_types test_types.cpp)
target_link_libraries(test_types PRIVATE bluez_types GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_types)
