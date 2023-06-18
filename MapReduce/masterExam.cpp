#include<iostream>
#include"master.h"

int main(int argc, char** argv)
{
    Master *m = Master::GetInstance();
    m->Init(argc, argv, 5, 5555);
    m->start();
    return 0;
}
