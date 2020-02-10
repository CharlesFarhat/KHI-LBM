cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(KHI-LBM VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -DPARALLEL_MODE_OMP -DOLB_PRECOMPILED -O3 -Wall -march=native -mtune=native -std=c++14")
set(EXECUTABLE_OUTPUT_PATH ./../bin)

#set openlb folder PLEASE SET HERE :
set(OpenLB /home/hades/ProgramFiles/OpenLB-1.3)

include_directories("${OpenLB}/src")
include_directories("${OpenLB}/src/external")
include_directories("${OpenLB}/src/external/zlib")

# set lib directory
LINK_DIRECTORIES(${OpenLB}/build/precompiled/lib)


add_executable(KHI-LBM ./src/LBM-Sim.cpp)
add_executable(RB-LBM src/RB-LBM.cpp)
add_executable(Cylinder src/Cylinder-LBM.cpp)
target_link_libraries(RB-LBM olb z)
target_link_libraries(KHI-LBM olb z)
target_link_libraries(Cylinder olb z)
