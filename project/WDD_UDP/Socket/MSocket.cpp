// MSocket.cpp: 实现文件
#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <string>
#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "MSocket.h"


MSocket::MSocket(){

}
MSocket::MSocket(char link){
	LinkStus = link;
}

MSocket::~MSocket(){
	Close();
}

void MSocket::Close()
{
	closesocket(sc);
	WSACleanup();
	LinkStus = 0;
}

short MSocket::CsConnect(char *addr, short port)
{
    //char sIp[] = "127.0.0.1";
    char sIp[] = "192.168.137.1";		    //本地IP地址,自动获取本机地址绑定
    short sport = 6001;				        //本地端口
    // 初始化WSA  
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
        return 1;
    }
    if (LinkStus) {
        return 0;
    }
    // 创建UDP套接字
    sc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // 设置超时时间
    struct timeval timeout;
    timeout.tv_sec = 5000;      // 设置超时时间为 5 s,单位：ms
    timeout.tv_usec = 0;        // 微秒设置为 0
    // 设置套接字选项，应用超时设置
    if (setsockopt(sc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed.\n");
        return -1;
    }
    // 设置UDP接收缓冲区大小，windows默认8K
    int nRecvBuf = 1024000;//设置为1M
    if (setsockopt(sc, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int)) < 0) {
        perror("setsockopt buf size failed.\n");
        return -1;
    }

    socklen_t valSize = sizeof(nRecvBuf);
    getsockopt(sc, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuf, &valSize);
    DEBUG("socket size: %d\n", nRecvBuf);

    if (sc == INVALID_SOCKET) {
        return 1;
    }
    //通信的套接字和本地的IP与端口绑定
    struct sockaddr_in server;
    memset(&server, 0, sizeof(struct sockaddr_in)); //清空结构体
    server.sin_family = AF_INET;
    server.sin_port = htons(sport);   
    server.sin_addr.s_addr = INADDR_ANY;            //让内核自动选择一个网卡
    int ret = bind(sc, (struct sockaddr*)&server, sizeof(server));

    // 保存远程地址
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    inet_pton(AF_INET, addr, &client.sin_addr.s_addr);
    memcpy(&remoteAddr, &client, sizeof(client));

    // 将连接状态设置为已连接
    LinkStus = 1;
    return 0;
}

short MSocket::SendData(char *ch, int sz) 
{
    int rtn = 0;
    if (LinkStus != 1) return 1;
    rtn = sendto(sc, ch, sz, 0, (sockaddr*)&remoteAddr, sizeof(remoteAddr));
    DEBUG("sendtoIP:%s,sendtoPort:%d\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port));
    DEBUG("send data:");
    for (int i = 0; i < sz; ++i) {
        DEBUG("%02x ", static_cast<unsigned char>(ch[i]));
    }
    DEBUG("\n");
    return rtn;
}

short MSocket::RecvData(char *ch)
{
    int rtn = 0;
    int bufferSize = 1500;
    if (LinkStus != 1) return 1;

    sockaddr_in from;
    int fromLen = sizeof(from);

    rtn = recvfrom(sc, rcvbuf, bufferSize, 0, (sockaddr*)&from, &fromLen);

    DEBUG("recvfromIP:%s,recvfromPort:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));
    DEBUG("recv data:");
    for (int i = 0; i < rtn; ++i) {
        DEBUG("%02x ", static_cast<unsigned char>(rcvbuf[i]));
    }
    DEBUG("\n");

    if (rtn > 0) {
        memcpy(ch, rcvbuf, rtn);
    }

    return rtn;
}

