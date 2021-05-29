project(GSDEstimator)
cmake_minimum_required(VERSION 2.8.8)
include(ExternalProject)
find_package(Git REQUIRED)

set( OPENCL_INSTALL_DIR "opencl")

ExternalProject_Add(
    opencl-sdk
    PREFIX opencl-sdk
    GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-SDK
    INSTALL_DIR ${OPENCL_INSTALL_DIR}
    GIT_TAG main
    TIMEOUT 10
    CONFIGURE_COMMAND cmake -S ../opencl-sdk -B . -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
link_directories(${CMAKE_BINARY_DIR}/${OPENCL_INSTALL_DIR}/lib)
include_directories(${CMAKE_BINARY_DIR}/opencl-sdk/src/opencl-sdk/include/api ".")


add_executable(GSDEstimator "main.cpp" "pal.cpp")
target_compile_definitions(GSDEstimator PUBLIC -DDEBUG=1)
target_link_libraries(GSDEstimator OpenCL)
add_dependencies(GSDEstimator opencl-sdk)
