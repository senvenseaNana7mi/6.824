#include "raft.hpp"
#include "buttonrpc.hpp"
#include "logger.hpp"
#include "persister.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <mutex>
#include <string>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>

// 任期时间 单位s
#define TERMDURING 10

// 选举超时时间 单位s
#define TIMEOUT 5

// 心跳时间 单位s
#define HEARTTIME 100000

// rpc的重传次数
#define RETRYNUM 4

// 打印日志用
void printINFO(string format, ...)
{
    time_t now = time(NULL);
    tm *nowtm = localtime(&now);

    char time_buf[128] = {0};
    sprintf(time_buf, "%d-%d-%d %d:%d:%d  : ", nowtm->tm_year + 1900,
            nowtm->tm_mon + 1, nowtm->tm_mday, nowtm->tm_hour, nowtm->tm_min,
            nowtm->tm_sec);
    cout << time_buf;

    va_list st;
    va_start(st, format);
    char buf[1024];
    vsprintf(buf, format.c_str(), st);
    va_end(st);
    cout << buf << endl;
}

// 初始化
void Raft::Init(vector<PeerInfo> &peer, int id)
{
    currentTerm_ = 0;
    voteFor_ = -1;
    log_.clear();

    commitIndex_ = 0;
    lastApplied_ = 0;

    nextIndex_.resize(peer.size(), 1);
    matchIndex_.resize(peer.size(), 0);

    recvVotes_ = 0;
    finishVoteNum_ = 0;

    cur_peerId_ = -1;
    leaderId_ = -1;

    peers_ = peer;
    state_ = FOLLOWER;
    m_id_ = id;
    dead_ = false;
    updateWake();
    updateHeart();

    saveState();

    // 由该线程开启新的electloop线程
    thread t_vote(listenVote, this);
    t_vote.detach();

    // 日志处理线程
    thread t_append(listenAppend, this);
    t_append.detach();

    // // 提交日志线程，包括通知客户端
    thread t_apply(&Raft::commitLoop, this);
    t_apply.detach();

    applyfd_ = eventfd(0, EFD_SEMAPHORE);
    if (applyfd_ == -1)
    {
        perror("eventfd");
        exit(EXIT_FAILURE);
    }
    printINFO("the num %d raft init accomplish!", id);
}

// 保存状态函数
void Raft::saveState()
{
    persister_.log = this->log_;
    persister_.term = this->currentTerm_;
    persister_.voteFor = this->voteFor_;
    saveRaft();
}

// 监听voteRpc的server线程
void Raft::listenVote(Raft *raft)
{
    buttonrpc listenVoteServer_;
    listenVoteServer_.as_server(raft->peers_[raft->m_id_].votePort_);
    listenVoteServer_.bind("replyVote", &Raft::replyVote, raft);

    thread t_elect(electionLoop, raft);
    t_elect.detach();

    listenVoteServer_.run();
    printf("exit!\n");
}

// 获取唤醒时间
int Raft::getduringTime()
{
    nowTimeType nowtime = chrono::system_clock::now();
    return chrono::duration_cast<seconds>(nowtime - lastWakeTime_).count();
}

// 获取任期时间
int Raft::getTermTime()
{
    nowTimeType nowtime = chrono::system_clock::now();
    return chrono::duration_cast<seconds>(nowtime - termStart_).count();
}

//  获取距离上次心跳事件的间距
int Raft::getHeartTime()
{
    nowTimeType nowtime = chrono::system_clock::now();
    return chrono::duration_cast<seconds>(nowtime - lastHeartTime_).count();
}

