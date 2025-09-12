#JSONCPP library
# from https://github.com/open-source-parsers/jsoncpp
project (JsonCPP)
set(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_C_FLAGS "-O3")
add_library(jsoncpp
jsoncpp.cpp
)
