cmake_minimum_required(VERSION 3.9.4)
project(CRfast VERSION 0.1.0)

file(GLOB source_files ${PROJECT_SOURCE_DIR}/src/*.cpp)

set(CMAKE_CXX_COMPILER "clang++")

add_executable(${PROJECT_NAME} ${source_files})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Werror -Wno-unused-variable -Wno-unused-but-set-variable")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