// 选举线程
void Raft::electionLoop(Raft *raft)
{
    chrono::system_clock::now();
    while (!raft->dead_)
    {
        // 定义timeout，单位ms
        int timeOut = rand() % TIMEOUT + TIMEOUT;
        // 进入一轮选举
        while (true)
        {
            // 适当阻塞一下，降低cpu的压力
            usleep(1000);
            unique_lock<mutex> lock(raft->m_lock);
            int during_time = raft->getduringTime();
            if (during_time > timeOut && raft->state_ == FOLLOWER)
            {
                raft->state_ = CANDIDATE;
            }
            // 开始选举
            if (raft->state_ == CANDIDATE && during_time > timeOut)
            {

                // 打印日志，日志系统后面实现
                raft->updateWake();
                raft->currentTerm_++;
                raft->voteFor_ = raft->m_id_;
                raft->saveState();
                printINFO("the num %d raft start elect!, term is %d",
                          raft->m_id_, raft->currentTerm_);
                raft->recvVotes_ = 1;
                raft->finishVoteNum_ = 1;
                raft->leaderId_ = raft->m_id_;

                for (int i = 0; i < raft->peers_.size(); ++i)
                {
                    if (i == raft->m_id_)
                        continue;
                    auto &peer = raft->peers_[i];
                    // 请求选举线程，这里是否可以用线程池优化
                    thread tvote(Raft::callVote, raft, i);
                    tvote.detach();
                }

                while (raft->recvVotes_ <= raft->peers_.size() / 2 &&
                       raft->finishVoteNum_ != raft->peers_.size())
                {
                    // 等待线程
                    raft->cond_.wait(lock);
                }

                if (raft->state_ == FOLLOWER)
                {
                    continue;
                }

                if (raft->recvVotes_ > raft->peers_.size() / 2)
                {
                    raft->state_ = LEADER;

                    raft->updateHeart();
                    printINFO("num %d raft is Leader!, term is %d", raft->m_id_,
                              raft->currentTerm_);
                    // 当上领导后便维护
                    for (int i = 0; i < raft->peers_.size(); i++)
                    {
                        raft->nextIndex_[i] = raft->log_.size() + 1;
                        raft->matchIndex_[i] = 0;
                    }
                    // 更新term
                    raft->termStart_ = chrono::system_clock::now();
                    // 设置任期时间
                    raft->termDuring_ = rand() % TERMDURING + TERMDURING;

                    break;
                }
            }
        }
    }
}

// 请求投票函数
void Raft::callVote(Raft *raft, int id)
{
    buttonrpc client;
    RequestVoteRequest request;
    request.candidateId = raft->m_id_;
    request.lastLogIndex = raft->log_.size();
    request.term = raft->currentTerm_;
    if (raft->log_.size() == 0)
    {
        request.lastLogTerm = 0;
    }
    else
        request.lastLogTerm = raft->log_.back().term_;

    client.as_client("127.0.0.1", raft->peers_[id].votePort_);

    RequestVoteResponse response =
        client.call<RequestVoteResponse>("replyVote", request).val();

    raft->finishVoteNum_++;
    if (response.voteGranted)
    {
        unique_lock<mutex> lock(raft->m_lock);
        raft->recvVotes_++;
    }
    raft->cond_.notify_one();
}

// 是否投票
RequestVoteResponse Raft::replyVote(RequestVoteRequest request)
{
    RequestVoteResponse response;
    response.voteGranted = false;
    response.term = currentTerm_;

    lock_guard<mutex> lock(m_lock);

    if (request.term <= currentTerm_)
    {
        return response;
    }

    currentTerm_ = request.term;

    bool ret = isNewLog(request);

    // 通过检查
    if (!ret)
    {
        return response;
    }

    printINFO("num%d raft vote to num%d raft, term is %d", m_id_,
              request.candidateId, request.term);
    currentTerm_ = request.term;
    response.voteGranted = true;
    voteFor_ = request.candidateId;
    updateWake();
    state_ = FOLLOWER;
    leaderId_ = request.candidateId;
    saveState();
    return response;
}

// 日志相关函数
//  监听appendRpc的server线程
void Raft::listenAppend(Raft *raft)
{
    buttonrpc appendServer_;
    appendServer_.as_server(raft->peers_[raft->m_id_].appendPort_);

    appendServer_.bind("replyAppend", &Raft::replyAppend, raft);

    thread t_append(Raft::processEntriesLoop, raft);
    t_append.detach();

    appendServer_.run();
    printf("exit!\n");
}

