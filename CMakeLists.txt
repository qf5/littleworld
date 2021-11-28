cmake_minimum_required(VERSION 3.4.1)

project(littleworld)

set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/build)
SET(CMAKE_C_COMPILER g++)
SET(CMAK_CXX_COMPILER g++)
add_compile_options(-std=c++14)

include_directories(src/)
set(SRC src/log.cpp
		src/coroutine.cpp)

add_executable(test_make test/test_make.cpp)
add_executable(test_log ${SRC} test/test_log.cpp)
add_executable(test_coroutine ${SRC} test/test_coroutine.cpp)
