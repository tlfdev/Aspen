project (aspen)
set (CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -pedantic")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set (aspen_VERSION_MAJOR 1)
set (aspen_VERSION_MINOR 0)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/BuildConfig.h.in"
  "${PROJECT_BINARY_DIR}/BuildConfig.h")
include_directories("${PROJECT_BINARY_DIR}"
${CMAKE_CURRENT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp
)
add_subdirectory(xml)
add_subdirectory(angelscript)
add_subdirectory(jsoncpp)
add_subdirectory(src)
