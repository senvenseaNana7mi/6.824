#pragma once
#include<string>

/*
    Mapreduce框架里基本的传播模式
*/

class KeyValue{
public:
    KeyValue(){}
    explicit KeyValue(const std::string& k,const std::string& v):key(k),value(v)
    {

    }
    std::string key;
    std::string value;
};