cmake_minimum_required(VERSION 3.10)
project(ClearMoon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 生成 compile_commands.json 供 clangd 等工具使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# integration test target
file(GLOB_RECURSE LIB_SRCS CONFIGURE_DEPENDS
	${CMAKE_SOURCE_DIR}/net/*.cc
	${CMAKE_SOURCE_DIR}/base/*.cc
)

add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

add_executable(HttpServer_test ${LIB_SRCS} ${CMAKE_SOURCE_DIR}/test/HttpServer_test.cc ${CMAKE_SOURCE_DIR}/test/HttpCallback.cc)

target_include_directories(HttpServer_test PRIVATE
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/net
	${CMAKE_SOURCE_DIR}/net/Http
	${CMAKE_SOURCE_DIR}/base
	${CMAKE_SOURCE_DIR}/net/Poller
	${CMAKE_SOURCE_DIR}/net/Log
)
set_target_properties(HttpServer_test PROPERTIES CXX_EXTENSIONS OFF)
