#include "logger.hpp"

#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <time.h>

// 输入信息
void Logger::pushMsg(std::string &msg)
{
    // 当停止了就不能输入日志了
    if (!stoppush_)
    {
        std::lock_guard<std::mutex> lock(m_mtx_);
        msgQue_.push(msg);
        m_cond_.notify_one();
    }
}

// 输出信息
std::string Logger::popMsg()
{
    std::unique_lock<std::mutex> lock(m_mtx_);
    while (!isstop_ && msgQue_.empty())
    {
        m_cond_.wait(lock);
    }
    std::string rtmsg;
    if (!isstop_)
    {
        rtmsg = msgQue_.front();
        msgQue_.pop();
    }
    else
    {
        rtmsg = "log stop";
    }
    return rtmsg;
}

// 获取日志的单例
Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

// 构造函数启动写日志线程
Logger::Logger()
{
    // 打开文件
    time_t nowday = time(nullptr);
    tm *nowday_t = localtime(&nowday);

    char file_name[128];
    // 日志文件名
    sprintf(file_name, "%d-%d-%d-log.txt", nowday_t->tm_year + 1900,
            nowday_t->tm_mon + 1, nowday_t->tm_mday);

    pf = fopen(file_name, "a+");
    if (pf == nullptr)
    {
        std::cout << "logger file :" << file_name << "open error" << std::endl;
        exit(EXIT_FAILURE);
    }
    writeLogTask = std::move( std::thread(([&]() {
        for (; !isstop_;)
        {
            std::string msg = popMsg();

            time_t now = time(NULL);
            tm *nowtm = localtime(&now);

            char time_buf[128] = {0};
            sprintf(time_buf, "%d-%d-%d %d:%d:%d ->[%s] ",
                    nowtm->tm_year + 1000, nowtm->tm_mon + 1, nowtm->tm_mday,
                    nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec,
                    (m_loglevel == INFO ? "info" : "error"));

            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
        }
        fclose(pf);
    })));
}
// 设置日志的级别
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}
// 写日志，把日志信息写道缓冲区当中
void Logger::Log(std::string msg)
{
    pushMsg(msg);
}
Logger::~Logger()
{
    stoppush_ = true;
    while (!msgQue_.empty())
    {
    }
    isstop_ = true;
    m_cond_.notify_all();
    writeLogTask.join();
}