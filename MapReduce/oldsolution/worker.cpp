#include "worker.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <strings.h>
#include <unistd.h>
#include <unordered_map>
#include "masterRPC.pb.h"


#define MAX_REDUCE_NUM 15
// 可能造成的bug，考虑write多次写，每次写1024用while读进buf
// c_str()返回的是一个临时指针，值传递没事，但若涉及地址出错

int disabledMapId = 0;    // 用于人为让特定map任务超时的Id
int disabledReduceId = 0; // 用于人为让特定reduce任务超时的Id



// 定义master分配给自己的map和reduce任务数，实际无所谓随意创建几个，我是为了更方便测试代码是否ok
int map_task_num;
int reduce_task_num;

// 给每个map线程分配的任务ID，用于写中间文件时的命名
int MapId = 0;
pthread_mutex_t map_mutex;
pthread_cond_t cond;
int fileId = 0;

//定义一个客户端，以远程调用


//计算给哪个reduce
int ihash(string str)
{
    int sum = 0;
    for (char s : str)
    {
        sum += (s - '0');
    }
    return sum % reduce_task_num;
}

// 删除所有写入中间值的临时文件，是由不同得map和reduce生成得文件
void removeFiles(){
    string path;
    for(int i = 0; i < map_task_num; i++){
        for(int j = 0; j < reduce_task_num; j++){
            path = "mr-" + to_string(i) + "-" + to_string(j);
            int ret = access(path.c_str(), F_OK);
            if(ret == 0) remove(path.c_str());
        }
    }
}


// 取得  key:filename, value:content 的kv对作为map任务的输入
KeyValue getContent(char *file)
{
    int fd = open(file,O_RDONLY);
    int length = lseek(fd, 0, SEEK_END);
    char buf[length];
    bzero(buf, length);
    int len = read(fd, buf,length);
    if(len!=length)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    KeyValue kv;
    kv.key = string(file);
    kv.value = string(buf);
    close(fd);
    return kv;
}

// 将map任务产生的中间值写入临时文件
void writeKV(int fd, const KeyValue &kv)
{
    string tmp = kv.key +",1";
    int len = write(fd, tmp.c_str(),tmp.size());
    if(len==-1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

// 创建每个map任务对应的不同reduce号的中间文件并调用 -> writeKV 写入磁盘
//mapTaskIdx是选择map得index
void writeInDisk(const vector<KeyValue> &kvs, int mapTaskIdx)
{
    //根据hash值选择reduce，但是这样是否有点慢呢
    for(const auto& v: kvs)
    {
        int reduce_idx = ihash(v.key);
        string path;
        path = "mr-"+ to_string(mapTaskIdx) + "-" + to_string(reduce_idx);
        int ret = access(path.c_str(),F_OK);
        if(ret == -1)
        {
            int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0664);
            writeKV(fd,v);
        }
        else if(ret==0)
        {
            int fd = open(path.c_str(), O_WRONLY | O_APPEND);
            writeKV(fd, v);
        }
    }
}

// 以char类型的op为分割拆分字符串
vector<string> split(string text, char op)
{
    int n = text.size();
    vector<string> str;
    string tmp="";
    for(int i=0;i<n;++i)
    {
        if(text[i]!=op)
        {
            tmp +=text[i];
        }
        else 
        {
            if(tmp.size() != 0 )
            {
                str.push_back(tmp);
            }
            tmp = "";
        }
    }
    return str;
}

// 以逗号为分割拆分字符串，重载版本
string split(string text)
{
    string tmp ="";
    for(int i=0;i<text.size();++i)
    {
        if(text[i]!=',')
        {
            tmp+=text[i];
        }
        else break;
    }
    return tmp;
}

// 获取对应reduce编号的所有中间文件
vector<string> getAllfile(string path, int op)
{
    DIR* dir = opendir(path.c_str());
    vector<string> ret;
    if( dir == NULL)
    {
        printf("[ERROR] %s is not a directory or not exist!", path.c_str());
        return ret;
    }
    struct dirent* entry;
    while((entry=readdir(dir))!=NULL)
    {
        int len = strlen(entry->d_name);
        int oplen = to_string(op).size();
        if(len - oplen<5)continue;
        string filename(entry->d_name);
        if(!(filename[0] == 'm' && filename[1] == 'r' && filename[len - oplen - 1] == '-')) continue;
        string cmp_str = filename.substr(len - oplen, oplen);
        if(cmp_str == to_string(op))
        {
            ret.push_back(entry->d_name);
        }
    }
    closedir(dir);
    return ret;
}

// 对于一个ReduceTask，获取所有相关文件并将value的list以string写入vector
// vector中每个元素的形式为"abc 11111";
vector<KeyValue> Myshuffle(int reduceTaskNum)
{
    string path;
    vector<string> str;
    str.clear();
    vector<string> filename = getAllfile(".",reduceTaskNum);
    unordered_map<string, string> hash;
    for(int i=0;i<filename.size();++i)
    {
        path = filename[i];
        char text[path.size()+1];
        strcpy(text, path.c_str());
        KeyValue kv = getContent(text);
        string context = kv.value;
        vector<string> retStr = split(context, ' ');
        str.insert(str.end(),retStr.begin(), retStr.end());
    }
    for(auto& a: str)
    {
        hash[split(a)]+="1";
    }
    vector<KeyValue> retKvs;
    KeyValue tmpKv;
    for(const auto& a : hash){
        tmpKv.key = a.first;
        tmpKv.value = a.second;
        retKvs.push_back(tmpKv);
    }
    sort(retKvs.begin(), retKvs.end(), [](KeyValue& kv1, KeyValue& kv2){
        return kv1.key < kv2.key;
    });
    return retKvs;
}

void *mapWorker(void *arg)
{

}

// 用于最后写入磁盘的函数，输出最终结果
void myWrite(int fd, vector<string> &str);

void *reduceWorker(void *arg);

// 删除最终输出文件，用于程序第二次执行时清除上次保存的结果
void removeOutputFiles();