/*
* @Author: triplesheep
* @Date:   2018-04-17 21:22:37
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-17 22:26:35
*/

#include "async_client.h"
#include "simple_log.h"
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

int init() {
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
        bool empty = true;
        while (!client->_connect_pool.empty()) {
            /*error here, may be i need smart point*/
            int connect = client->_connect_pool.front().fd;
            client->_connect_pool.pop();
            struct epoll_event event;
            event.data.fd = connect;
            event.events = EPOLLOUT;
            epoll_ctl(thread_arg.epoll_id, EPOLL_CTL_ADD, connect, &event);
            empty = false;
        }
        if (!emtpy) {
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
                        /*epoll_event_add();*/
                        epoll_ctl(thread_arg.epoll_id, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    }
                }
                pthread_mutex_unlock(&thread_arg.lock);
            }
        }
    }
}