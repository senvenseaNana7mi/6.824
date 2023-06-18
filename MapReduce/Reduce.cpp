#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Reduce.hpp"
#include "buttonrpc.hpp"

// 构造函数
ReduceServer::ReduceServer(ReduceFunc *reducef, const string &ip, short port)
    : reducefun_(reducef), rpcIp_(ip), rpcPort_(port)
{
}

ReduceServer::~ReduceServer()
{
}
void ReduceServer::startReduce()
{
    // 第一步初始化
    Init();
    // 循环执行
    while (true)
    {
        int taskid = getReduceTask();
        // map任务还未完成
        if (taskid == -2)
        {
            // 等待两秒
            sleep(2);
        }
        // 已完成reduce
        else if (taskid == -1)
        {
            break;
        }
        // 有任务
        else
        {
            reduceTaskId_ = taskid;
            // 根据mapid逐步查找文件
            string filenameMask = "Map_temp_";
            for (int i = 0;; ++i)
            {
                // 中间文件名字格式 Map_temp_mapid_reducetask
                string filename =
                    filenameMask + to_string(i) + '_' + to_string(taskid);
                if (-1 ==
                    access(filename.c_str(), R_OK)) // 文件不存在直接终止任务
                {
                    break;
                }
                if (!getReduceKvs(filename))
                {
                    perror("open failed!");
                    exit(EXIT_FAILURE);
                }
                // reduce任务
                reducefun_->Reduce(kVs_, reduceResult_);
                // 一定要重置任务，因为这里已经存储在哈希表中
                kVs_.resize(0);
            }
            if (setTaskSuccess())
            {
                // 防止因为超时而多写了很多
                writeTofile();
            }
            // 重置状态
            resetState();
        }
    }
}

// 初始化
void ReduceServer::Init()
{
    kVs_.clear();
    reduceResult_.clear();

    // 网络
    server_ = make_unique<buttonrpc>();
    server_->as_client(rpcIp_, rpcPort_);
}

// 获取任务，要等map做完所有任务reduce才开始做，不然不准确,rpc调用
int ReduceServer::getReduceTask()
{
    return server_->call<int>("assignReduceTask").val();
}

// 获取获取相关reducetaskid的内容，因为内容可能很多，可能要循环作用，写进kVs
bool ReduceServer::getReduceKvs(const string &filename)
{
    int fd = open(filename.c_str(), O_RDONLY);
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
    string key, value;
    int i = 0;
    while (i < readLength)
    {
        while (i < readLength && buf[i] != ',')
        {
            key += buf[i++];
        }
        ++i;
        while (i < readLength && buf[i] != '\n')
        {
            value += buf[i++];
        }
        ++i;
        kVs_.emplace_back(key, value);
        key.clear();
        value.clear();
    }
    return true;
}

// 设置任务成功,rpc调用
bool ReduceServer::setTaskSuccess()
{
    return server_->call<bool>("setReducesuccess", reduceTaskId_).val();
}

// 写文件根据reduceTask写
void ReduceServer::writeTofile()
{
    string filename = "Reduce_Result_" + to_string(reduceTaskId_);
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666);
    for (auto it = reduceResult_.begin(); it != reduceResult_.end(); ++it)
    {
        string line = it->first + ',' + it->second + '\n';
        write(fd, line.c_str(), line.size());
    }
    close(fd);
}

/// 重置状态，每次完成一个任务便执行
void ReduceServer::resetState()
{
    reduceResult_.clear();
    kVs_.resize(0);
    reduceTaskId_ = -1;
}