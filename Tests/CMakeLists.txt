
add_executable(BiletomatTest test_main.cpp)
target_include_directories(BiletomatTest PUBLIC ../Client)
target_include_directories(BiletomatTest PUBLIC ../Common)

target_link_libraries(BiletomatTest PRIVATE
    Biletomat_lib
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(BiletomatTest)