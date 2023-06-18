#include "raft.hpp"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

/*
测试raft的脚本，采用多线程模拟。多进程可能要考虑进程间的通信问题，要设置peer数量，以及同步peerInfo
*/

// 通过传参设置5个raft, raft最好设置为奇数
#define COMMOM_PORT 4567
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("loss parameter of peersNum\n");
        exit(-1);
    }
    int peerNum = atoi(argv[1]);
    if (peerNum % 2 == 0)
    {
        printf(
            "the peersNum should be odd\n"); // 必须传入奇数，这是raft集群的要求
        exit(-1);
    }
    // 设置随机数种子
    srand(time(NULL));
    vector<PeerInfo> peers(peerNum);
    for (int i = 0; i < peerNum; ++i)
    {
        peers[i].peerId_ = i;
        peers[i].appendPort_ = COMMOM_PORT + i;
        peers[i].votePort_ = COMMOM_PORT + i + peers.size();
    }

    Raft *raft = new Raft[peers.size()];
    for (int i = 0; i < peers.size(); i++)
    {
        raft[i].Init(peers, i);
    }

    while (true)
    {
        sleep(2);
        for (int i = 0; i < peers.size(); ++i)
        {
            if (raft[i].getstate() == Raft::LEADER)
            {
                char buf[128];
                time_t now = time(NULL);
                tm *nowtm = localtime(&now);

                sprintf(buf,
                        " %d-%d-%d %d:%d:%d : this is log for raft num%d", nowtm->tm_year + 1900,
                        nowtm->tm_mon + 1, nowtm->tm_mday, nowtm->tm_hour,
                        nowtm->tm_min, nowtm->tm_sec, raft[i].getid());
                string command(buf);
                raft[i].appendlog(command);
            }
        }
    }

    //------------------------------test部分--------------------------
    while (1)
        ;
}