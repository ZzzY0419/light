/*
* @Author: dummy
* @Date:   2018-04-09 15:00:22
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-14 21:42:39
*/

#ifndef PRESSURE_CLIENT_H
#define PRESSURE_CLIENT_H

#include <sys/time.h>
#include <unordered_map>
#include <arpa/inet.h>

class PressureClient {
public:
    struct ThreadArg {
        int _single_request;
        int _single_success;
    };
    PressureClient(const char* ip, int port, int concurrency, int request, void* data);
    void run();
    struct sockaddr_in _server_addr;
    void* _data;
    std::unordered_map<pthread_t, ThreadArg> _thread_pool;
private:
    void gen_report();
    int thread_init();
    int nonblock_send(int sockfd, char* data);
    static void* pressure_connect(void* arg);
    int _concurrency;
    int _request;
    int _success;
    struct timeval _begin_time;
    struct timeval _end_time;
};


#endif