// 持续处理日志同步的线程，状态为leader时才用
void Raft::processEntriesLoop(Raft *raft)
{

    // 用于判断是否能发送append
    while (!raft->dead_)
    {
        // 阻塞防止cpu负载过高
        usleep(1000);

        // 这里加作用域是为了自动释放，防止死锁
        {
            lock_guard<mutex> lock(raft->m_lock);
            if (raft->state_ != LEADER)
            {
                continue;
            }
        }
        // 是leader了，为每个peer创建线程
        for (int i = 0; i < raft->peers_.size(); ++i)
        {
            // 直接开启线程对每个peer的日志进行同步
            if (i == raft->m_id_)
                continue;
            thread tappend(Raft::SendAppendEntrie, raft, i);
            tappend.detach();
        }

        // 然后判断是否有append发送
        raft->Loglock_.lock();
        int logSize = raft->log_.size();
        raft->Loglock_.unlock();

        while (true)
        {
            // 防止该线程作用过快，一直抢占锁
            usleep(1000);
            // 判断是否是leader或者任期是否结束
            {
                lock_guard<mutex> lock1(raft->m_lock);
                if (raft->state_ != LEADER)
                {
                    break;
                }
                int termdur = raft->getTermTime();
                // 任期过了
                if (termdur >= raft->termDuring_)
                {
                    raft->state_ = FOLLOWER;
                    // 这里要唤醒一下，因为发送线程可能在阻塞，需要结束发送线程
                    raft->appendCond_.notify_all();
                    // 要重新开始计时
                    raft->updateWake();
                    break;
                }
            }
            unique_lock<mutex> lock(raft->appendLock_);
            int heartdur = raft->getHeartTime();

            raft->Loglock_.lock();
            int nowLogSize = raft->log_.size();
            raft->Loglock_.unlock();

            // 有新日志
            if (nowLogSize > logSize)
            {
                raft->appendCond_.notify_all();
                logSize = nowLogSize;
            }
            // 发送心跳
            else if (heartdur >= HEARTTIME)
            {
                raft->appendCond_.notify_all();
                raft->updateHeart();
            }
        }
    }
}

// 发送日志线程，并且是心跳机制
void Raft::SendAppendEntrie(Raft *raft, int id)
{
    // 循环到任期结束或者状态不是leader
    int logPreSize;
    {
        lock_guard<mutex> lock(raft->Loglock_);
        logPreSize = raft->log_.size();
    }
    // 连接rpc服务器
    buttonrpc client;
    client.as_client("127.0.0.1", raft->peers_[id].appendPort_);
    client.set_timeout(1000);
    printINFO("the num%d raft's sendappend thread start running!", raft->m_id_);
    while (true)
    {
        usleep(1000);
        {
            lock_guard<mutex> lock(raft->m_lock);
            if (raft->state_ != LEADER)
            {
                break;
            }
        }
        int logsize, logterm = 0;
        {
            lock_guard<mutex> lock(raft->Loglock_);
            logsize = raft->log_.size();
            if (raft->log_.size() != 0)
            {
                logterm = raft->log_.back().term_;
            }
        }
        unique_lock<mutex> lock(raft->appendLock_);

        int needIndex = raft->nextIndex_[id];

        // 日志没有到需求的日志，等待主线程唤醒
        if (logsize < needIndex)
        {
            raft->appendCond_.wait(lock);
        }

        // 因为当任期过了后也会唤醒，然后会结束发送线程。这里其实可以不用结束，但是考虑到线程过多直接结束好了
        usleep(1000);
        {
            lock_guard<mutex> lock(raft->m_lock);
            if (raft->state_ != LEADER)
            {
                break;
            }
        }

        // 这个锁貌似就没什么用了，因为只是为了起到通知作用，提高并发的速度，减少隔离区代码量
        lock.unlock();

        AppendEntriesRequest request;
        request.PrevLogIndex = 0;
        request.PrevLogTerm = 0;
        request.leaderCommit = raft->commitIndex_;
        request.term = raft->currentTerm_;
        request.leaderId = raft->m_id_;

        logsize = raft->log_.size();

        // 没日志发送心跳消息
        if (logsize < needIndex)
        {
            request.PrevLogIndex = logsize;
            request.PrevLogTerm = logterm;
            request.logs = "1";
            request.logterm = 0;
            printINFO(
                "num%d raft is leader, send heart to num%d raft, term is %d",
                raft->m_id_, id, raft->currentTerm_);
        }
        // 有日志发送
        else
        {
            lock_guard<mutex> lock(raft->Loglock_);
            if (needIndex != 1)
            {
                request.PrevLogIndex = needIndex - 1;
                request.PrevLogTerm = raft->log_[needIndex - 2].term_;
            }
            request.logterm = raft->log_[needIndex -1].term_;
            request.logs = raft->log_[needIndex - 1].command_;
            printINFO("num%d raft is leader, send log to num%d raft, term is "
                      "%d, log index is %d",
                      raft->m_id_, id, raft->currentTerm_, needIndex);
        }

        int falsenum = 0;
        auto ret = client.call<AppendEntriesResponse>("replyAppend", request);

        while (falsenum < RETRYNUM &&
               ret.error_code() == buttonrpc::RPC_ERR_RECV_TIMEOUT)
        {
            falsenum++;
            ret = client.call<AppendEntriesResponse>("replyAppend", request);
        }
        // 超过重传次数
        if (falsenum == RETRYNUM)
        {
            // 等待10ms
            printINFO("error!num%d send log to num %d raft fail !", raft->m_id_,
                      id);
            usleep(10000);
            continue;
        }
        auto responce = ret.val();
        // 有日志发送的时候才更新，换而言之如果是心跳消息便不管
        if (logsize >= needIndex)
        {
            if (responce.success)
            {
                {
                    lock_guard<mutex> lock(raft->Loglock_);
                    // 需要注意的是logNum_在用户给日志的时候就会在leader当中直接+1
                    raft->logNum_[needIndex - 1]++;
                    int copyNum = raft->logNum_[needIndex - 1];
                    // 要专门设置个提交线程，依次提交
                    // 需要注意的是，leader仅仅通过计数提交当前任期的日志，对于其它任期，
                    // 因为选举限制，leader选出来没有提交的日志一定是在大多数服务器复制了
                    // 因此能够安全提交，这里处理的逻辑较为麻烦，并且要考虑边界问题
                    if (needIndex > raft->commitIndex_ &&
                        copyNum >= raft->peers_.size() / 2 &&
                        raft->log_[needIndex - 1].term_ == raft->currentTerm_)
                    {
                        printINFO("update commitIndex %d to %d",
                                  raft->commitIndex_, needIndex);
                        raft->commitLog(needIndex);
                    }
                }
                raft->nextIndex_[id]++;
            }
            // 发送失败进行一致性检查
            else
            {
                // 逐步减小发送的日志下表，并且重新发送
                if (raft->nextIndex_[id] > 1)
                {
                    raft->nextIndex_[id]--;
                }
            }
        }
    }
    printINFO("the num%d raft's sendappend thread stop running!", raft->m_id_);
}

