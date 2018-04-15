/*
* @Author: dummy
* @Date:   2018-04-09 15:27:30
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-15 20:19:53
*/

#include "pressure_client.h"
#include "simple_log.h"
#include <sys/sysinfo.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <error.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>

const int MAX_EVENT_NUMBER = 10000;

PressureClient::PressureClient(const char* ip, int port, int concurrency, int request, void* data) : 
                            _concurrency(concurrency), _request(request), _data(data), _success(0) {
    bzero(&_server_addr, sizeof(_server_addr));
    _server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &_server_addr.sin_addr);
    _server_addr.sin_port = htons(port);
    if (_concurrency <= 1)
        _concurrency = get_nprocs();
    SIMPLE_LOG_TRACE("Concurrency: %d, total request %d", _concurrency, _request);
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
        SIMPLE_LOG_TRACE("Num: %d, tid: %d, single request: %d", i, static_cast<int>(tid), arg._single_request);
    }
    return 0;
}

int PressureClient::nonblock_send(int sockfd, char* data) {
    int len = strlen(data);
    while(true) {
        int send_ret = send(sockfd, data, len, 0);
        if (send_ret < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                SIMPLE_LOG_WARN("Nonblock_send fail");
                return -1;
            }
        } else {
            if (send_ret == len) {
                SIMPLE_LOG_WARN("Nonblock_send success");
                return 0;
            }
            else {
                len -= send_ret;
                data += send_ret;
                SIMPLE_LOG_WARN("Nonblock_send while");
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
    if (fd_count <= 0)  return arg;
    int epollfd = epoll_create(fd_count);
    epoll_event events[MAX_EVENT_NUMBER];
    char* data = static_cast<char*>(pressure_client->_data);
    for (int i = 0; i < fd_count; ++i) {
        int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        /*Set Addr reuse*/
        int reuse = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        int ret = connect(sockfd, (struct sockaddr*)&(pressure_client->_server_addr), 
                    sizeof(pressure_client->_server_addr));
        if (ret == 0) {
            int send_ret = pressure_client->nonblock_send(sockfd, data);
            if (send_ret == 0)  ++success;
            --fd_count; /* use this to check for empty epoll event listen*/
            close(sockfd);
            SIMPLE_LOG_WARN("Connect recently");
        } else if (errno == EINPROGRESS){
            struct epoll_event event;
            event.data.fd = sockfd;
            event.events = EPOLLOUT | EPOLLET;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
            SIMPLE_LOG_WARN("Wait for epoll to connect");
        } else {
            --fd_count;
            close(sockfd);
            SIMPLE_LOG_ERROR("Connect error: %s", strerror(errno));
        }
    }

    while (fd_count) {
        /*timeout s*/
        int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, 30000);
        if (ret < 0) {
            break;
        }
        if (ret == 0) {
            SIMPLE_LOG_ERROR("Epoll timeout");
            break;
        }
        for (int i = 0; i < ret; ++i) {
            int sockfd = events[i].data.fd;
            int error = 0;
            socklen_t length = sizeof(error);
            if (events[i].events & EPOLLOUT) {
                if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0) {
                    --fd_count;
                    close(sockfd);
                    SIMPLE_LOG_WARN("Epoll getsockopt error");
                    continue;
                }
                if (error != 0) {
                    --fd_count;
                    close(sockfd);
                    SIMPLE_LOG_WARN("Epoll getsockopt get error");
                    continue;
                }
                int send_ret = pressure_client->nonblock_send(sockfd, data);
                if (send_ret == 0)  ++success;
                --fd_count;
            }
            close(sockfd);
        }
    }
    pressure_client->_thread_pool[tid]._single_success = success;
    SIMPLE_LOG_TRACE("Thread end");
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
    SIMPLE_LOG_TRACE("Concurrency: %d, request: %d, success: %d", _concurrency, _request, _success);
}


