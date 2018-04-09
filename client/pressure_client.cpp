/*
* @Author: dummy
* @Date:   2018-04-09 15:27:30
* @Last Modified by:   dummy
* @Last Modified time: 2018-04-09 22:01:45
*/

#include "pressure_client.h"
#include <sys/sysinfo.h>
#include <pthread.h>

const int MAX_EVENT_NUMBER = 1000;

PressureClient::PressureClient(char* ip, int port, int concurrency, int request, void* data) : 
                            _concurrency(concurrency), _request(request), _data(data) {
    bzero(&_server_addr, sizeof(_server_addr));
    _server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, _server_addr.sin_addr);
    _server_addr.sin_port = htons(port);
    if (_concurrency <= 1)
        _concurrency = get_nprocs();
}

int PressureClient::thread_init() {
    int average = _request / _concurrency;
    average = (average > 0) ? average : 1;
    int count = _request;
    for (int i = 0; i < _concurrency; ++i) {
        PressureClient::ThreadArg arg;
        pthread_t tid;
        arg._single_request = (i == _concurrency - 1) ? count : average;
        arg._single_success = 0;
        count -= average;
        if (pthread_create(&tid, NULL, pressure_connect, this) != 0)
            return -1;
        _thread_pool[tid] = arg;
    }
    return 0;
}

int PressureClient::nonblock_send(int sockfd, char* data) {
    int len = sizeof(data);
    while(true) {
        int send_ret = send(sockfd, data, len, 0);
        if (send_ret < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                return -1;
            }
        } else {
            if (send_ret == len) {
                return 0;
            }
            else {
                len -= send_ret;
                data += send_ret;
            }
        }
    }
}

/*static member function access class member via arg pointer*/
void* PressureClient::pressure_connect(void* arg) {
    PressureClient* pressure_client = static_cast<PressureClient*>(arg);
    pthread_t tid = pthread_self();
    int fd_count = pressure_client->_thread_pool[tid]._single_request;
    int success = 0;
    int epollfd = epoll_create(fd_count);
    epoll_event events[MAX_EVENT_NUMBER];
    char* data = static_cast<char*>(pressure_client->_data);
    for (int i = 0; i < fd_count; ++i) {
        int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        int ret = connect(sockfd, (struct sockaddr*)&(pressure_client->_server_addr), 
                    sizeof(pressure_client->_server_addr));
        if (ret == 0) {
            int send_ret = pressure_client->nonblock_send(sockfd, data);
            if (send_ret == 0)  ++success;
            --fd_count; /* use this to check for empty epoll event listen*/
        } else if (errno == INPROCESS){
            struct epoll_event event;
            event.data.fd = sockfd;
            event.events = EPOLLOUT | EPOLLET;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
        } else {
            --fd_count;
        }
    }

    while (fd_count) {
        int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if (ret < 0) {
            break;
        }
        for (int i = 0; i < ret; ++i) {
            int sockfd = events[i].data.fd;
            int error = 0;
            socklen_t length = sizeof(error);
            if (events[i].events & EPOLLOUT) {
                if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0) {
                    --fd_count;
                    continue;
                }
                if (error != 0) {
                    --fd_count;
                    continue;
                }
                int send_ret = pressure_client->nonblock_send(sockfd, data);
                if (send_ret == 0)  ++success;
                --fd_count;
            }
        }
    }
    pressure_client->_thread_pool[tid]._single_success = success;
    return arg;
}

void PressureClient::run() {
    /*init and start thread*/
    gettimeofday(&_begin_time, NULL);
    thread_init();
    /*recycle thread*/
    for (auto thread : _thread_pool) {
        pthread_join(thread.first, NULL);
        _success += thread.second._single_success;
    }
    gettimeofday(&_end_time, NULL);
    gen_report();
}

void PressureClient::gen_report() {
    long secTime  = _end_time.tv_sec - _begin_time.tv_sec;
    long usecTime = _end_time.tv_usec - _begin_time.tv_usec; 
}