// 日志复制的回复，follow才回复
AppendEntriesResponse Raft::replyAppend(AppendEntriesRequest request)
{

    AppendEntriesResponse response;
    response.success = false;
    response.term = currentTerm_;
    /*
        一下代码为模拟日志丢失的场景，在特定情况下将接收失败
    */
    // 这种情况不接收!
    int n = rand() % 10;
    // 模拟1/10的概率不接受日志
    if (n == 1)
    {
        printINFO("because of the network problem, num%d raft not recv log "
                  "from num%d, index is %d",
                  m_id_, request.leaderId, request.PrevLogIndex + 1);
        return response;
    }

    // 如果遇到的不是leader的直接丢掉
    if (request.term < currentTerm_)
    {
        printINFO("num%d raft reject log from num%d, index is %d", m_id_,
                  request.leaderId, request.PrevLogIndex + 1);

        return response;
    }
    {
        lock_guard<mutex> lock(m_lock);
        if (state_ != FOLLOWER)
        {
            state_ = FOLLOWER;
        }
        leaderId_ = request.leaderId;
    }

    // 如果是心跳消息，有心跳消息可能是没有新日志，不能确定完成同步没有
    if (request.logs == "1")
    {
        printINFO("num%d raft recv heart from num%d", m_id_, request.leaderId);
        int commit = request.leaderCommit;
        // 可以同步提交
        if (request.PrevLogIndex == this->log_.size() &&
            (log_.size() == 0 ||
             request.PrevLogTerm == this->log_.back().term_ &&
                 commit < commitIndex_))
        {
            commitLog(commit);
        }
        updateWake();
    }
    // 日志消息
    else
    {
        // 判断是否接收日志
        int index = request.PrevLogIndex, term = request.PrevLogTerm;
        // 判断能接收日志不，要判断没有日志的边界情况
        if (log_.size() >= index &&
            (index == 0 || log_[index - 1].term_ == term))
        {
            updateWake();
            printINFO("num%d raft recv log from num%d, index is %d", m_id_,
                      request.leaderId, request.PrevLogIndex + 1);
            response.success = true;
            // 追加日志
            if (log_.size() == index)
            {
                log_.emplace_back(request.logs, request.term);
                logNum_.push_back(0);
            }
            // 更改日志
            else
            {
                log_[index].command_ = request.logs;
                log_[index].term_ = request.logterm;
            }
            // 最后处理commit
            if (request.leaderCommit > commitIndex_)
            {
                // 提交的和复制的
                commitLog(min(request.leaderCommit, index + 1));
            }
        }
        else
        {
            printINFO("num%d raft reject log from num%d, index is %d", m_id_,
                      request.leaderId, request.PrevLogIndex + 1);
        }
    }
    return response;
}

