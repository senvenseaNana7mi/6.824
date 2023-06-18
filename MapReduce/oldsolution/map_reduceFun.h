#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>

using namespace std;

class KeyValue
{
  public:
    string key;
    string value;
};

//将整段的字符分解成字符串数组，用于map
vector<string> split(string& content);

//将内容分片，并且按照想要的格式保存成key-value
vector<KeyValue> mapF(KeyValue& kv);


//处理map处理完的文件，其中的传入的kvs由Shuffle函数读取对应的reduceNum文件
vector<KeyValue> shuffleFunc(vector<KeyValue>& kvs);

//将shuffle的key按要求处理，在这个任务中按照key,1111后面那个
vector<KeyValue> reduceF(vector<KeyValue>& kvs, int reduceTaskIdx);

