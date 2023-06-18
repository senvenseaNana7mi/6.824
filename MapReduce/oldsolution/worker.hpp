#pragma once
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include "locker.hpp"
#include <bits/stdc++.h>
#include <pthread.h>
#include <dlfcn.h>
using namespace std;

class KeyValue{
public:
    string key;
    string value;
};



//对每个字符串求hash找到其对应要分配的reduce线程
int ihash(string str);


//删除所有写入中间值的临时文件
void removeFiles();

//取得  key:filename, value:content 的kv对作为map任务的输入
KeyValue getContent(char* file);


//将map任务产生的中间值写入临时文件
void writeKV(int fd, const KeyValue& kv);

//创建每个map任务对应的不同reduce号的中间文件并调用 -> writeKV 写入磁盘
void writeInDisk(const vector<KeyValue>& kvs, int mapTaskIdx);

//以char类型的op为分割拆分字符串
vector<string> split(string text, char op);

//以逗号为分割拆分字符串，重载版本
string split(string text);

//获取对应reduce编号的所有中间文件
vector<string> getAllfile(string path, int op);


//对于一个ReduceTask，获取所有相关文件并将value的list以string写入vector
//vector中每个元素的形式为"abc 11111";
vector<KeyValue> Myshuffle(int reduceTaskNum);


void* mapWorker(void* arg);

//用于最后写入磁盘的函数，输出最终结果
void myWrite(int fd, vector<string>& str);

void* reduceWorker(void* arg);

//删除最终输出文件，用于程序第二次执行时清除上次保存的结果
void removeOutputFiles();