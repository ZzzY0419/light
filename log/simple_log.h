/*
* @Author: dummy
* @Date:   2018-04-09 21:47:27
* @Last Modified by:   dummy
* @Last Modified time: 2018-04-09 22:13:14
*/

#ifndef SIMPLE_LOG_H
#define SIMPLE_LOG_H

#include "log_info.h"

class SimpleLog {
public:
    enum LOGLEVEL {
        LOG_TRACE = 0,
        LOG_WARN,
        LOG_ERROR,
        LOG_FATAL
    };
    static SimpleLog* get_instance() {
        static SimpleLog instance;
        return &instance;
    }
    int run();
    int append(string content);
    static void* clock_to_write(void* arg);
    static void* block_to_write(void* arg);
    bool inited = false;
    string _log_file;
    int _filefd;
    pthread_mutex_t _lock;
    LOGLEVEL _log_level;
    queue<LogInfo> _log_queue;
private:
    SimpleLog(string file_path, LOGLEVEL level);
    int write_log(string log);
};

#endif