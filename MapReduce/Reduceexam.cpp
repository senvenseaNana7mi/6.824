#include "Reduce.hpp"
#include <cstdlib>

class m_reduceFunc:public ReduceFunc
{
public:
    virtual void Reduce(vector<KeyValue>& kvs, std::unordered_map<string, string>& reduceResult)
    {
        for(auto& kv:kvs)
        {
            if(!reduceResult.count(kv.key))
            {
                reduceResult[kv.key] = kv.value;
            }
            else 
            {
                int num1 = atoi(kv.value.c_str());
                int num2 = atoi(reduceResult[kv.key].c_str());
                reduceResult[kv.key] = to_string(num1+num2);
            }
        }
    }
};

int main()
{
    m_reduceFunc refunc;
    ReduceServer server_(&refunc);
    server_.startReduce();
    return 0;
}