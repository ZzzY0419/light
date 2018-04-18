/*
* @Author: triplesheep
* @Date:   2018-04-17 21:22:37
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-18 22:29:59
*/

#include "async_client.h"
#include "simple_log.h"
#include <limits.h>
#include <sys/sysinfo.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <error.h>
#include <pthread.h>

AsyncClient(int threadnum = 0, int connect_pool_size = 1000) : _threadnum(threadnum),
                                                _connect_pool_size(connect_pool_size) {
    if (_threadnum == 0) {
        _threadnum = get_nprocs();
    }
    _threadnum = max(2, _threadnum);
    if (_connect_pool_size < 1000) {
        _connect_pool_size = 1000;
    }
}

~AsyncClient() {
    /* clean the half connected socket*/
    for (auto connect : _connect_pool) {
        close(connect);
    }
    /* clean the thread and thread lock*/
    for (auto thread : _threads) {
        thread.second.run = false;
    }
    for (auto thread : _threads) {
        thread_joins(thread.first);
        pthread_mutex_destroy(&thread.second.lock);
    }
}

void init() {
    init_threads();
}

void init_threads() {
    pthread_t tid1;
    int epoll1 = epoll_create(_connect_pool_size);
    pthread_mutex_t lock1;
    pthread_mutex_init(&lock1, NULL);
    pthread_create(&tid1, NULL, async_connect, this);
    ThreadArg arg1(epoll1, lock1);
    _threads[tid1] = arg1;

    for (int i = 1; i <= _threadnum; ++i) {
        pthread_t tid;
        int epoll = epoll_create(_connect_pool_size);
        pthread_mutex_t lock;
        pthread_mutex_init(&lock, NULL);
        pthread_create(&tid, NULL, epoll_thread_talk, this);
        ThreadArg arg(epoll, lock);
        _threads[tid] = arg;
    }
}

void* async_connect(void* arg) {
    AsyncClient* client = static_cast<AsyncClient*>(arg);
    pthread_t tid = pthread_self();
    ThreadArg& thread_arg = client->_threads[tid];
    struct epoll_event events[client->_connect_pool_size];
    while (thread_arg.run) {
        pthread_mutex_lock(&thread_arg.lock);
        while (!client->_connect_pool.empty()) {
            EpollData* data = client->_connect_pool.front();
            int connect = data->fd;
            client->_connect_pool.pop();
            struct epoll_event event;
            event.data.ptr = data;
            event.events = EPOLLOUT;
            if (epoll_ctl(thread_arg.epoll_id, EPOLL_CTL_ADD, connect, &event) == 0)
                ++thread_arg.size;
        }
        pthread_mutex_unlock(&thread_arg.lock);
        if (thread_arg.size) {
            pthread_mutex_lock(&thread_arg.lock);
            int ret = epoll_wait(thread_arg.epoll_id, events, client->_connect_pool_size, 1000);
            if (ret < 0) {
                pthread_mutex_unlock(&thread_arg.lock);
                return this;
            }
            if (ret == 0) {
                pthread_mutex_unlock(&thread_arg.lock);
                continue;
            }
            for (int i = 0; i < ret; ++i) {
                int error = 0;
                socklen_t length = sizeof(error);
                if (events[i].events & EPOLLOUT) {
                    int result = getsockopt(events[i].data.fd, SOL_SOCKET, SO_ERROR, &error, &length);
                    if (result < 0) {
                        pthread_mutex_unlock(&thread_arg.lock);
                        continue;
                    }
                    if (errno == 0) {
                        EpollData* data = static_cast<EpollData*>(events[i].data.ptr);
                        epoll_event_add(data)
                        epoll_ctl(thread_arg.epoll_id, EPOLL_CTL_DEL, data->fd, NULL);
                    }
                }
                pthread_mutex_unlock(&thread_arg.lock);
            }
            thread_arg.size -= ret;
        }
    }
    return arg;
}

void epoll_event_add(EpollData* data) {
    /*chose the thread with min fd size*/
    pthread_t tid;
    int min_fd = INT_MAX;
    bool begin = true;
    for (auto thread : _threads) {
        if (begin) {    /*skip connect thread*/
            begin = false;
            continue;
        }
        if (thread.second.size > min_fd) {
            min_fd = thread.second.size;
            tid = thread.first;
        }
    }
    /*add event in this thread epoll*/
    pthread_mutex_lock(_threads[tid].lock);
    struct epoll_event event;
    event.data.ptr = data;
    event.events = EPOLLOUT;
    /*reset fd block*/
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & (~O_NONBLOCK));  /*use ~ and & to del*/
    if (epoll_ctl(_threads[tid].epoll_id, EPOLL_CTL_ADD, data->fd, &event) == 0)
        ++_threads[tid].size;
    pthread_mutex_unlock(_threads[tid].lock);
    return;
}

void* epoll_thread_talk(void* arg) {
    AsyncClient* client = static_cast<AsyncClient*>(arg);
    pthread_t tid = pthread_self();
    ThreadArg& thread_arg = client->_threads[tid];
    struct epoll_event events[client->_connect_pool_size];
    while (thread_arg.run) {
        if (thread_arg.size) {
            pthread_mutex_lock(&thread_arg.lock);
            int ret = epoll_wait(thread_arg.epoll_id, events, client->_connect_pool_size, 1000);
            if (ret < 0) {
                pthread_mutex_unlock(&thread_arg.lock);
                break;
            }
            if (ret == 0) {
                pthread_mutex_unlock(&thread_arg.lock);
                continue;
            }
            for (int i = 0; i < ret; ++i) {
                EpollData* data = static_cast<EpollData*>(events[i].data.ptr);
                if (events[i].event | EPOLLOUT) {
                    char* message = static_cast<char*>(data->message);
                    if (send(data->fd, message, strlen(message), 0) == strlen(message)) {
                        struct epoll_event event;
                        event.data.ptr = data;
                        event.events = EPOLLOIN;
                        epoll_ctl(thread_arg.epoll_id, EPOLL_CTL_MOD, data->fd, &event);
                    } else {
                        epoll_ctl(thread_arg.epoll_id, EPOLL_CTL_DEL, data->fd, NULL);
                        close(data->fd);
                        --thread_arg.size;
                    }
                } else if (events[i].event | EPOLLIN) {
                    if (recv(data->fd, data->callback.arg, data->callback.len, 0) > 0) {
                        data->callback.callback(data->callback.arg);    /*call user's callback*/
                    }
                    epoll_ctl(thread_arg.epoll_id, EPOLL_CTL_DEL, data->fd, NULL);
                    close(data->fd);
                    --thread_arg.size;
                }
            }
        }
    }
    return arg;
}

void unblock_connect(sockaddr_in addr, EpollData* data) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    data->fd = fd;
    int ret = connect(data->fd, (struct sockaddr*)(&addr), sizeof(addr));
    if (ret == 0) {
        epoll_event_add(data);
    } else if (errno == EINPROGRESS && _connect_pool.size() < _connect_pool_size) {
        _connect_pool.push(data);
    }
}

void async_single_talk(const char* ip, int port, EpollData* data) {
    struct sock_addr addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = port;
    unblock_connect(addr, data);
}