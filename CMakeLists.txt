cmake_minimum_required(VERSION 4.0)
project(ClearMoon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 生成 compile_commands.json 供 clangd 等工具使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
