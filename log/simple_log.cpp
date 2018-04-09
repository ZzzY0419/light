/*
* @Author: dummy
* @Date:   2018-04-09 22:01:09
* @Last Modified by:   dummy
* @Last Modified time: 2018-04-09 22:13:22
*/

#include "simple_log.h"

SimpleLog::SimpleLog(string file_path, LOGLEVEL level) : __log_file(file_path) : _log_level(level) {
    /*open file get fd*/

    /*init lock*/
    pthread_mutex_init(&_lock, NULL);
}

SimpleLog::~SimpleLog() {
    pthread_mutex_destroy(&_lock)
}

void* SimpleLog::block_to_write(void* arg) {
    SimpleLog* simple_log = static_cast<SimpleLog*>(arg);
    if (simple_log->_log_queue >= 10) {
        
    }
}