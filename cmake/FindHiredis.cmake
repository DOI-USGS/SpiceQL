
# CMake module for find_package(hiredis) the C redis API
# Finds include directory and all applicable libraries
#
# Sets the following:
#   HIREDIS_INCLUDE_DIR
#   HIREDIS_LIB

find_path(HIREDIS_INCLUDE_DIR
  NAME hiredis.h
  PATH_SUFFIXES hiredis
)

find_library(HIREDIS_LIB
  NAMES hiredis
)

message(STATUS "Hiredis INCLUDE: " ${HIREDIS_INCLUDE_DIR} )
message(STATUS "Hiredis LIB: "  ${HIREDIS_LIB} )