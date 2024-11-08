# FindNLopt.cmake

# Locate the NLopt library and headers
# This module defines the following variables:
#   NLOPT_FOUND, if false, do not try to use NLopt
#   NLOPT_INCLUDE_DIRS, where to find nlopt.h, etc.
#   NLOPT_LIBRARIES, the libraries to link against to use NLopt
#   NLOPT_LIBRARY_DIRS, the directories containing the libraries

find_path(NLOPT_INCLUDE_DIR
  NAMES nlopt.h
  PATHS ${NLopt_INSTALL_DIR}/include/
  /usr/local/lib
  /usr/lib
)

find_library(NLOPT_LIBRARY
  NAMES nlopt
  PATHS ${NLopt_INSTALL_DIR}/lib/
  /usr/local/lib
  /usr/lib
)

# Handle the QUIETLY and REQUIRED arguments and set NLOPT_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NLopt REQUIRED_VARS NLOPT_INCLUDE_DIR NLOPT_LIBRARY)
message(STATUS "JE CHERCHE NLOPTs")
message(STATUS ${NLOPT_INCLUDE_DIR})
message(STATUS ${NLOPT_LIBRARY})
message(STATUS ${NLopt_INSTALL_DIR})

if(NLOPT_FOUND)
  set(NLOPT_INCLUDE_DIRS ${NLOPT_INCLUDE_DIR})
  set(NLOPT_LIBRARIES ${NLOPT_LIBRARY})
  set(NLOPT_LIBRARY_DIRS ${NLopt_INSTALL_DIR}/lib)
else()
  message(FATAL_ERROR "Could not find NLopt")
endif()
