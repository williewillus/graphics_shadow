SET(pwd ${CMAKE_CURRENT_LIST_DIR})

SET(src "")
AUX_SOURCE_DIRECTORY(${pwd} src)
add_executable(shadow ${src})
message(STATUS "shadow added ${src}")

target_link_libraries(shadow ${stdgl_libraries})
FIND_PACKAGE(JPEG REQUIRED)
TARGET_LINK_LIBRARIES(shadow ${JPEG_LIBRARIES})
