cmake_minimum_required(VERSION 3.16)
project(RaymarchingSeasons LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RaymarchingSeasons
	src/main.cpp
	src/shader.cpp
	src/controls.cpp
	src/shapes.cpp
	src/heightmap.cpp
	src/texture.cpp
	src/gpuTimer.cpp
)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw-3.4)

# GLEW
set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
add_subdirectory(
	external/glew-2.3.1/build/cmake
	external/glew-build
)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
add_library(imgui STATIC
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
	${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
	${IMGUI_DIR}
	${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)
# Others
target_include_directories(RaymarchingSeasons PRIVATE
	external/glew-2.3.1/include
	external/glm
	include
)

target_link_libraries(RaymarchingSeasons PRIVATE
	glfw
	glew_s
	imgui
)
