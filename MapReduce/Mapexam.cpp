#include "Map.hpp"

/*
一个测试用例，这里对mapfunc进行继承，并且定义定义虚函数，这里的任务是统计英文单词的数量，如果有新任务，
那么要重写split函数，原函数有缺省的实现
*/
class m_MapFunc:public MapFunc
{
public:
    virtual void Map(vector<string>& content, vector<KeyValue>& kvs)
    {
        for(auto &str:content)
        {
            kvs.emplace_back(str,"1");
        }
    }
};

int main()
{
    m_MapFunc mf;
    MapServer map_server(&mf);
    map_server.startMap();
    return 0;
}