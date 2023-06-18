#pragma once 
#include "LogEntry.hpp"
#include<vector>
using namespace std;
/*
持久化类，实际应用是存在磁盘上;
持久化log, term, votefor
*/

class Persister
{
public:
    vector<LogEntry> log;
    int term;
    int voteFor;
};