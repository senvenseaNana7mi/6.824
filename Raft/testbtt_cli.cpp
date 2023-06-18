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

int main()
{
    buttonrpc client;
    client.as_client("127.0.0.1", 4567);
    AppendEntriesRequest a1;
    a1.leaderCommit = 1;
    a1.leaderId = 2;
    a1.PrevLogIndex =3;
    a1.PrevLogIndex = 4;
    a1.logs = "123";
    a1.term = 5;
    AppendEntriesRequest aa = client.call<AppendEntriesRequest>("func",a1).val();
    cout<<aa.logs<<endl;
    return 0;

}