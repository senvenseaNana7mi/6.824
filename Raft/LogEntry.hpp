#pragma once
#include <string>
using namespace std;
class LogEntry
{
  public:
    // 日志类
    LogEntry(string cmd = "", int term = -1) : command_(cmd), term_(term)
    {
    }
    string command_;
    int term_;
};

hyw change