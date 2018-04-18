/*
* @Author: triplesheep
* @Date:   2018-04-17 20:05:13
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-18 22:24:21
*/
#ifndef ASYNC_CLIENT_H
#define ASYNC_CLIENT_H

#include <pthread.h>
#include <unordered_map>
#include <queue>

class AsyncClient {
public:
    typedef void* (*func)(void*) fp;
    struct Callback {
        fp callback;
        void* arg;
        int len;
    };
    struct EpollData {
        int fd;
        Callback callback;
        void* message;
    };
    AsyncClient(int threadnum = 0, int connect_pool_size = 1000);
    ~AsyncClient();
    void init();
    void async_single_talk(const char* ip, int port, EpollData* data);
private:
    struct ThreadArg {
        ThreadArg(int epoll_id, pthread_mutex_t lock) : epoll_id(epoll_id), lock(lock),
                    size(0), run(true) {}
        int epoll_id;
        pthread_mutex_t lock;
        int size;
        bool run;
    };
    void init_threads();
    void unblock_connect(sockaddr_in* addr, EpollData* data);
    void epoll_event_add(EpollData* data);
    static void* async_connect(void* arg);
    static void* epoll_thread_talk(void* arg);
    int _threadnum;
    unordered_map<pthread_t, ThreadArg> _threads;
    queue<EpollData*> _connect_pool;    /*EpollData* new by user, so user control its lifecycle*/
    int _connect_pool_size;
};

#endif