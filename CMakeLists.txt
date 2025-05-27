cmake_minimum_required(VERSION 3.22)
project(EIS CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# If no build type is specified, set Release as the default
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# project
add_subdirectory(src)


#optionally use googles improved hashmap
find_path(SPARSEHASH_INCLUDE_DIR sparsehash/dense_hash_map)
if(SPARSEHASH_INCLUDE_DIR)
    message(STATUS "SparseHash found at ${SPARSEHASH_INCLUDE_DIR}")
    add_definitions(-DUSE_SPARSEHASH)
else()
    message(STATUS "SparseHash not found, falling back to STL containers")
endif()



#targets
add_executable(EIS
   src/main_EIS.cpp
   src/graph.cpp
   src/EIS_sample.cpp
   src/bicoloredGraph.cpp)
target_include_directories(EIS
   PRIVATE
   ${PROJECT_SOURCE_DIR}/include)

add_executable(EISm
   src/main_EISm.cpp
   src/graph.cpp
   src/EIS_sample.cpp
   src/bicoloredGraph.cpp)
target_include_directories(EISm
   PRIVATE
   ${PROJECT_SOURCE_DIR}/include)

add_executable(NIS
   src/main_NIS.cpp
   src/graph.cpp
   src/EIS_sample.cpp
   src/bicoloredGraph.cpp)
target_include_directories(NIS
   PRIVATE
   ${PROJECT_SOURCE_DIR}/include)

add_executable(3ES
   src/main_3ES.cpp
   src/graph.cpp
   src/EIS_sample.cpp
   src/bicoloredGraph.cpp)
target_include_directories(3ES
   PRIVATE
   ${PROJECT_SOURCE_DIR}/include)


if(SPARSEHASH_INCLUDE_DIR)
   target_include_directories(EIS PRIVATE ${SPARSEHASH_INCLUDE_DIR})
   target_include_directories(EISm PRIVATE ${SPARSEHASH_INCLUDE_DIR})
   target_include_directories(NIS PRIVATE ${SPARSEHASH_INCLUDE_DIR})
   target_include_directories(3ES PRIVATE ${SPARSEHASH_INCLUDE_DIR})
endif()
