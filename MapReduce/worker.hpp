#pragma once
#include "map_reduceFun.h"
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>

#include <unistd.h>

// 记录reduce线程数
int reduce_task_num;

// 对每个字符串求hash找到其对应要分配的reduce线程，感觉像是shuffle那一步
int ihash(string &str);

// 根据filename，不好用输入输出流，用linux内部的write
KeyValue getContent(string &filename);


//将mapfunc处理好的key-value键值对放到合适的文件中达到合适的效果
