include(CMakeDependentOption)
cmake_minimum_required(VERSION 3.10)
project(SpiceQL VERSION 1.6.0 DESCRIPTION "Spice Query Library")

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
set(SPICEQL_BUILD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/include/SpiceQL")

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

# WebAssembly (Emscripten) build. Auto-enable when configured through emcmake,
# which sets the EMSCRIPTEN variable. In this mode HDF5/HighFive and libcurl are
# excluded, cspice is built from source, and the library is built STATIC.
option (SPICEQL_WASM "Build SpiceQL for WebAssembly via Emscripten" OFF)
if(EMSCRIPTEN)
  set(SPICEQL_WASM ON CACHE BOOL "Build SpiceQL for WebAssembly via Emscripten" FORCE)
endif()

# Note: the WASM build does not support the remote REST transport (useWeb=true).
# Awaiting the async fetch() would require suspending the wasm stack (JSPI), which
# through Embind forces the whole JS API to become async, and ASYNCIFY=1 conflicts
# with the -fwasm-exceptions ABI CSPICE needs. useWeb=true therefore throws in the
# WASM build (see spiceAPIQuery in SpiceQL/src/api.cpp); call the SpiceQL REST
# service directly from JavaScript if you need it.

if(SPICEQL_BUILD_LIB)

  set(JSON_BuildTests OFF CACHE INTERNAL "")

  add_subdirectory("submodules/gularkfilesystem")
  add_subdirectory("submodules/json")

  if(SPICEQL_WASM)
    # No conda under emcmake: build cspice from source and fetch the header-only
    # deps. HDF5/HighFive and CURL are intentionally excluded from this build.
    include(FetchContent)
    include(BuildCspiceWasm)

    find_package(fmt CONFIG QUIET)
    if(NOT fmt_FOUND)
      FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 11.1.4)
      FetchContent_MakeAvailable(fmt)
    endif()

    find_package(cereal CONFIG QUIET)
    if(NOT cereal_FOUND)
      set(BUILD_DOC OFF CACHE INTERNAL "")
      set(BUILD_SANDBOX OFF CACHE INTERNAL "")
      set(SKIP_PERFORMANCE_COMPARISON ON CACHE INTERNAL "")
      FetchContent_Declare(cereal
        GIT_REPOSITORY https://github.com/USCiLab/cereal.git
        GIT_TAG v1.3.2)
      FetchContent_MakeAvailable(cereal)
    endif()
  else()
    find_package(cspice REQUIRED)
    find_package(fmt CONFIG REQUIRED)
    find_package(cereal CONFIG REQUIRED)
    find_package(HighFive REQUIRED)
    find_package(CURL REQUIRED)
  endif()

  set(SPICEQL_INSTALL_INCLUDE_DIR "include/SpiceQL")
  set(SPICEQL_SRC_FILES   ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/spiceql.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/utils.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/io.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/query.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/spice_types.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/memoized_functions.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/config.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/api.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/alias_map.cpp)

  if(SPICEQL_WASM)
    # HDF5-backed inventory is excluded; inventory_wasm.cpp provides the same
    # Inventory:: symbols (kernel search throws; explicit kernelLists resolve
    # by extension). See SpiceQL/src/inventory_wasm.cpp.
    list(APPEND SPICEQL_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/inventory_wasm.cpp)
  else()
    list(APPEND SPICEQL_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/inventory.cpp
                                  ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/src/inventoryimpl.cpp)
  endif()


  set(SPICEQL_HEADER_FILES ${SPICEQL_BUILD_INCLUDE_DIR}/spiceql.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/utils.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/memoized_functions.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/io.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/spice_types.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/query.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/config.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/inventory.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/inventoryimpl.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/api.h
                           ${SPICEQL_BUILD_INCLUDE_DIR}/alias_map.h)

  set(SPICEQL_PRIVATE_HEADER_FILES ${SPICEQL_BUILD_INCLUDE_DIR}/memo.h
                                   ${SPICEQL_BUILD_INCLUDE_DIR}/restincurl.h)

  set(SPICEQL_ALIASMAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/aliasMap.json)

  set(SPICEQL_CONFIG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/apollo15.json 
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/apollo16.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/apollo17.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/base.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/cassini.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/chandrayaan1.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/chandrayaan2.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/clem1.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/clipper.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/dawn.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/galileo.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/hayabusa.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/hayabusa2.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/juno.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/kaguya.json
                           ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/kplo.json
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

  set(SPICEQL_KERNELS ${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/db/kernels/naif0012.tls)

  # Generate the version header into the build tree (not the source tree) and
  # reach it as <SpiceQL/spiceql_version.h> via the build-dir include path below.
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/include/SpiceQL/spiceql_version.h.in ${CMAKE_BINARY_DIR}/SpiceQL/spiceql_version.h @ONLY)
  # Emscripten links archives, so build STATIC for the WASM target; SHARED otherwise.
  if(SPICEQL_WASM)
    add_library(SpiceQL STATIC ${SPICEQL_SRC_FILES})
  else()
    add_library(SpiceQL SHARED ${SPICEQL_SRC_FILES})
  endif()

  set_target_properties(SpiceQL PROPERTIES
                                  VERSION ${PROJECT_VERSION}
                                  SOVERSION 0)

  # spdlog dependency removed: see SpiceQL/include/spiceql_logging.h. Logging is
  # now a small self-contained logger using fmt, so no SPDLOG_ACTIVE_LEVEL is
  # needed.
  if(SPICEQL_WASM)
    target_compile_definitions(SpiceQL PUBLIC -D_SOURCE_PREFIX="/spiceql")
    target_compile_definitions(SpiceQL PUBLIC SPICEQL_WASM)
    target_compile_options(SpiceQL PUBLIC -fPIC)
    target_compile_options(SpiceQL PUBLIC -fwasm-exceptions)
    target_link_options(SpiceQL PUBLIC -fwasm-exceptions)
  else()
    target_compile_definitions(SpiceQL PUBLIC -D_SOURCE_PREFIX="${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  # On Windows, prevent <windows.h> (pulled in by HDF5/HighFive) from defining
  # the min/max macros, which otherwise break std::min/std::max and headers
  # such as HighFive that use them.
  if(WIN32)
    target_compile_definitions(SpiceQL PUBLIC NOMINMAX)
  endif()

get_target_property(CSPICE_INCLUDE_DIRS CSPICE::cspice INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(SpiceQL
                            PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SpiceQL/include>
                            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                            $<INSTALL_INTERFACE:include>
                            PRIVATE
                            ${CMAKE_CURRENT_SOURCE_DIR}/submodules/BTree/include/
                            ${CMAKE_CURRENT_SOURCE_DIR}/submodules/gularkfilesystem/include/
                            ${CMAKE_CURRENT_SOURCE_DIR}/submodules/utcet/
                            cereal
                            ${CSPICE_INCLUDE_DIRS}
                            )

  if(SPICEQL_WASM)
    # WASM: no HighFive/HDF5, no libcurl (REST goes through JS fetch).
    target_link_libraries(SpiceQL
                          PUBLIC
                          fmt::fmt-header-only
                          nlohmann_json::nlohmann_json
                          cereal::cereal
                          PRIVATE
                          CSPICE::cspice
                          )
  else()
    target_link_libraries(SpiceQL
                          PUBLIC
                          fmt::fmt-header-only
                          nlohmann_json::nlohmann_json
                          cereal::cereal
                          PRIVATE
                          CSPICE::cspice
                          HighFive
                          ${CURL_LIBRARIES}
                          )
  endif()
   
  install(TARGETS SpiceQL LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

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
  install(FILES ${SPICEQL_ALIASMAP_FILE} DESTINATION "etc/SpiceQL")

  # Install the json db files
  install(FILES ${SPICEQL_CONFIG_FILES} DESTINATION "etc/SpiceQL/db")

  # Install the shipped kernels
  install(FILES ${SPICEQL_KERNELS} DESTINATION "etc/SpiceQL/db/kernels")

  # Install the library
  install(TARGETS SpiceQL
          LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR})

else()
  message(STATUS "Skipping Library")
endif()

###############
# Tests Build #
###############

# The gtest suite is not built for WASM (default OFF there); native default ON.
if(SPICEQL_WASM)
  set(SPICEQL_TESTS_DEFAULT OFF)
else()
  set(SPICEQL_TESTS_DEFAULT ON)
endif()
cmake_dependent_option (SPICEQL_BUILD_TESTS "Build the SpiceQL Tests" ${SPICEQL_TESTS_DEFAULT} SPICEQL_BUILD_LIB OFF)

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
  if(SPICEQL_WASM)
    # WASM: build the Emscripten/Embind JS bindings instead of the SWIG Python ones.
    add_subdirectory(bindings/wasm)
  else()
    add_subdirectory(bindings)
  endif()
else()
  message(STATUS "Skipping Bindings")
endif()

