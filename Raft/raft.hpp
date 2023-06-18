#pragma once

#include "Raft/LogEntry.hpp"
#include "Serializer.hpp"
#include "persister.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <functional>
#include <mutex>
#include <string>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <vector>

using namespace std;

void printINFO(string format,...);

/*
raft类，使用多进程实现
*/

// 选举请求，因为rpc库的垃圾之处，所以才定义到这儿，不然会重复包含报错
class PeerInfo
{
  public:
    PeerInfo(){}
    PeerInfo(int id, short vp, short ap, string vip = "127.0.0.1",
             string aip = "127.0.0.1")
        : peerId_(id), voteIp_(vip), appendIp_(aip), votePort_(vp),
          appendPort_(ap)
    {
    }
    string voteIp_;
    string appendIp_;
    short votePort_;
    short appendPort_;

    int peerId_;
};

class RequestVoteRequest
{
  public:
    int term;         // 自己的任期
    int candidateId;  // 自己的id
    int lastLogIndex; // 最后一个日志的index
    int lastLogTerm;  // 最后一个日志的任期号

    // 序列化
    friend Serializer &operator>>(Serializer &in, RequestVoteRequest &r)
    {
        in >> r.term >> r.candidateId >> r.lastLogIndex >> r.lastLogTerm;
        return in;
    }
    friend Serializer &operator<<(Serializer &out, RequestVoteRequest &r)
    {
        out << r.term << r.candidateId << r.lastLogIndex << r.lastLogTerm;
        return out;
    }
};

// 选举回复
class RequestVoteResponse
{
  public:
    int term;         // 当前任期号
    bool voteGranted; // 投票与否
    friend Serializer &operator>>(Serializer &in, RequestVoteResponse &r)
    {
        in >> r.term >> r.voteGranted;
        return in;
    }
    friend Serializer &operator<<(Serializer &out, RequestVoteResponse &r)
    {
        out << r.term << r.voteGranted;
        return out;
    }
};

// 日志相关的类
// 日志请求类，由leader发起
class AppendEntriesRequest
{
  public:
    int term;
    int leaderId;
    int PrevLogIndex;
    int PrevLogTerm;
    string logs;
    int leaderCommit;

    friend Serializer &operator>>(Serializer &in, AppendEntriesRequest &request)
    {
        in >> request.term >> request.leaderId >> request.PrevLogIndex >>
            request.PrevLogTerm >>request.logs >> request.leaderCommit;
        return in;
    }

    friend Serializer &operator<<(Serializer &out,
                                  AppendEntriesRequest &request)
    {
        out << request.term << request.leaderId << request.PrevLogIndex
            << request.PrevLogTerm << request.logs << request.leaderCommit ;
        return out;
    }
};

// 日志答复，由follower答复
class AppendEntriesResponse
{
  public:
    AppendEntriesResponse():success(false),term(0){}
    int term;             // 当前任期号
    bool success;         // 如果follower包括前一个日志，返回true

    friend Serializer &operator>>(Serializer &in,
                                  AppendEntriesResponse &response)
    {
        in >> response.term >> response.success ;
        return in;
    }

    friend Serializer &operator<<(Serializer &out,
                                  AppendEntriesResponse &response)
    {
        out << response.term << response.success ;
        return out;
    }
};

class Raft
{
  public:
    // 必要的线程函数
    // 选举函数
    static void listenVote(Raft *raft); // 监听voteRpc的server线程
    static void callVote(
        Raft *raft, int id); // 用于发起voteRpc的，第二个参数是要求投票的raftid
    static void electionLoop(Raft *raft); // 选举线程

    // 日志相关函数
    static void listenAppend(Raft *raft); // 监听appendRpc的server线程
    static void processEntriesLoop(Raft *raft); // 持续处理日志同步的线程，监听心跳机制
    static void SendAppendEntrie(Raft *raft, int id); // 发appendRPC的线程
    

    //需要单独一个线程来提交日志，具体做法是监听每个日志的提交的次数，然后一个一个提交
    void commitLoop();

    RequestVoteResponse replyVote(RequestVoteRequest request); // 投票函数
    AppendEntriesResponse replyAppend(
        AppendEntriesRequest request); // 回复日志rpc函数

    enum State
    {
        LEADER = 0,
        CANDIDATE,
        FOLLOWER
    }; // 三种状态
    // 初始化
    void Init(vector<PeerInfo> &peer, int id);

