#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "msgpackc" for configuration "RelWithDebInfo"
set_property(TARGET msgpackc APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(msgpackc PROPERTIES
  IMPORTED_IMPLIB_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/msgpackc_import.lib"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/msgpackc.dll"
  )

list(APPEND _cmake_import_check_targets msgpackc )
list(APPEND _cmake_import_check_files_for_msgpackc "${_IMPORT_PREFIX}/lib/msgpackc_import.lib" "${_IMPORT_PREFIX}/bin/msgpackc.dll" )

# Import target "msgpackc-static" for configuration "RelWithDebInfo"
set_property(TARGET msgpackc-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(msgpackc-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/msgpackc.lib"
  )

list(APPEND _cmake_import_check_targets msgpackc-static )
list(APPEND _cmake_import_check_files_for_msgpackc-static "${_IMPORT_PREFIX}/lib/msgpackc.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
