#pragma once

#include <bits/types/FILE.h>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
};

// 单例
// Mprpc框架日志系统
// 定义宏
#define LOG_INFO(logmsgformat, ...)                                            \
    do                                                                         \
    {                                                                          \
        Logger &logger = Logger::GetInstance();                                \
        logger.SetLogLevel(INFO);                                              \
        char c[1024] = {0};                                                    \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);                        \
        logger.Log(c);                                                         \
    } while (0);

#define LOG_ERR(logmsgformat, ...)                                             \
    do                                                                         \
    {                                                                          \
        Logger &logger = Logger::GetInstance();                                \
        logger.SetLogLevel(ERROR);                                             \
        char c[1024] = {0};                                                    \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);                        \
        logger.Log(c);                                                         \
    } while (0);



class Logger
{
  public:
    // 获取日志的单例
    static Logger &GetInstance();
    // 设置日志的级别
    void SetLogLevel(LogLevel level);
    // 写日志
    void Log(std::string msg);

    ~Logger();

  private:
    std::string popMsg();
    void pushMsg(std::string &msg);

    std::thread writeLogTask;
    LogLevel m_loglevel; // 记录Log级别
    bool isstop_ = false;
    bool stoppush_ = false;
    FILE *pf = nullptr;
    std::condition_variable m_cond_;
    std::mutex m_mtx_;
    std::queue<std::string> msgQue_;

    Logger();
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
};
