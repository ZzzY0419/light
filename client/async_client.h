/*
* @Author: triplesheep
* @Date:   2018-04-17 20:05:13
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-17 22:15:23
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
    };
    struct EpollData {
        int fd;
        Callback callback;
        void* message;
    };
    AsyncClient(int threadnum = 0, int connect_pool_size = 1000);
    ~AsyncClient();
    init();
    async_single_talk(const char* ip, int port, void* message, fp callback, void* arg);
private:
    struct ThreadArg {
        ThreadArg(int epoll_id, pthread_mutex_t lock) : epoll_id(epoll_id), lock(lock),
                    size(0), run(true) {}
        int epoll_id;
        pthread_mutex_t lock;
        int size;
        bool run;
    };
    int init_threads();
    int unblock_connect(sockaddr_in* addr, void* message, fp callback, void* arg);
    int epoll_event_add(EpollData* data);
    static void* async_connect(void* arg);
    static void* epoll_thread_talk(void* arg);
    int _threadnum;
    unordered_map<pthread_t, ThreadArg> _threads;
    queue<EpollData> _connect_pool;
    int _connect_pool_size;
};

#endif