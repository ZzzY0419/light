/*
* @Author: dummy
* @Date:   2018-04-09 21:34:16
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-10 15:30:45
*/

#ifndef LOG_INFO_H
#define LOG_INFO_H

#include <pthread.h>
#include <string>

class LogInfo {
public:
    LogInfo(pthread_t tid, std::string time, int line, std::string file, std::string content) :
            _tid(tid), _time(time), _content(content), _line(line), _file(file) {}
    int _line;
    pthread_t _tid;
    std::string _file;
    std::string _time;
    std::string _content;
    std::string log_format() {
        return _time + " " + _file + " : " + std::to_string(_line) + " tid:"
                + std::to_string(_tid) + " [" + _content + "]" + "\n";
    }
};

#endif