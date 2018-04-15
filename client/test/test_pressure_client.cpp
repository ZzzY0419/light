/*
* @Author: triplesheep
* @Date:   2018-04-12 18:36:54
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-15 19:22:44
*/

#include "pressure_client.h"
#include "simple_log.h"

int main() {
    /*init log*/
    SimpleLog* log = SimpleLog::get_instance();
    if (log->init("./simple.log", SimpleLog::LOG_TRACE) != 0) {
        return 0;
    }
    /*init client*/
    const char *ip = "127.0.0.1";
    int port = 3999;
    int concurrency = 0;
    int request = 10000;
    const char *data = "Test server pressure\n";
    PressureClient client(ip, port, concurrency, request, const_cast<char*>(data));
    client.run();
    SIMPLE_LOG_TRACE("Test pressure client end");
    return 0;
}
