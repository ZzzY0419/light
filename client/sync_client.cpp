/*
* @Author: dummy
* @Date:   2018-04-09 13:39:25
* @Last Modified by:   dummy
* @Last Modified time: 2018-04-09 15:51:39
*/
#include "sync_client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>


SyncClient::SyncClient() : _sockfd(-1) , _connected(false) { }

SyncClient::~SyncClient() {
    release();
}

void SyncClient::release() {
    if (_sockfd != -1) {
        close(_sockfd);
    }
}

int SyncClient::build_connect(char* ip, int port) {
    if (port == 0 || !ip)  return -1;
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        return -1;
    }
    _sockfd = sockfd;
    _connected = true;
    return _sockfd;
}

int SyncClient::send_data(int sockfd, void* data) {
    assert(_connected);
    char* data_to_send = static_cast<char*>(data);
    int ret = send(sockfd, data_to_send, sizeof(data_to_send), 0);
    if (ret < 0)
        return -1;
    return ret;
}

int SyncClient::recv_data(int sockfd, void* buff) {
    assert(_connected);
    char* buff_to_recv = static_cast<char*>(buff);
    int ret = recv(sockfd, buff_to_recv, sizeof(buff_to_recv) - 1, 0);
    if (ret < 0)
        return -1;
    return ret;
}

int SyncClient::commuicate_process(int sockfd, void* data, void* buff, void* callback) {
    assert(_connected);
    assert(!callback);
    int ret = send_data(sockfd, data);
    if (ret < 0)
        return -1;
    if (!buff)
        return ret;
    ret = recv(sockfd, buff);
    if (ret < 0)
        return -1;
    return ret;
}

