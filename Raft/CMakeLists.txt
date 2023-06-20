#设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Raft/bin)


aux_source_directory(. SRC_LIST)

set(RAFT_LIST raft.cpp rafttest.cpp ../logger.cpp)

add_executable(raft ${RAFT_LIST})
#设置项目库文件搜索路径
link_libraries(${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(raft zmq pthread)

