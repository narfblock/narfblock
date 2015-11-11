FIND_PATH(LibOgg_INCLUDE_DIR ogg.h PATH_SUFFIXES include/ogg include)
FIND_LIBRARY(LibOgg_LIBRARY NAMES ogg PATH_SUFFIXES lib lib64)

SET(LibOgg_INCLUDE_DIRS ${LibOgg_INCLUDE_DIR})
SET(LibOgg_LIBRARIES ${LibOgg_LIBRARY})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibOgg
    REQUIRED_VARS LibOgg_LIBRARY LibOgg_INCLUDE_DIRS)

MARK_AS_ADVANCED(LibOgg_INCLUDE_DIR LibOgg_LIBRARY)