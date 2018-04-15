/*
* @Author: dummy
* @Date:   2018-04-09 21:47:27
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-12 20:34:29
*/


#ifndef SIMPLE_LOG_H
#define SIMPLE_LOG_H

#include "log_info.h"
#include <signal.h>
#include <queue>

/* Recently, the log macro already support args */
#define SIMPLE_LOG_TRACE(fmt, ...) {\
    char buff[300] = {'\0'};\
    sprintf(buff, fmt, ##__VA_ARGS__);\
    SimpleLog::get_instance()->append(SimpleLog::LOG_TRACE, __LINE__, __FILE__, buff);\
}
#define SIMPLE_LOG_WARN(fmt, ...) {\
    char buff[300] = {'\0'};\
    sprintf(buff, fmt, ##__VA_ARGS__);\
    SimpleLog::get_instance()->append(SimpleLog::LOG_WARN, __LINE__, __FILE__, buff);\
}
#define SIMPLE_LOG_ERROR(fmt, ...) {\
    char buff[300] = {'\0'};\
    sprintf(buff, fmt, ##__VA_ARGS__);\
    SimpleLog::get_instance()->append(SimpleLog::LOG_ERROR, __LINE__, __FILE__, buff);\
}
#define SIMPLE_LOG_FATAL(fmt, ...) {\
    char buff[300] = {'\0'};\
    sprintf(buff, fmt, ##__VA_ARGS__);\
    SimpleLog::get_instance()->append(SimpleLog::LOG_FATAL, __LINE__, __FILE__, buff);\
}

class SimpleLog {
public:
    enum LOGLEVEL {
        LOG_TRACE = 0,
        LOG_WARN,
        LOG_ERROR,
        LOG_FATAL
    };
    ~SimpleLog();
    static SimpleLog* get_instance() {
        static SimpleLog log;
        return &log;
    }
    int init(const char* file_path, LOGLEVEL level);
    void release_thread();
    void append(LOGLEVEL level, int line, std::string file, std::string content);
    static void* block_to_write(void* arg);
    static void clock_sig_handler(int sig);
    static bool _timeout;
    bool _inited;
    bool _run;
    int _filefd;
    pthread_mutex_t _lock;
    LOGLEVEL _log_level;
    std::queue<LogInfo> _log_queue;
private:
    SimpleLog();
    int write_log();
    pthread_t _tid_block;
    struct sigaction _old_act;
};

#endif