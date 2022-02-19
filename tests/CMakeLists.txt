
cmake_minimum_required(VERSION 3.19)

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

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

foreach (filename catch_amalgamated.hpp catch_amalgamated.cpp)
  file(COPY  ${catch2_SOURCE_DIR}/extras/${filename} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR} )
endforeach()

add_executable(tests test.cpp)

target_link_libraries(tests PRIVATE Catch2::Catch2)
if (TARGET Catch2::Catch2WithMain)
    # link Catch2::Catch2Main only if it exists (so probably catch2 v3)
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
endif()


#add_test(sm_test tests)

include(CTest)
include(Catch)
catch_discover_tests(tests)