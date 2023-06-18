#pragma once
#include <fcntl.h>
#include <iostream>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <bits/stdc++.h>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../rpc/include/mprpcapplication.h"
#include "../rpc/include/rpcprovider.h"
#include "locker.hpp"
#include "masterRPC.pb.h"
#include <google/protobuf/port_def.inc>

using namespace std;

class Master : public fixbug::MapReduceRpc
{
  public:
    static void waitMapTask(Master *master);    // 回收map的定时线程
    static void waitReduceTask(Master *master); // 回收reduce的定时线程
    static void waitTime(char *arg);            // 用于定时的线程
    Master(int mapNum = 8, int reduceNum = 8);  // 带缺省值的狗在函数
    void GetAllFile(char *file[], int index); // 从argv[]中获取待处理的文件名

    int getMapNum()
    {
        return m_mapNum;
    }
    int getReduceNum()
    {
        return m_reduceNum;
    }
    string assignTask();              // 分配map任务的函数，RPC
    int assignReduceTask();           // 分配reduce任务的函数，RPC
    void setMapStat(string filename); // 设置特定map任务完成的函数，RPC
    bool isMapDone();                 // 检验所有map任务是否完成，RPC
    void setReduceStat(int taskIndex); // 设置特定reduce任务完成的函数，RPC
    void waitMap(string filename);
    void waitReduce(int reduceIdx);
    bool Done(); // 判断reduce任务是否已经完成
    bool getFinalStat()
    { // 所有任务是否完成，实际上reduce完成就完成了，有点小重复
        return m_done;
    }

    virtual void assignTask(::PROTOBUF_NAMESPACE_ID::RpcController *controller,
                            const PROTOBUF_NAMESPACE_ID::Empty *request,
                            ::fixbug::Mystring *response,
                            ::google::protobuf::Closure *done)
    {
        response->set_str(assignTask());
    }
    virtual void assignReduceTask(
        ::PROTOBUF_NAMESPACE_ID::RpcController *controller,
        const PROTOBUF_NAMESPACE_ID::Empty *request, ::fixbug::Myint *response,
        ::google::protobuf::Closure *done)
    {
        response->set_it(assignReduceTask());
    }
    virtual void setMapStat(::PROTOBUF_NAMESPACE_ID::RpcController *controller,
                            const ::fixbug::Mystring *request,
                            PROTOBUF_NAMESPACE_ID::Empty *response,
                            ::google::protobuf::Closure *done)
    {
        setMapStat(request->str());
    }
    virtual void isMapDone(::PROTOBUF_NAMESPACE_ID::RpcController *controller,
                           const PROTOBUF_NAMESPACE_ID::Empty *request,
                           ::fixbug::Mybool *response,
                           ::google::protobuf::Closure *done)
    {
        response->set_bl(isMapDone());
    }
    virtual void setReduceStat(
        ::PROTOBUF_NAMESPACE_ID::RpcController *controller,
        const ::fixbug::Myint *request, PROTOBUF_NAMESPACE_ID::Empty *response,
        ::google::protobuf::Closure *done)
    {
        setReduceStat(request->it());
    }
    virtual void Done(::PROTOBUF_NAMESPACE_ID::RpcController *controller,
                      const PROTOBUF_NAMESPACE_ID::Empty *request,
                      ::fixbug::Mybool *response,
                      ::google::protobuf::Closure *done)
    {
        response->set_bl(Done());
    }

  private:
    bool m_done;
    list<char *> m_list; // 所有map任务的待工作队列
    mutex m_assign_lock; // 保护所有共享数据的锁
    int fileNum;         // 从命令行读取到的文件总数
    int m_mapNum;
    int m_reduceNum;

    unordered_map<string, int>
        finishedMapTask; // 存放所有完成的map任务对应的文件名
    unordered_map<int, int>
        finishedReduceTask; // 存放所有完成reduce任务对应的reduce编号
    vector<int> reduceIndex; // 所有reduce任务的工作队列
    vector<string>
        runningMapWork; // 正在处理的map任务，分配出去就加到这个队列，用于判断超时处理重发
    int curMapIndex;    // 当前处理第几个任务
    int curReduceIndex; // 当前处理第几个reduce任务
    vector<int>
        runningReduceWork; // 正在处理的reduce任务，分配出去就加到这个队列，用于判断超时处理重发
};