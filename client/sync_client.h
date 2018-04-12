/*
* @Author: dummy
* @Date:   2018-04-09 13:34:30
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-12 20:26:32
*/
#ifndef SYNC_CLIENT_H
#define SYNC_CLIENT_H


class SyncClient {
public:
    SyncClient();
    ~SyncClient();
    int build_connect(const char* ip, int port);
    int send_data(void* data);
    int recv_data(void* buff, int buff_len);
    int commuicate_process(void* data, void* buff, int buff_len);
    void release();
private:
    int _sockfd;
    bool _connected;
};


#endif