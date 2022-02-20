message(STATUS "The CMake version used is ${CMAKE_VERSION}")
cmake_minimum_required(VERSION 3.19...${CMAKE_VERSION})

project(cooker)

Include(FetchContent)

#set(CATCH_TAG "v2.11.1")
set(CATCH_TAG "v3.0.0-preview4")

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        ${CATCH_TAG}
)
FetchContent_MakeAvailable(Catch2)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else(MSVC)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
endif(MSVC)

add_subdirectory(tests)
enable_testing()