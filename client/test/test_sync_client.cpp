/*
* @Author: triplesheep
* @Date:   2018-04-12 18:36:39
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-12 20:26:47
*/

#include "sync_client.h"
#include <iostream>
#include <strings.h>

int main() {
    SyncClient client;
    const char *ip = "127.0.0.1";
    int port = 3999;
    if (client.build_connect(ip, port) == -1) {
        std::cout << "connect error" << std::endl;
    }
    const char *data = "hello world\n";
    int ret = client.send_data(const_cast<char*>(data));
    if (ret == -1) {
        std::cout << "connect error" << std::endl;
    } else {
        std::cout << "send " << ret << " bytes" << std::endl;
    }
    char buf[100];
    bzero(buf, sizeof(buf));
    ret = client.recv_data(buf, 100);
    if (ret == -1) {
        std::cout << "recv error" << std::endl;
    } else {
        std::cout << "recv " << ret << " bytes" << std::endl;
    }
    std::cout << buf << std::endl;
    bzero(buf, sizeof(buf));
    ret = client.commuicate_process(const_cast<char*>(data), buf, 100);
    if (ret == -1) {
        std::cout << "commuicate error" << std::endl;
    }
    std::cout << buf << std::endl;
    return 0;
}