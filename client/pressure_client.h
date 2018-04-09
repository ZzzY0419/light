/*
* @Author: dummy
* @Date:   2018-04-09 15:00:22
* @Last Modified by:   dummy
* @Last Modified time: 2018-04-09 21:03:19
*/

#ifndef PRESSURE_CLIENT_H
#define PRESSURE_CLIENT_H

#include <sys/time.h>
#include <unordered_map>

class PressureClient {
public:
    struct ThreadArg {
        int _single_request;
        int _single_success;
    };
    PressureClient(char* ip, int port, int concurrency, int request, void* data);
    int thread_init();
    void run();
    void gen_report();
    int nonblock_send(int sockfd, char* data);
    static void* pressure_connect(void* arg);
    struct sockaddr_in _server_addr;
    void* _data;
    unordered_map<pthread_t, ThreadArg> _thread_pool;
private:
    int nonblock_send(int sockfd, char* data);
    int _concurrency;
    int _request;
    int _success;
    struct timeval _begin_time;
    struct timeval _end_time;
};


#endif