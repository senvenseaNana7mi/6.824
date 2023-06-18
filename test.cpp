#include<string>
#include<iostream>
#include<stdarg.h>

using namespace std;

void printINFO(string format,...)
{
    time_t now = time(NULL);
    tm *nowtm = localtime(&now);

    char time_buf[128] = {0};
    sprintf(time_buf, "%d-%d-%d %d:%d:%d  : ",
        nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday,
        nowtm->tm_hour, nowtm->tm_min, nowtm->tm_sec);
    cout<<time_buf;
    
    va_list st;
    va_start(st, format);
    char buf[1024];
    vsprintf(buf, format.c_str(), st);
    va_end(st);
    cout<<buf<<endl;
}

int main()
{
    printINFO("%s-%s-%d","first","second",1);
    return 0;
}