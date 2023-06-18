#pragma once 

#include <cstdint>
#include <list>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

#include "keyValue.hpp"

/*
    作为master，进行任务的分发，并提供任务所需的内容。在这里主要是分配文件，在实际应用中文件可能会储存在GFS文件管理上，而
    该lab主要是训练作用，便用直接用本地的文件。master直接给map服务器文件名以达到分配任务的效果
*/

//定义超时时长，先定义10s
#define maxMapTime 10
#define maxReduceTime 10


//因为buttonrpc.hpp里有函数的定义，因此不在这里包含头文件，做前置声明

class buttonrpc;

class Master
{
  public:
    static Master *GetInstance();

    // 待定，可能要加参数，对类初始化，并且对rpc服务器初始化
    void Init(int argc, char **argv, int reducenum_, short port);

    void start();

    //给定一个map一个编号，用于区分生成的中间文件名
    int getMapId()
    {
        static int index = 0;
        return index ++;
    }
    // 分配map任务
    std::string assignMapTask();

    //设置某个Map任务完成
    bool setMapsuccess(std::string taskName);

    // 分配reduce任务
    int assignReduceTask();

    // 获取reduce的数量，用于map计算hash值
    int getReduceNum()
    {
        return reduceNum_;
    }

    //设置某个reduce任务完成
    bool setReducesuccess(int k);
    
    //判断Map是否完成
    bool isMapDone()
    {
        return fininshMapTasks_.size()==mapNum_;
    }
    
    //判断reduce是否完成
    bool Done()
    {
        return finishReduceTaskNum_ == reduceNum_;
    }

  private:

    Master();
    
    // 任务列表，通过参数传递，用链表储存方便插入
    std::list<std::string> mapTaskLists_;

    // map任务数量
    int mapNum_;

    // 完成任务的列表
    std::unordered_set<std::string> fininshMapTasks_;

    // 以防并发调用保证任务的线程安全
    std::mutex taskLock_;

    // reduce任务的数量，将自定义，并且给Map或者reduce，Map服务器求hash值的时候要用，编号从0
    // --- reduceNum_ - 1
    int reduceNum_;

    // 记录reduce的任务
    std::list<int> reduceTaskLists_;

    // 记录完成的reduce
    std::vector<bool> reduceVec_;
    // 记录完成的reduce数量
    int finishReduceTaskNum_;

    // 以下是网络模块
    std::unique_ptr<buttonrpc> rpcServer_;
    // 判断该服务器是否初始化
    bool isInit;

    // 定时判断任务处理完没有，如果没处理完重新加入task列表，这里作为多线程的函数
    void waitMapTask(std::string taskName);

    void waitReduceTask(int reduceTasknum);
};
