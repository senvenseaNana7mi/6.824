#pragma once 
#include<string>
#include <sys/types.h>
#include<vector>
#include <memory>
#include "keyValue.hpp"

using namespace std;

//前置声明，类中用指针类型
class buttonrpc;

//声明一个纯虚类实现mapfunc，作为参数传给MapServer
class MapFunc
{
public:
    //写入kvs
    virtual void Map(vector<string>& content, vector<KeyValue>& kvs)=0;
    virtual void split(char *buf, int length, vector<string>& content);
    virtual void split(string& buf, vector<string>& content);
};

//Map服务器类
class MapServer
{
public:
    MapServer(MapFunc* mapfunc, const string &ip = "127.0.0.1", short port = 5555);
    ~MapServer();

    //启动函数
    void startMap(); 

private:
    //hash函数，放到合适的文件中以分配给reduce
    int ihash(const string& key);

    void Init();
    //rpc调用，获取reduce数量
    int getReduceNum();
    //rpc调用，获取map的编号
    int getMapId();

    //打开输出文件
    void openOutputFile();
    
    //rpc调用获取任务
    bool getTask();
       
    //获取内容
    bool getContent();

    //rpc 设置任务完成
    bool setTaskSuccess();
    
    //写进文件
    void writeTofile();

    //重置状态，每次完成一个任务便执行
    void resetState();

    //关闭所有文件描述符
    void closeFd();

    //服务器
    unique_ptr<buttonrpc> server_;
    //该map服务器的id
    int mapid_;
    //rpc调用获取，计算hash值可能会用
    int reduceNum_;
    //maptask的名字，rpc获取
    string mapTaskName_;
    //content，获取的内容
    vector<string> content_;
    //存放文件描述符，我们要根据hash值将key,value存入相应的名字，防止重复的关闭或者开启文件造成损耗，最后将关掉文件描述符
    vector<int> fdVec_;
    //存放map完的答案
    vector<KeyValue> kvs_;

    MapFunc* mapfunc_;  //作为必传参数

    //网络内容
    string rpcIp_;
    short rpcPort_;
};