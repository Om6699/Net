#include "Service/NetService.h"
#include <iostream>

int main(int argc, char* argv[])
{
    if(argc < 2) 
    {
        std::cout << "缺少参数" << std::endl;
        return 0;
    }
    NetService server(2000,std::atoi(argv[1]));
    server.run();
    return 0;
}