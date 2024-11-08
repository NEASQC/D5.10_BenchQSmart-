   
#CPLEX
#set(ENV{CPLEX_ROOT} "/logiciels/cplex127/")

  find_path(CPLEX_INCLUDE_DIR
    ilcplex/cplex.h
    PATHS $ENV{CPLEX_ROOT}/cplex/include/)
	
  find_library(CPLEX_LIBRARY
    libcplex.a
    PATHS $ENV{CPLEX_ROOT}/cplex/lib/x86-64_linux/static_pic)

  find_library(CPLEX_CXX_LIBRARY
    libilocplex.a
    PATHS $ENV{CPLEX_ROOT}/cplex/lib/x86-64_linux/static_pic)
    
  find_path(CPLEX_LIBRARY_DIRS
      NAMES libcplex.a
      PATHS $ENV{CPLEX_ROOT}/cplex/lib/x86-64_linux/static_pic)


# Concert

   find_path(CONCERT_INCLUDE_DIR
    ilconcert/cplexconcertdoc.h
    PATHS $ENV{CPLEX_ROOT}/concert/include)
 
   find_library(CPLEX_LIBRARY_CONCERT
      libconcert.a
      PATHS $ENV{CPLEX_ROOT}/concert/lib/x86-64_linux/static_pic)
      
   find_path(CPLEX_LIBRARY_CONCERT_DIRS
      libconcert.a
      PATHS $ENV{CPLEX_ROOT}/concert/lib/x86-64_linux/static_pic)
  
  add_definitions("-D_CPLEX_FOUND")
  mark_as_advanced(CPLEX_LIBRARY CPLEX_INCLUDE_DIR CPLEX_LIBRARY_DIRS CPLEX_LIBRARY_CONCERT)
  set(CPLEX_LIBRARIES ${CPLEX_CXX_LIBRARY} ${CPLEX_LIBRARY})

if (NOT ${CPLEX_INCLUDE_DIR} MATCHES "CPLEX_INCLUDE_DIR_NOTFOUND")
  message(STATUS "CPLEX_INCLUDE_DIR=${CPLEX_INCLUDE_DIR}")
else()
   message(SEND_ERROR "CPLEX_INCLUDE_DIR not found")
endif()
