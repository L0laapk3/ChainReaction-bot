cmake_minimum_required(VERSION 3.9.4)
project(tb2 VERSION 0.1.0)
set(CMAKE_VERBOSE_MAKEFILE ON)

file(GLOB source_files ${PROJECT_SOURCE_DIR}/src/*.cpp)

set(CMAKE_CPP_COMPILER clang++)

add_executable(${PROJECT_NAME} ${source_files})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_CXX_FLAGS "-march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -D_DLL")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use=${PROJECT_SOURCE_DIR}/tb6.profdata")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
