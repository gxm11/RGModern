# - Try to find libffi define the variables for the binaries/headers and include 
#
# Once done this will define
#  libffi_FOUND - System has libffi
#  libffi::libffi - Imported target for the libffi

find_library(FFI_LIBRARY NAMES ffi libffi
    PATHS $ENV{FFI_DIR}
    PATH_SUFFIXES lib
    HINTS ${PC_LIBFFI_LIBDIR} ${PC_LIBFFI_LIBRARY_DIRS}
)

find_path(FFI_INCLUDE_DIR ffi.h
    PATHS $ENV{FFI_DIR}
    PATH_SUFFIXES include include/ffi
    HINTS ${PC_LIBFFI_INCLUDEDIR} ${PC_LIBFFI_INCLUDE_DIRS}
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set libffi_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libffi DEFAULT_MSG
    FFI_LIBRARY FFI_INCLUDE_DIR
)
mark_as_advanced(FFI_INCLUDE_DIR FFI_LIBRARY)

set(FFI_LIBRARIES ${FFI_LIBRARY})
set(FFI_INCLUDE_DIRS ${FFI_INCLUDE_DIR})


if(libffi_FOUND AND NOT TARGET libffi::libffi)
    add_library(libffi::libffi UNKNOWN IMPORTED)
    set_target_properties(libffi::libffi PROPERTIES
        IMPORTED_LOCATION "${FFI_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FFI_INCLUDE_DIR}"
    )
endif()
