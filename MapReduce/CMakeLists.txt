
#设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/MapReduce/bin)
#设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/MapReduce/bin)

aux_source_directory(. SRC_LIST)

set(MASTER_LIST masterExam.cpp master.cpp )

add_executable(masterserver ${MASTER_LIST})
target_link_libraries(masterserver zmq pthread)


set(MAPSERVER_LIST Map.cpp Mapexam.cpp)

add_executable(mapserver ${MAPSERVER_LIST})
target_link_libraries(mapserver zmq)


set(REDUCESERVER_LIST Reduce.cpp Reduceexam.cpp)

add_executable(reduceserver ${REDUCESERVER_LIST})
target_link_libraries(reduceserver zmq)
#生成可执行文件
#add_executable()


#生成库文件
#add_library(mrFunc SHARED map_reduceFun.cpp)


#target_link_libraries()