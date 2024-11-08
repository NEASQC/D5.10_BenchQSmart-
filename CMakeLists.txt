cmake_minimum_required(VERSION 3.13.4)
project(Quantum_BnP)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-DIL_STD") 
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

###################################################################################################################################
###################################################### Path Library settings ######################################################
###################################################################################################################################

set(NLopt_INSTALL_DIR "${PROJECT_SOURCE_DIR}/nlopt_install")
option(QB_ENABLE_SCIP "Use of SCIP library" OFF)
set(SCIP_DIR YOU_PATH_TO_SCIP)


option(QP_ENABLE_CPLEX "Use of CPLEX solver"	OFF)
set(ENV{CPLEX_ROOT} YOU_PATH_TO_CPLEX)


############################################################################################################
############################################################################################################
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})


if (QB_ENABLE_SCIP)
	find_package(SCIP REQUIRED)
endif()

if(QP_ENABLE_CPLEX)
	
	add_library(CPLEX INTERFACE IMPORTED)
	FIND_PACKAGE(CPLEX REQUIRED)
	set(ADDITIONAL_LIBRARIES
	${CPLEX_LIBRARY}
	${CPLEX_LIBRARY_CONCERT})

	link_directories(${CPLEX_INCLUDE_DIRS})
	link_directories(${CPLEX_LIBRARIES})
endif()






list(APPEND COLORING coloring/Branching.cpp coloring/Branching.h coloring/coloring.cpp coloring/coloring.h coloring/ConstraintHandler.cpp coloring/ConstraintHandler.h coloring/Pricer.cpp coloring/Pricer.h coloring/Probdata.cpp coloring/Probdata.h coloring/Vardata.cpp coloring/Vardata.h)
list(APPEND MWIS mwis/greedy.cpp mwis/LocalSearch.cpp mwis/LocalSearch.h mwis/mwis.h mwis/cplex.cpp)
list(APPEND BASICS Graph.h Graph.cpp)
list(APPEND QUANTUM quantum/Hamiltonian.h quantum/Hamiltonian.cpp)
file(GLOB SOURCE coloring/*)



find_package(OpenMP)
find_library(OPENBLAS openblas)
find_package(NLopt REQUIRED)

set(PROJECT_INCLUDE_DIRS 
	${CMAKE_CURRENT_SOURCE_DIR}/mwis
    ${CMAKE_CURRENT_SOURCE_DIR}/coloring
    ${CMAKE_CURRENT_SOURCE_DIR}/quantum
	${CPLEX_INCLUDE_DIR}
	${CPLEX_INCLUDE_DIR}/ilcplex
	${CONCERT_INCLUDE_DIR} 
	${CONCERT_INCLUDE_DIR}/ilconcert
	${NLOPT_INCLUDE_DIRS})

foreach(include_dir ${PROJECT_INCLUDE_DIRS})
	include_directories(${include_dir})
	file(GLOB tmp_headers
          ${include_dir}/*.hpp
          ${include_dir}/*.hx
          ${include_dir}/*.h)
    list(APPEND header_files ${tmp_headers})  
endforeach()

    
link_directories(${NLOPT_LIBRARY_DIRS})
link_directories(${NLopt_INSTALL_DIR}/lib)


add_executable(Quantum_BnP main.cpp ${BASICS} ${MWIS} ${QUANTUM} ${COLORING} ${CPLEX_INCLUDE_DIRS} ${ADDITIONAL_LIBRARIES} )
target_compile_definitions(Quantum_BnP INTERFACE -DIL_STD)
target_link_libraries(Quantum_BnP PUBLIC ${CMAKE_DL_LIBS} ${ADDITIONAL_LIBRARIES})

target_link_libraries(Quantum_BnP PUBLIC nlopt)


if(QP_ENABLE_CPLEX)
	target_link_libraries(Quantum_BnP INTERFACE ${CPLEX_LIBRARIES} pthread)
	target_include_directories(Quantum_BnP INTERFACE ${CPLEX_INCLUDE_DIRS})
	target_link_libraries(Quantum_BnP PUBLIC ${CPLEX_LIBRARIES})
endif()

if(QP_ENABLE_SCP)
	target_link_libraries(Quantum_BnP PUBLIC ${SCIP_LIBRARIES})
endif()
