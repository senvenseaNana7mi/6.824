cmake_minimum_required(VERSION 3.0)
project(6.824)

# 配置编译选项
set(CMAKE_BUILD_TYPE "Debug")

#设置头文件目录
include_directories(${PROJECT_SOURCE_DIR}/buttonrpc-master)
include_directories(${PROJECT_SOURCE_DIR})

# #设置项目库文件搜索路径
link_libraries(${PROJECT_SOURCE_DIR}/lib)

#设置库的输出路径
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#
#add_library(logger logger.cpp)

#add_subdirectory(MapReduce)
add_subdirectory(Raft)
