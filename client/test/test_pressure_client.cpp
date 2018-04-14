/*
* @Author: triplesheep
* @Date:   2018-04-12 18:36:54
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-14 22:34:13
*/

#include "pressure_client.h"
#include <iostream>

int main() {
    const char *ip = "127.0.0.1";
    int port = 3999;
    int concurrency = 0;
    int request = 10000;
    const char *data = "Test server pressure\n";
    PressureClient client(ip, port, concurrency, request, const_cast<char*>(data));
    client.run();
    std::cout << " end " << std::endl;
}
