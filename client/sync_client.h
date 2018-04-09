/*
* @Author: dummy
* @Date:   2018-04-09 13:34:30
* @Last Modified by:   dummy
* @Last Modified time: 2018-04-09 15:51:11
*/
#ifndef SYNC_CLIENT_H
#define SYNC_CLIENT_H

#include "client_base.h"

class SyncClient {
public:
    ClientBase();
    int build_connect(char* ip, int port);
    int send_data(int sockfd, void* data);
    int recv_data(int sockfd, void* buff);
    int commuicate_process(int sockfd, void* data, void* buff, void* callback);
    void release();
private:
    int _sockfd;
    bool _connected;
};


#endif