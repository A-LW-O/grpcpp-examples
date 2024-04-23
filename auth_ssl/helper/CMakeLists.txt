file(GLOB srcs "*.cc")
set(target helper)
add_library(${target}
  ${srcs}
)

target_include_directories(${target}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT PROTO_LIB_NAME)
    message(FATAL_ERROR "PROTO_LIB_NAME is not defined.")
endif()
target_link_libraries(${target}
  ${PROTO_LIB_NAME}
)

set(HELPER_LIB_NAME "${target}" CACHE STRING "The name of helper library")
