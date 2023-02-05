# Locate libzip
# This module defines
# LIBZIP_LIBRARY
# LIBZIP_FOUND, if false, do not try to link to libzip
# LIBZIP_INCLUDE_DIR, where to find the headers
#

find_path(LIBZIP_INCLUDE_DIR
    NAMES zip.h
    HINTS $ENV{LIBZIP_DIR} ${LIBZIP_DIR}
    PATH_SUFFIXES include
)
find_library(LIBZIP_LIBRARY
    NAMES libzip zip
    HINTS $ENV{LIBZIP_DIR} ${LIBZIP_DIR}
    PATH_SUFFIXES lib
)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(LibZip
    REQUIRED_VARS LIBZIP_LIBRARY LIBZIP_INCLUDE_DIR
)

SET(LIBZIP_FOUND "NO")
IF(LIBZIP_LIBRARY AND LIBZIP_INCLUDE_DIR)
    SET(LIBZIP_FOUND "YES")
ENDIF(LIBZIP_LIBRARY AND LIBZIP_INCLUDE_DIR)

mark_as_advanced(LIBZIP_LIBRARY LIBZIP_INCLUDE_DIR)
