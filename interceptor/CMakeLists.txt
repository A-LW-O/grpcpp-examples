cmake_minimum_required(VERSION 3.15)

project(app VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_COMPILER "g++")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(proto)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/helper")
    add_subdirectory(helper)
endif()
add_subdirectory(server)
add_subdirectory(client)
