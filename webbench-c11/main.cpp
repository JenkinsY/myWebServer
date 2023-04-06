#include<iostream>
#include"process.h"
using namespace std;
extern PROCESS process;
int main(int argc,char* argv[])
{
    return process.start(argc,argv);
    
}