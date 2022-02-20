list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

foreach (filename catch_amalgamated.hpp catch_amalgamated.cpp)
  file(COPY  ${catch2_SOURCE_DIR}/extras/${filename} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR} )
endforeach()

# if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
#   message(STATUS "Set default build type to Release")
#  	set(CMAKE_BUILD_TYPE Release)
# endif()

add_executable(tests test.cpp)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else(MSVC)
target_compile_features(tests PRIVATE cxx_std_17)
endif(MSVC)

target_include_directories(tests PRIVATE ${PROJECT_SOURCE_DIR}/Core/Inc)

target_link_libraries(tests PRIVATE Catch2::Catch2)
if (TARGET Catch2::Catch2WithMain)
    # link Catch2::Catch2Main only if it exists (so probably catch2 v3)
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
endif()

include(CTest)
include(Catch)
catch_discover_tests(tests)
enable_testing()