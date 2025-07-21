cmake_minimum_required(VERSION 3.12)
project(fs-log-viewer)
add_subdirectory(engine)

option(FS_INCLUDE_ASSIMP OFF)
option(FS_INCLUDE_FT OFF)

add_executable(fs-log-viewer
	include/cfg.hpp
	include/log_parser.hpp
	include/recent.hpp
	main.cpp
)
target_link_libraries(fs-log-viewer
	firesteel
	glad
	imgui
	glm
)
target_compile_features(fs-log-viewer PUBLIC cxx_std_17)
target_include_directories(fs-log-viewer PUBLIC includes)
target_include_directories(fs-log-viewer PUBLIC engine/external/glfw)
set_target_properties(fs-log-viewer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)