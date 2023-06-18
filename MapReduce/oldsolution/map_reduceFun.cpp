#include "map_reduceFun.h"
#include <cstdlib>
#include <map>

vector<string> split(string &content)
{
    vector<string> str;
    string tmp = "";
    for (int i = 0; i < content.size(); ++i)
    {
        if ((content[i] >= 'A' && content[i] <= 'Z') ||
            (content[i] >= 'a' && content[i] <= 'z'))
        {
            ++i;
            tmp += content[i];
        }
        else
        {
            str.push_back(tmp);
            tmp = "";
        }
    }
    return str;
}

//将内容分片，并且按照想要的格式保存成key-value
vector<KeyValue> mapF(KeyValue& kv)
{
    vector<string> str=split(kv.value);
    vector<KeyValue> kvs(str.size());
    for(int i=0;i<str.size();++i)
    {
        KeyValue tmp;
        tmp.key = str[i];
        tmp.value = "1";
        kvs[i]=tmp;
    }
    return kvs;
}


//处理map处理完的文件，其中的传入的kvs由Shuffle函数读取对应的reduceNum文件，不知道这样做会不会出问题，因为是一次性读完所有相应的reduceNum文件
vector<KeyValue> shuffleFunc(vector<KeyValue>& kvs)
{
    map<string,string> keyNum;
    for(auto &kv : kvs)
    {
        keyNum[kv.key]+=atoi(kv.value.c_str());
    }
    vector<KeyValue> shufret;
    for(auto it = keyNum.begin();it!=keyNum.end();++it)
    {
        shufret.emplace_back(it->first,it->second);
    }
    return shufret;
}

//将shuffle的key按要求处理，在这个任务中按照key,1111后面那个，其实这个没有什么用
vector<KeyValue> reduceF(vector<KeyValue>& kvs, int reduceTaskIdx)
{
    vector<KeyValue> reduceKvs(kvs.size());
    for(int i=0;i<kvs.size();++i)
    {
        KeyValue tmp;
        tmp.key = kvs[i].key;
        tmp.value = to_string(kvs[i].value.size());
        reduceKvs[i]=tmp;
    }
    return reduceKvs;
}

