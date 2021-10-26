#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
#include <Winsock2.h> 
#include "stdafx.h"
#include <stdio.h> 
#include <iostream>
#include <tchar.h>
#pragma  comment(lib, "WS2_32.lib")

/*!
	与硬件网络通信，UDP，每个站代表一个客户端，每个客户端有个独立的接收端口号，
	该端口号跟站号对应，也即ini配置里的NetStationId
*/
class ZCUDP_Client{
public:

	ZCUDP_Client();
	~ZCUDP_Client();

	void setip(char *hardwareip, char *localip);//设置硬件、本地IP
	void setportandip(int listenport, int localport, char *hardwareip, char *localip);// 设置网络端口，listenport是服务器的接收口，localport是本地口
	void write(char *data, int size);//发送的数据是16进制的，0x11这种
	void read(int *data, int size);//data放置接收结果，需预先分配内存
	//void clearbuf();//清除接收队列里的数据（临时写法）

	
private:
	WSADATA m_wsaData;

	SOCKET m_socketclient;
	struct sockaddr_in m_server;
	struct sockaddr_in m_local;

	char *m_chardwareip;
	char *m_clocalip;

};

extern ZCUDP_Client g_udpclient;

namespace Net_UDP
{
	inline ZCUDP_Client& Getudp_client()
	{
		return g_udpclient;
	}

}


