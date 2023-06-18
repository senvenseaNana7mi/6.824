#pragma once 
#include <unordered_map>
#include<vector>
#include <memory>
#include<string>

#include"keyValue.hpp"

using namespace std;

/*
在实际应用中，map和reduce在两个服务器当中，其中还涉及到map和reduce服务器的通信，这两个之间有必要的网络通信，
通过集群这两个服务器可以提高性能。因为一个reduceTask对应一种reduceid，在很多或者所有map服务器都有相应的内容，
因此也许需要发布订阅消息队列来实现多服务器之间的通信，此处直接简化操作，假设所有文件都会在本地服务器上，较难的后面实现（也许吧！）
*/


//reduce纯虚类，若要使用，自定义reduce功能，此处为了统计文本中的数量
class ReduceFunc
{
public:
    //kvs是读取map获得的， 
    virtual void Reduce(vector<KeyValue>& kvs, std::unordered_map<string, string>& reduceResult)=0;
};

class buttonrpc;

class ReduceServer
{
public:
    ReduceServer(ReduceFunc* reducef,const string& ip = "127.0.0.1", short port = 5555);

    ~ReduceServer();

    //唯一暴露的接口，作为启动函数
    void startReduce();
private:
    //初始化
    void Init();
    //获取任务，要等map做完所有任务reduce才开始做，不然不准确,rpc调用
    int getReduceTask();
    //获取获取相关reducetaskid的内容，因为内容可能很多，可能要循环作用，写进kVs
    bool getReduceKvs(const string& filename);

    //设置任务成功,rpc调用
    bool setTaskSuccess();

    //写文件根据reduceTask写
    void writeTofile();

    ///重置状态，每次完成一个任务便执行
    void resetState();


    //master分配给reduce的任务，可能以reduce服务器会处理很多reduce任务
    int reduceTaskId_;
    //相应reducetask的所有内容，由map处理
    vector<KeyValue> kVs_;
    //用于保存reduce结果
    unordered_map<string, string> reduceResult_;
    
    //网络相关
    unique_ptr<buttonrpc> server_;
    string rpcIp_;
    short rpcPort_;

    //任务相关
    ReduceFunc* reducefun_;
};