/*
* @Author: dummy
* @Date:   2018-04-09 21:34:16
* @Last Modified by:   dummy
* @Last Modified time: 2018-04-09 22:06:56
*/

#ifndef LOG_INFO_H
#define LOG_INFO_H

#include <pthread.h>
#include <string.h>

class LogInfo {
public:
    LogInfo(pthread_t tid, string time, string content) :
            _tid(tid), _time(time), _content(content) {}
    pthread_t _tid;
    string _time;
    stirng _content;
    string log_format() {
        return _time + " tid:" + to_string(_tid) + " [" + _content + "]";
    }
};

#endif