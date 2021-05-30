project(GSDEstimator)
cmake_minimum_required(VERSION 3.16.3)
include(ExternalProject)
find_package(Git REQUIRED)

set(OPENCL_INSTALL_DIR "opencl")

ExternalProject_Add(
    opencl-sdk
    PREFIX opencl-sdk
    GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-SDK
    INSTALL_DIR ${OPENCL_INSTALL_DIR}
    GIT_TAG main
    TIMEOUT 10
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -G ${CMAKE_GENERATOR}
)
link_directories(${CMAKE_BINARY_DIR}/${OPENCL_INSTALL_DIR}/lib)
include_directories(${CMAKE_BINARY_DIR}/opencl-sdk/src/opencl-sdk/external/OpenCL-Headers ".")


add_executable(GSDEstimator "main.cpp" "pal.cpp")
target_compile_definitions(GSDEstimator PUBLIC -DDEBUG=1)
target_link_libraries(GSDEstimator OpenCL)
add_dependencies(GSDEstimator opencl-sdk)
