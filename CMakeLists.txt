project(GSDEstimator)
cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME GSDEstimator)
set(OPENCL_INSTALL_DIR "opencl")
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
  add_compile_options(/D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
endif()
include_directories(${CMAKE_SOURCE_DIR}/OpenCL-Headers ".")
add_subdirectory(${CMAKE_SOURCE_DIR}/OpenCL-ICD-Loader)

add_executable(${TARGET_NAME} "main.cpp" "pal.cpp")

if(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX /D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
  set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "/O3")
else()
  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

target_link_libraries(${TARGET_NAME} OpenCL)