// 日志提交部分，用一个线程不断
void Raft::commitLoop()
{
    while (!this->dead_)
    {
        if (lastApplied_ < commitIndex_)
        {
            notifyClient();
            applyLog(log_[lastApplied_], lastApplied_ + 1);
            lastApplied_++;
        }
        else
        {
            // 等待唤醒，以防线程一直循环导致浪费cpu
            eventfd_t value;
            eventfd_read(applyfd_, &value);
        }
    }
}

// 日志的应用这里写在文件里
void Raft::applyLog(LogEntry &log, int index)
{
    if (state_ == LEADER)
    {
        notifyClient();
    }
    LOG_INFO("The number %d service apply Loger success! Log index: %d, log "
             "term: %d \nlog contentL:[%s].",
             this->m_id_, index, log.term_, log.command_.c_str());
    // 模拟处理时间
    sleep(1);
}

void Raft::notifyClient()
{
    cout << "the num" << leaderId_ << " server notify client success!" << endl;
}

void Raft::saveRaft()
{
    char filename[128] = {0};
    sprintf(filename, "raft-%d-persistent.txt", m_id_);
    int fd = open(filename, O_WRONLY | O_CREAT, 0664);
    if (fd == -1)
    {
        perror("persist open");
        exit(EXIT_FAILURE);
    }
    char buf[1024];
    // 写term
    sprintf(buf, "term:%d\n", persister_.term);
    int len = strlen(buf);
    if (len != write(fd, buf, strlen(buf)))
    {
        perror("write term error!");
        exit(EXIT_FAILURE);
    }
    // 写votefor
    sprintf(buf, "votefor:%d\n", persister_.voteFor);
    len = strlen(buf);
    if (len != write(fd, buf, strlen(buf)))
    {
        perror("write term error!");
        exit(EXIT_FAILURE);
    }
    // 写日志
    for (int i = 0; i < persister_.log.size(); ++i)
    {
        sprintf(buf, "logTerm:%d\n", log_[i].term_);
        len = strlen(buf);
        if (len != write(fd, buf, strlen(buf)))
        {
            perror("write term error!");
            exit(EXIT_FAILURE);
        }
        sprintf(buf, "logcontent:%s", log_[i].command_.c_str());
        len = strlen(buf);
        if (len != write(fd, buf, strlen(buf)))
        {
            perror("write term error!");
            exit(EXIT_FAILURE);
        }
    }
    close(fd);
}

void Raft::loadRaft()
{
    char filename[128] = {0};
    sprintf(filename, "raft-%d-persistent.txt", m_id_);
    // 因为系统调用write过于难用，这里使用c++的输入输出流
    ifstream fs;
    fs.open(filename, ios::in);

    if (fs.fail())
    {
        perror("file open failed!");
        exit(EXIT_FAILURE);
    }

    string content;
    // 读取term
    if (!getline(fs, content))
    {
        cout << "content is not complete" << endl;
        return;
    }
    int loc = content.find(':');
    int term = atoi(content.substr(loc + 1, content.size() - loc - 1).c_str());
    currentTerm_ = term;
    // 读取votefor
    if (!getline(fs, content))
    {
        cout << "content is not complete" << endl;
        return;
    }
    loc = content.find(':');
    int votefor =
        atoi(content.substr(loc + 1, content.size() - loc - 1).c_str());
    voteFor_ = term;
    // 读取log
    for (int i = 0; getline(fs, content); ++i)
    {
        loc = content.find(':');
        int logterm =
            atoi(content.substr(loc + 1, content.size() - loc - 1).c_str());
        getline(fs, content);
        loc = content.find(':');
        string log = content.substr(loc + 1, content.size() - loc - 1).c_str();
        if (i < log_.size())
        {
            log_[i].command_ = log;
            log_[i].term_ = logterm;
        }
        else
        {
            log_.emplace_back(log, logterm);
            logNum_.push_back(0);
        }
    }
}
