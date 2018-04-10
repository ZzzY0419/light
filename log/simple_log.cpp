/*
* @Author: dummy
* @Date:   2018-04-09 22:01:09
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-10 17:21:33
*/

#include "simple_log.h"
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h> 
#include <unistd.h>
#include <strings.h>
#include <sys/syscall.h> 

bool SimpleLog::_timeout = false;

SimpleLog::SimpleLog() : _inited(false) {}

SimpleLog::~SimpleLog() {
    if (_inited) {
        pthread_mutex_destroy(&_lock);
        close(_filefd);
    }
}

void SimpleLog::init(const char* file_path, LOGLEVEL level) {
    /*open file get fd*/
    int fd = open(file_path, O_RDWR | O_CREAT | O_APPEND, 0777);
    if (fd > 0) {
        _inited = true;
        _filefd = fd;
        _log_level = level;
    } else {
        return;
    }
    /*init lock*/
    pthread_mutex_init(&_lock, NULL);
}

int SimpleLog::write_log() {
    if (_log_queue.empty()) return 0;
    pthread_mutex_lock(&_lock);
    std::string data;
    while (!_log_queue.empty()) {
        data += _log_queue.front().log_format();
        _log_queue.pop();
    }
    pthread_mutex_unlock(&_lock);
    int count = write(_filefd, data.c_str(), data.length());
    if (count != data.length()) {
        return -1;
    }
    return 0;
}

void* SimpleLog::block_to_write(void* arg) {
    SimpleLog* simple_log = static_cast<SimpleLog*>(arg);
    while (true) {
        if (simple_log->_log_queue.size() >= 10) {
            int ret = simple_log->write_log();
            if (ret == -1)  break;
        }
    }
    return arg;
}

void* SimpleLog::clock_to_write(void* arg) {
    SimpleLog* simple_log = static_cast<SimpleLog*>(arg);
    /*prevent to use lock*/
    bool old_time_out = _timeout;
    while (true) {
        if (_timeout != old_time_out) {
            old_time_out = _timeout;
            int ret = simple_log->write_log();
            if (ret == -1)  break;
        }
    }
    return arg;
}

void SimpleLog::append(LOGLEVEL level, int line, std::string file, std::string content) {
    assert(_inited);
    if (level < _log_level) return;
    time_t t = time(0);
    const int BUFFLEN = 30;
    char timeBuf[BUFFLEN] = {'\0'};
    strftime(timeBuf, BUFFLEN, "%Y-%m-%d %H:%M:%S", localtime(&t));
    LogInfo log(syscall(SYS_gettid), timeBuf, line, file, content);
    pthread_mutex_lock(&_lock);
    _log_queue.push(log);
    pthread_mutex_unlock(&_lock);
}

void SimpleLog::clock_sig_handler(int sig) {
    _timeout = !_timeout;
}

void SimpleLog::run() {
    /*register signal process*/
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = clock_sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    /*create two thread*/
    pthread_create(&_tid_block, NULL, block_to_write, this);
    pthread_create(&_tid_clock, NULL, clock_to_write, this);
    /*set time event*/
    struct itimerval new_value, old_value;
    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_usec = 1;
    new_value.it_interval.tv_sec = 3;
    new_value.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &new_value, &old_value);
}

void SimpleLog::release() {
    /*recycle thread*/
    pthread_join(_tid_block, NULL);
    pthread_join(_tid_clock, NULL);
}