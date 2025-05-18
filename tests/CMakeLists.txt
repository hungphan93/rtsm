add_executable(unit_tests
    adlinuxsysteminforeader_test.cpp
    ../adapters/adlinuxsysteminforeader.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Core Quick)


# Include parent project headers
target_include_directories(unit_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

# Link Qt and GTest
target_link_libraries(unit_tests
    PRIVATE
        Qt6::Core
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
