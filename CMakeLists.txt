project(GSDEstimator)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME GSDEstimator)
set(OPENCL_INSTALL_DIR "opencl")

include_directories(${CMAKE_SOURCE_DIR}/OpenCL-Headers ".")
add_subdirectory(${CMAKE_SOURCE_DIR}/OpenCL-ICD-Loader)

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
add_executable(${TARGET_NAME} "main.cpp" "pal.cpp")
target_compile_definitions(${TARGET_NAME} PUBLIC -DDEBUG=1)
target_link_libraries(${TARGET_NAME} OpenCL)

if(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
