project(GSDEstimator)
cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME GSDEstimator)
set(OPENCL_INSTALL_DIR "opencl")

include_directories(${CMAKE_SOURCE_DIR}/OpenCL-Headers ".")
add_subdirectory(${CMAKE_SOURCE_DIR}/OpenCL-ICD-Loader)

add_executable(${TARGET_NAME} "main.cpp" "pal.cpp")

if(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
  set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "/Ox")
else()
  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

target_link_libraries(${TARGET_NAME} OpenCL)
