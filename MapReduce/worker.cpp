#include "worker.hpp"

int ihash(string &str)
{
    int sum = 0;
    for (int i = 0; i < str.size(); ++i)
    {
        sum += (str[i] - 'a');
    }
    return sum % reduce_task_num;
}

KeyValue getContent(string &filename)
{
    KeyValue content;
    content.value.clear();
    content.key = filename;
    if (-1 == access(filename.c_str(), F_OK))
    {
        cout<<filename<<" is not exist"<<endl;
        return content;
    }
    int fd = open(filename.c_str(), O_RDONLY);
    if(fd==-1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    int len = lseek(fd, 0, SEEK_END);
    char buf[len];
    int length = read(fd, buf, len);
    if(length!=len)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    content.key = string(buf);
    return content;
}

