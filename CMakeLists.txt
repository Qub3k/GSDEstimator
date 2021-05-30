project(GSDEstimator)
cmake_minimum_required(VERSION 3.0)
include(ExternalProject)
find_package(Git REQUIRED)

set(OPENCL_INSTALL_DIR "opencl")

include_directories(${CMAKE_SOURCE_DIR}/OpenCL-Headers ".")
add_subdirectory(${CMAKE_SOURCE_DIR}/OpenCL-ICD-Loader)

add_executable(GSDEstimator "main.cpp" "pal.cpp")
target_compile_definitions(GSDEstimator PUBLIC -DDEBUG=1)
target_link_libraries(GSDEstimator OpenCL)
