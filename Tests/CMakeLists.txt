
add_executable(BiletomatTest test_main.cpp)
target_include_directories(BiletomatTest PUBLIC ../Client)
target_include_directories(BiletomatTest PUBLIC ../Common)

target_link_libraries(BiletomatTest PRIVATE
    Biletomat_lib
    nlohmann_json::nlohmann_json
    GTest::gtest
    GTest::gtest_main
)

target_compile_definitions(BiletomatTest PRIVATE
    TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/Tests/testData"
)
include(GoogleTest)
gtest_discover_tests(BiletomatTest)