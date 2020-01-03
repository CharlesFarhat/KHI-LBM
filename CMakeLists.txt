cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(KHI-LBM VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(EXECUTABLE_OUTPUT_PATH ./bin)

#set openlb folder PLEASE SET HERE :
set(OpenLB /home/hades/ProgramFiles/OpenLB-1.3)

include_directories("${OpenLB}/src")

# set lib directory
LINK_DIRECTORIES(${OpenLB}/build/precompiled/lib)


add_executable(KHI-LBM ./src/LBM-Sim.cpp)
target_link_libraries(KHI-LBM olb z)
