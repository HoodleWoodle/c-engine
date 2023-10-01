project(glad)

add_library(${PROJECT_NAME} STATIC "${PROJECT_SOURCE_DIR}/glad.c")

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}")