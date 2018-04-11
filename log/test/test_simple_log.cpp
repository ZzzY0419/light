/*
* @Author: triplesheep
* @Date:   2018-04-10 14:04:22
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-11 14:50:10
*/

/*
*   g++ -std=c++11 -o test_simple_log test_simple_log.cpp simple_log.cpp -lpthread
*/

#include "simple_log.h"
#include <pthread.h>
#include <unistd.h>
#include <iostream>

void* thread_func(void* arg) {
    int a = 1;
    SIMPLE_LOG_TRACE("log trace");
    SIMPLE_LOG_WARN("log warn");
    SIMPLE_LOG_ERROR("log error");
    SIMPLE_LOG_FATAL("log fatal");
    SIMPLE_LOG_TRACE("log trace %d", a);
    SIMPLE_LOG_WARN("log warn %d", a);
    SIMPLE_LOG_ERROR("log error %d", a);
    SIMPLE_LOG_FATAL("log fatal %d", a);
}

int main() {
    SimpleLog* log = SimpleLog::get_instance();
    if (log->init("./simple.log", SimpleLog::LOG_TRACE) != 0) {
        return 0;
    }
    pthread_t tid[10];
    for (int i = 0; i < 10; ++i) {
        pthread_create(&tid[i], NULL, thread_func, NULL);
    }
    for (int i = 0; i < 10; ++i) {
        pthread_join(tid[i], NULL);
    }
    int left = 10; /*sleep could be interrupted by signal*/
    while (left) {
        left = sleep(left);
    }
    return 0;
}
