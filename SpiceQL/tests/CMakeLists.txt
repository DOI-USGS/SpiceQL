cmake_minimum_required(VERSION 3.10)

# A lot of linux OSes do not support C++20
set(CMAKE_CXX_STANDARD 20)

find_package(CSpice REQUIRED)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()
set(SPICEQL_TEST_DIRECTORY ${CMAKE_SOURCE_DIR}/SpiceQL/tests/)

# collect all of the test sources
set (SPICEQL_TEST_SOURCE    ${SPICEQL_TEST_DIRECTORY}/TestUtilities.cpp
                            ${SPICEQL_TEST_DIRECTORY}/Fixtures.cpp
                            ${SPICEQL_TEST_DIRECTORY}/UtilTests.cpp
                            ${SPICEQL_TEST_DIRECTORY}/QueryTests.cpp
                            ${SPICEQL_TEST_DIRECTORY}/IoTests.cpp
                            ${SPICEQL_TEST_DIRECTORY}/KernelTests.cpp
                            ${SPICEQL_TEST_DIRECTORY}/MemoTests.cpp
                            ${SPICEQL_TEST_DIRECTORY}/InventoryTests.cpp 
                            ${SPICEQL_TEST_DIRECTORY}/FunctionalTestsSpiceQueries.cpp
                            ${SPICEQL_TEST_DIRECTORY}/FunctionalTestsConfig.cpp)

# setup test executable
add_executable(runSpiceQLTests TestMain.cpp ${SPICEQL_TEST_SOURCE})

target_compile_definitions(runSpiceQLTests PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

target_link_libraries(runSpiceQLTests
                      PRIVATE
                      SpiceQL
                      redis++
                      cspice
                      gtest
                      gmock
                      Threads::Threads
                      spdlog::spdlog_header_only
                      )


target_include_directories(runSpiceQLTests PRIVATE ${CMAKE_SOURCE_DIR}/submodules/redis-plus-plus/src/ 
                                                   ${CMAKE_SOURCE_DIR}/submodules/hippomocks/ 
                                                   ${CMAKE_SOURCE_DIR}/submodules/BTree/include/)

gtest_discover_tests(runSpiceQLTests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/SpiceQL/tests/)
