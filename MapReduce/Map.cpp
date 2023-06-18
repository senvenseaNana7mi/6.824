#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

#include "Map.hpp"
#include "buttonrpc.hpp"
#include "keyValue.hpp"

inline bool isNeed(char str)
{
    return (str >= 'A' && str <= 'Z') || (str >= 'a' && str <= 'z');
}

// MapFunc中split的缺省实现
void MapFunc::split(char *buf, int length, vector<string> &content)
{
    if (length <= 0)
    {
        return;
    }
    string emp = "";
    for (int i = 0; i < length; ++i)
    {
        if (isNeed(buf[i]))
        {
            emp += buf[i];
        }
        else
        {
            if (!emp.empty())
            {
                content.emplace_back(std::move(emp));
                emp = "";
            }
        }
    }
}

void MapFunc::split(string &buf, vector<string> &content)
{
    if (buf.size() == 0)
    {
        return;
    }
    string emp = "";
    for (int i = 0; i < buf.size(); ++i)
    {
        if (isNeed(buf[i]))
        {
            emp += buf[i];
        }
        else
        {
            if (!emp.empty())
            {
                content.emplace_back(std::move(emp));
                emp = "";
            }
        }
    }
}
// MapServer的部分成员函数实现
MapServer::MapServer(MapFunc *mapfunc, const string &ip, short port)
    : mapfunc_(mapfunc), rpcIp_(ip), rpcPort_(port)
{
}

// 析构
MapServer::~MapServer()
{
}

// 启动函数
void MapServer::startMap()
{
    Init();
    while (getTask())
    {
        if (!getContent())
        {
            perror("the map task file is not exist or can't read");
            exit(EXIT_FAILURE);
        }
        // map任务
        mapfunc_->Map(content_, kvs_);

        // 设置任务成功若成功写入文件若不成功不写
        if (setTaskSuccess())
        {
            writeTofile();
        }
        resetState();
    }
    closeFd();
}
int MapServer::ihash(const string &key)
{
    int sumNum = 0;
    for (char str : key)
    {
        sumNum += (str);
    }
    return sumNum % reduceNum_;
}

void MapServer::Init()
{
    server_ = make_unique<buttonrpc>();
    server_->as_client(rpcIp_, rpcPort_);
    server_->set_timeout(2000);
    // 服务器连接好了便可以使用rpc函数

    // 首先获取reduceNum
    reduceNum_ = getReduceNum();
    // 获取mapid_
    mapid_ = getMapId();
    // 打开相应的文件描述符
    fdVec_.clear();
    content_.clear();
    kvs_.clear();
    openOutputFile();
}

// rpc调用，获取reduce数量
int MapServer::getReduceNum()
{
    return server_->call<int>("getReduceNum").val();
}
// rpc调用，获取map的编号
int MapServer::getMapId()
{
    return server_->call<int>("getMapId").val();
}
// 打开文件描述符
void MapServer::openOutputFile()
{
    fdVec_.resize(reduceNum_);
    for (int i = 0; i < reduceNum_; ++i)
    {
        // 临时文件名称 Map_temp_mapid_reducetask;
        string filename = "Map_temp_";
        filename += (to_string(mapid_) + "_" + to_string(i));
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666);
        if (fd < 0)
        {
            perror("open");
            exit(EXIT_FAILURE);
        }
        fdVec_[i] = fd;
    }
}

bool MapServer::getTask()
{
    mapTaskName_ = server_->call<string>("assignMapTask").val();
    if (mapTaskName_ == "empty")
    {
        return false;
    }
    return true;
}

// 获取得到文件的内容，在实际应用这里可能要访问GFS服务器
bool MapServer::getContent()
{
    int fd = open(mapTaskName_.c_str(), O_RDONLY);
    if (fd < 0)
    {
        return false;
    }
    int readLength = lseek(fd, 0, SEEK_END);

    lseek(fd, 0, SEEK_SET);

    char buf[readLength + 1]; // 声明一个buf
    bzero(buf, readLength + 1);
    buf[readLength] = '\0';
    int len = read(fd, buf, readLength);
    if (len != readLength)
    {
        // 读取发生错误
        return false;
    }
    mapfunc_->split(buf, readLength, content_);
    return true;
}

// 设置任务完成
bool MapServer::setTaskSuccess()
{
    return server_->call<bool>("setMapsuccess").val();
}

// 写进文件
void MapServer::writeTofile()
{
    for (auto &kv : kvs_)
    {
        int index = ihash(kv.key);
        //组织输出到文件的格式 key,value \n
        string kvContent = kv.key+","+kv.value+'\n';
        int fd = fdVec_[index];
        int len = write(fd,kvContent.c_str(),kvContent.size());
        if(len != kvContent.size())
        {
            perror("write error");
            return ;
        }
    }
}

void MapServer::resetState()
{
    content_.resize(0);
    kvs_.resize(0);
}

//用于清除文件描述符
void MapServer::closeFd()
{
    for(int i=0;i<fdVec_.size();++i)
    {
        close(fdVec_[i]);
    }
}