project(area)

cmake_minimum_required(VERSION 2.4)

# Turn compiler warnings up to 11, at least with gcc.  
if (CMAKE_BUILD_TOOL MATCHES "make")
    MESSAGE(STATUS "setting gcc options: -Wall -Werror -Wno-deprecated -pedantic-errors")
    # NON-optimized build:
    # add_definitions( -Wall  -Wno-deprecated -Werror -pedantic-errors)
    add_definitions(-fPIC)
endif (CMAKE_BUILD_TOOL MATCHES "make")

option(BUILD_TYPE
  "Build type: Release=ON/Debug=OFF  " ON)

if (BUILD_TYPE)
    MESSAGE(STATUS " CMAKE_BUILD_TYPE = Release")
    set(CMAKE_BUILD_TYPE Release)
endif(BUILD_TYPE)

if (NOT BUILD_TYPE)
    MESSAGE(STATUS " CMAKE_BUILD_TYPE = Debug")
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT BUILD_TYPE)

# this figures out the Python include directories and adds them to the
# header file search path
execute_process(
    COMMAND python-config --includes
    COMMAND sed -r "s/-I//g; s/ +/;/g"
    COMMAND tr -d '\n'
    OUTPUT_VARIABLE Python_Includes
)
include_directories(${Python_Includes})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package( Boost COMPONENTS python REQUIRED)  # find BOOST and boost-python
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "found Boost: " ${Boost_LIB_VERSION})
    MESSAGE(STATUS "boost-incude dirs are: " ${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "boost-python lib is: " ${Boost_PYTHON_LIBRARY})
    MESSAGE(STATUS "boost_LIBRARY_DIRS is: " ${Boost_LIBRARY_DIRS})
    MESSAGE(STATUS "Boost_LIBRARIES is: " ${Boost_LIBRARIES})    
endif()

# this defines the source-files
set(AREA_SRC
	${area_SOURCE_DIR}/PythonStuff.cpp

    ${area_SOURCE_DIR}/Arc.cpp
    ${area_SOURCE_DIR}/Area.cpp
    ${area_SOURCE_DIR}/AreaBoolean.cpp
    ${area_SOURCE_DIR}/AreaDxf.cpp
    ${area_SOURCE_DIR}/AreaOrderer.cpp
    ${area_SOURCE_DIR}/AreaPocket.cpp
    ${area_SOURCE_DIR}/Circle.cpp
    ${area_SOURCE_DIR}/Curve.cpp
    ${area_SOURCE_DIR}/dxf.cpp
    
    ${area_SOURCE_DIR}/kbool/src/booleng.cpp
    ${area_SOURCE_DIR}/kbool/src/record.cpp
    ${area_SOURCE_DIR}/kbool/src/scanbeam.cpp
    ${area_SOURCE_DIR}/kbool/src/graph.cpp
    ${area_SOURCE_DIR}/kbool/src/graphlst.cpp
    ${area_SOURCE_DIR}/kbool/src/instonly.cpp
    ${area_SOURCE_DIR}/kbool/src/line.cpp
    ${area_SOURCE_DIR}/kbool/src/link.cpp
    ${area_SOURCE_DIR}/kbool/src/lpoint.cpp    
    ${area_SOURCE_DIR}/kbool/src/node.cpp
     
    ${area_SOURCE_DIR}/kurve/Construction.cpp
    ${area_SOURCE_DIR}/kurve/Finite.cpp
    ${area_SOURCE_DIR}/kurve/kurve.cpp
    ${area_SOURCE_DIR}/kurve/Matrix.cpp
    ${area_SOURCE_DIR}/kurve/offset.cpp
)

# include directories
include_directories( 
	${area_SOURCE_DIR} 
	${area_SOURCE_DIR}/kurve/
	${area_SOURCE_DIR}/kbool/src/
)

# this makes the Python module
add_library(
    area
    MODULE
    ${AREA_SRC}
)
target_link_libraries(area ${Boost_LIBRARIES} ) 
set_target_properties(area PROPERTIES PREFIX "") 


#
# this figures out where to install the Python modules
#
execute_process(
    COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"
    OUTPUT_VARIABLE Python_site_packages
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
MESSAGE(STATUS "Python libraries will be installed to: " ${Python_site_packages})

# this installs the python library
install(
	TARGETS area
	LIBRARY DESTINATION ${Python_site_packages}
)

