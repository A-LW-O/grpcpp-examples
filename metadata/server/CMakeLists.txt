find_package(absl REQUIRED)

file(GLOB srcs "*.cc")
set(target server)
add_executable(${target}
  ${srcs}
)

target_include_directories(${target} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT PROTO_LIB_NAME)
    message(FATAL_ERROR "PROTO_LIB_NAME is not defined.")
endif()
target_link_libraries(${target}
  ${PROTO_LIB_NAME}
  abseil::abseil
  ${HELPER_LIB_NAME}
)
