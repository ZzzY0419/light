/*
* @Author: dummy
* @Date:   2018-04-09 13:39:25
* @Last Modified by:   triplesheep
* @Last Modified time: 2018-04-12 20:26:16
*/
#include "sync_client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>

SyncClient::SyncClient() : _sockfd(-1) , _connected(false) { }

SyncClient::~SyncClient() {
    release();
}

void SyncClient::release() {
    if (_sockfd != -1) {
        close(_sockfd);
    }
}

int SyncClient::build_connect(const char* ip, int port) {
    if (port == 0 || !ip)  return -1;
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
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

int SyncClient::send_data(void* data) {
    assert(_connected);
    char* data_to_send = static_cast<char*>(data);
    int ret = send(_sockfd, data_to_send, strlen(data_to_send), 0);
    if (ret < 0)
        return -1;
    return ret;
}

int SyncClient::recv_data(void* buff, int buff_len) {
    assert(_connected);
    char* buff_to_recv = static_cast<char*>(buff);
    int ret = recv(_sockfd, buff_to_recv, buff_len - 1, 0);
    if (ret == 0) {
        close(_sockfd);
        _connected = false;
        return 0;
    }
    if (ret < 0)
        return -1;
    return ret;
}

int SyncClient::commuicate_process(void* data, void* buff, int buff_len) {
    assert(_connected);
    int ret = send_data(data);
    if (ret < 0)
        return -1;
    if (!buff)
        return ret;
    ret = recv_data(buff, buff_len);
    if (ret < 0)
        return -1;
    return ret;
}

