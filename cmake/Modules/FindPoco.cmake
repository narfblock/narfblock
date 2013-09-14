# custom FindPoco module - Foundation and Util only

FIND_PATH(PocoFoundation_INCLUDE_DIR Poco/AbstractCache.h
  HINTS
  $ENV{POCO_ROOT}
  PATH_SUFFIXES include Foundation/include
  PATHS
  /opt
  /usr/local
  /usr
)
#MESSAGE("PocoFoundation_INCLUDE_DIR is ${PocoFoundation_INCLUDE_DIR}")

FIND_PATH(PocoUtil_INCLUDE_DIR Poco/AbstractCache.h
  HINTS
  $ENV{POCO_ROOT}
  PATH_SUFFIXES include Util/include
  PATHS
  /opt
  /usr/local
  /usr
)
#MESSAGE("PocoUtil_INCLUDE_DIR is ${PocoUtil_INCLUDE_DIR}")

FIND_LIBRARY(PocoFoundation_LIBRARY
  NAMES PocoFoundation
  HINTS
  $ENV{POCO_ROOT}
  PATH_SUFFIXES lib64 lib
  PATHS
  /opt
  /usr/local
  /usr
)
if (WIN32)
  SET(PocoFoundation_LIBRARY
    ${PocoFoundation_LIBRARY} iphlpapi
    )
endif()
#MESSAGE("PocoFoundation_LIBRARY is ${PocoFoundation_LIBRARY}")


FIND_LIBRARY(PocoUtil_LIBRARY
  NAMES PocoUtil
  HINTS
  $ENV{POCO_ROOT}
  PATH_SUFFIXES lib64 lib
  PATHS
  /opt
  /usr/local
  /usr
)
#MESSAGE("PocoUtil_LIBRARY is ${PocoUtil_LIBRARY}")
