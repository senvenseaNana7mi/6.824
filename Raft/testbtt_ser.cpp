#include<iostream>
using namespace std;
#include<buttonrpc.hpp>
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

AppendEntriesRequest func(AppendEntriesRequest aa)
{
    cout<<aa.logs<<endl;
    aa.logs+="456";
    cout<<aa.logs<<endl;
    return aa;
}
int main()
{
    buttonrpc server;
    server.as_server(4567);
    server.bind("func", func);
    cout<<"server run!"<<endl;
    server.run();

    return 0;
}