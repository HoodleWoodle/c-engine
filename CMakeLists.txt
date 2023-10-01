cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_STANDARD 99)

project(c-engine)

add_subdirectory(lib/glad/)
add_subdirectory(lib/glfw-3.3.8/)
add_subdirectory(lib/cglm-0.9.1/)

add_executable(engine
	src/main.c 
	src/engine.h
	src/engine.c
)

target_link_libraries(engine glfw glad cglm)