    // 杀死
    void kill()
    {
        dead_ = true;
    }
    // 获取持续时间
    int getduringTime();

    // 获取任期时间
    int getTermTime();

    //  获取距离上次心跳事件的间距
    int getHeartTime();

    //获取状态
    State getstate()
    {
        return state_;
    }

    //加入日志,有客户端的时候配合客户端
    bool appendlog(string command)
    {
        if(state_ != LEADER)
        {
            return false;
        }
        LogEntry log(command,currentTerm_);
        lock_guard<mutex> lock(Loglock_);
        log_.push_back(log);
        logNum_.push_back(0);
        return true;
    }
    
    int getid()
    {
        return m_id_;
    }
  private:
    // 判断日志新旧的函数，在选举时可能会用
    bool isNewLog(RequestVoteRequest &request)
    {
        if(log_.size()==0||request.lastLogTerm==log_.back().term_)
        {
          return request.lastLogIndex >= log_.size();
        }

        return request.lastLogTerm>log_.back().term_;
    }

    // 保存状态函数
    void saveState();

    //更新事件函数
    void updateWake()
    {
        lastWakeTime_ = chrono::system_clock::now();
    }

    void updateHeart()
    {
        lastHeartTime_ = chrono::system_clock::now();
    }

    typedef chrono::time_point<chrono::system_clock, chrono::nanoseconds>
        nowTimeType;
    using seconds = chrono::seconds;

    //日志提交相关操作，需不需要锁待定？
    void commitLog(int logindex)
    {
        if(commitIndex_ == lastApplied_)
        {
            //如果阻塞，便唤醒
            commitIndex_ = logindex;
            eventfd_write(applyfd_, 1);
        }
        else
        {
            commitIndex_ = logindex;
        }
    }
    
    //用于解析日志并且使用，提交到本地
    void applyLog(LogEntry& log, int index);

    //通知上层客户完成这次请求，这里暂不做实现，有趣的是，仅仅会通知当前任期的日志
    //与实现安全性的方式一致，leader只对当前任期的日志条目使用计算副本数的方式来提交

    void notifyClient();
    
    //持久化的函数
    void saveRaft();
    void loadRaft();

    //void 

    // Persistent state
    int currentTerm_;      // 现在的term
    int voteFor_;          // 这轮次投票的人
    vector<LogEntry> log_; // 日志
    vector<int> logNum_;    //记录每个日志复制的次数

    // Volatile state
    int commitIndex_; // 已提交的日志下标
    int lastApplied_; // 最后提交的日志，不知道为什么，不会和上一个重复吗，可能跟应用层相关

    // 当leader时用
    vector<int> nextIndex_;  // 记录每个peer的该发送的下一个日志
    vector<int> matchIndex_; // 记录每个peer已复制的最新日志

    // 选举相关
    int recvVotes_;            // 记录选票数
    atomic_int finishVoteNum_; // 记录选举完成的服务器个数
    int cur_peerId_; // 记录当前操作的peerId，即当前通信peerId
    int leaderId_;   // 领导的id

    // 记录
    vector<PeerInfo> peers_;   // 记录raft服务器的信息
    State state_;              // 记录服务器当前状态
    int m_id_;                 // 记录自己在集群中的id
    bool dead_;                // 记录raft服务器的状态
    nowTimeType lastWakeTime_; // 上次唤醒的时间
    nowTimeType termStart_;    // 当leader时term的起始时间
    nowTimeType lastHeartTime_;// 记录上一次发心跳消息的时间
    int termDuring_;  // 任期内的时间持续长度


    // 持久化类，可能会写进文档里，这里暂时不做处理
    Persister persister_;

    mutex m_lock;             // 互斥锁
    condition_variable cond_; // 条件变量，选举时使用，当有新选票的时候呼唤

    //发送append实现线程安全需要的锁和条件变量
    mutex Loglock_;       //用日志的时候加锁
    mutex appendLock_;    //发送append线程的锁，当有新日志的时候会使用条件变量唤醒
    condition_variable appendCond_;  //有新append消息的时候用
    
    //提交日志的时候需要用的唤醒，以防线程不断循环浪费cpu
    int applyfd_;                   //用eventfd当信号量来使用   
    bool isfirst = true;
};
