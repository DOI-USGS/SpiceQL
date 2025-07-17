include(CMakeDependentOption)
cmake_minimum_required(VERSION 3.10)
project(SpiceQL VERSION 1.2.4 DESCRIPTION "Spice Query Library")

set(PACKAGE_VERSION ${PROJECT_VERSION})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message(STATUS "CMake Module Path: " ${CMAKE_MODULE_PATH})

# A lot of linux OSes do not support C++20
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# to avoid complaints in conda-forge, see https://conda-forge.org/docs/maintainer/knowledge_base/#newer-c-features-with-old-sdk 
set(CXXFLAGS, "${CXXFLAGS} -D_LIBCPP_DISABLE_AVAILABILITY")

# Set up Anaconda prefix in the case with a non-default conda env is activated
if(EXISTS $ENV{CONDA_PREFIX})
  message("CONDA PREFIX: $ENV{CONDA_PREFIX}")
  list(APPEND CMAKE_FIND_ROOT_PATH $ENV{CONDA_PREFIX}
                                   $ENV{CONDA_PREFIX}/lib/cmake/Qt5)
endif()

list(APPEND CMAKE_FIND_ROOT_PATH /var/lang/)

message(STATUS "find path: " ${CMAKE_FIND_ROOT_PATH})

# Variables required by multiple build options
set(SPICEQL_BUILD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/include/")

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{CONDA_PREFIX}")
endif()
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")

#################
# Library Build #
#################

option (SPICEQL_BUILD_LIB "Build the SpiceQL Library" ON)

if(SPICEQL_BUILD_LIB)

  set(JSON_BuildTests OFF CACHE INTERNAL "")

  add_subdirectory("submodules/gularkfilesystem")
  add_subdirectory("submodules/json")
  
  find_package(cspice REQUIRED CONFIG)
  find_package(fmt REQUIRED CONFIG)
  find_package(cereal REQUIRED CONFIG)
  find_package(spdlog REQUIRED CONFIG)
  find_package(HighFive REQUIRED)
  find_package(CURL REQUIRED)

  set(SPICEQL_INSTALL_INCLUDE_DIR "include/SpiceQL")
  set(SPICEQL_SRC_FILES   ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/spiceql.cpp 
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/utils.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/io.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/query.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/spice_types.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/memoized_functions.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/config.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/inventory.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/inventoryimpl.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/api.cpp)


  set(SPICEQL_HEADER_FILES ${SPICEQL_BUILD_INCLUDE_DIR}/spiceql.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/utils.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/memoized_functions.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/io.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/spice_types.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/query.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/config.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/inventory.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/inventoryimpl.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/api.h)

  set(SPICEQL_PRIVATE_HEADER_FILES ${SPICEQL_BUILD_INCLUDE_DIR}/memo.h
                                   ${SPICEQL_BUILD_INCLUDE_DIR}/restincurl.h)

  set(SPICEQL_CONFIG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/apollo15.json 
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/apollo16.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/apollo17.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/base.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/cassini.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/chandrayaan1.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/chandrayaan2.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/clem1.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/dawn.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/galileo.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/hayabusa.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/hayabusa2.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/juno.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/kaguya.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/lro.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/lo.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/mariner10.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/mer1.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/mer2.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/mess.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/mex.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/mgs.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/mro.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/msl.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/near.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/newhorizons.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/odyssey.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/rosetta.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/smart1.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/tgo.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/viking1.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/viking2.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/voyager1.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/voyager2.json
                           )

  set(SPICEQL_KERNELS ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/kernels/naif0011.tls)

  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/include/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/include/version.h @ONLY)
  add_library(SpiceQL SHARED ${SPICEQL_SRC_FILES})

  set_target_properties(SpiceQL PROPERTIES
                                  VERSION ${PROJECT_VERSION}
                                  SOVERSION 0)

  target_compile_definitions(SpiceQL PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE 
                                     PUBLIC -D_SOURCE_PREFIX="${CMAKE_CURRENT_SOURCE_DIR}")

get_target_property(CSPICE_INCLUDE_DIRS CSPICE::cspice INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(SpiceQL
                            PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/include>
                            $<INSTALL_INTERFACE:include>
                            PRIVATE
                            ${CMAKE_CURRENT_SOURCE_DIR}/submodules/BTree/include/
                            cereal
                            ${CSPICE_INCLUDE_DIRS}
                            )

  target_link_libraries(SpiceQL
                        PUBLIC
                        fmt::fmt-header-only
                        nlohmann_json::nlohmann_json
                        PRIVATE
                        CSPICE::cspice
                        spdlog::spdlog_header_only
                        HighFive
                        ${CURL_LIBRARIES}
                        )
   
  install(TARGETS SpiceQL LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(DIRECTORY ${SPICEQL_INCLUDE_DIR} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  # Generate the package config
  configure_file(cmake/config.cmake.in
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
                @ONLY)

  # Install the package config
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

  # Install the headers
  install(FILES ${SPICEQL_HEADER_FILES} DESTINATION ${SPICEQL_INSTALL_INCLUDE_DIR})

  # Install the json db files
  install(FILES ${SPICEQL_CONFIG_FILES} DESTINATION "etc/SpiceQL/db")

  # Install the shipped kernels
  install(FILES ${SPICEQL_KERNELS} DESTINATION "etc/SpiceQL/db/kernels")

  # Install the library
  install(TARGETS SpiceQL nlohmann_json ghc_filesystem
          EXPORT spiceQLTargets
          LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
          INCLUDES DESTINATION ${SPICEQL_INSTALL_INCLUDE_DIR})

  # Install the target
  install(EXPORT spiceQLTargets
          NAMESPACE spice::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

else()
  message(STATUS "Skipping Library")
endif()

###############
# Tests Build #
###############

cmake_dependent_option (SPICEQL_BUILD_TESTS "Build the SpiceQL Tests" ON SPICEQL_BUILD_LIB OFF)

if(SPICEQL_BUILD_TESTS)
  include(GoogleTest)
  include(cmake/gtest.cmake)
  include(CTest)

  find_package (Threads)
  enable_testing()
  add_subdirectory(SpiceQL/tests)
else()
  message(STATUS "Skipping Tests")
endif()

##################
# Bindings Build #
##################

cmake_dependent_option (SPICEQL_BUILD_BINDINGS "Build the SpiceQL Bindings in Other Languages" ON SPICEQL_BUILD_LIB OFF)

if(SPICEQL_BUILD_BINDINGS)
  add_subdirectory(bindings)
else()
  message(STATUS "Skipping Bindings")
endif()

