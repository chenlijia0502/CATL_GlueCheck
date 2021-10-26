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
	��Ӳ������ͨ�ţ�UDP��ÿ��վ����һ���ͻ��ˣ�ÿ���ͻ����и������Ľ��ն˿ںţ�
	�ö˿ںŸ�վ�Ŷ�Ӧ��Ҳ��ini�������NetStationId
*/
class ZCUDP_Client{
public:

	ZCUDP_Client();
	~ZCUDP_Client();

	void setip(char *hardwareip, char *localip);//����Ӳ��������IP
	void setportandip(int listenport, int localport, char *hardwareip, char *localip);// ��������˿ڣ�listenport�Ƿ������Ľ��տڣ�localport�Ǳ��ؿ�
	void write(char *data, int size);//���͵�������16���Ƶģ�0x11����
	void read(int *data, int size);//data���ý��ս������Ԥ�ȷ����ڴ�
	//void clearbuf();//������ն���������ݣ���ʱд����

	
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


