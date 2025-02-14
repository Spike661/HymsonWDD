#pragma once

#include <stdint.h>
#include <wtypes.h>

//宏定义控制DEBUG输出信息 
//#define __DEBUG__  
#ifdef __DEBUG__
//#define DEBUG(format, ...) printf("File: %s, Line: %05d: \n" format "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif


#ifdef __cplusplus
extern "C" {
#endif

class MSocket
{
private:
    char sendbuf[1500];
    char rcvbuf[1500];
	sockaddr_in remoteAddr;  // 保存远程地址

public:
	MSocket(char link);
	MSocket();
	~MSocket();
	WSADATA wsd;			//WSADATA Variable  
	SOCKET sc;				//socket client
	int LinkStus;
	void Close();
	short CsConnect(char *addr, short port);
	short SendData(char *c, int sz);
	short RecvData(char *c);
};
#ifdef __cplusplus
}
#endif