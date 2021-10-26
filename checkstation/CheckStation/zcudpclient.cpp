#include "stdafx.h"
#include "zcudpclient.h"

ZCUDP_Client::ZCUDP_Client()
{
	int iErrorCode;
	if (WSAStartup(MAKEWORD(2, 1), &m_wsaData)) //调用Windows Sockets DLL 
	{
		printf("Winsock无法初始化!\n");
		WSACleanup();
		return;
	}

}

ZCUDP_Client::~ZCUDP_Client()
{

}

void ZCUDP_Client::setip(char *hardwareip, char *localip)
{
	m_chardwareip = hardwareip;
	m_clocalip = localip;
}


void ZCUDP_Client::setportandip(int listenport, int localport, char *hardwareip, char *localip)
{
	m_socketclient = socket(AF_INET, SOCK_DGRAM, 0);
	m_server.sin_family = AF_INET;
	m_server.sin_port = htons(listenport); ///server的监听端口 
	m_server.sin_addr.s_addr = inet_addr(hardwareip); ///server的地址 
	// bind客户端端口(这里是为了服务硬件的同事，他们的封装库无法获取套接字里的端口地址，
	//所以他们只能根据接收的数据的内容来指定回复那些特定的口，比如客户端给硬件服务器发送
	//的数据首字节是01，则他们认为完成动作之后要给01端口号回复，这里bind这个端口号就是为了
	//这个站能接收到) 
	m_local.sin_family = AF_INET;
	m_local.sin_port = htons(localport);
	m_local.sin_addr.s_addr = inet_addr(localip); ///client的地址 
	bind(m_socketclient, (struct sockaddr*)&m_local, sizeof(m_local)); //无论是客户端还是服务器都可bind

	struct timeval timeout = { 1, 0 };
	//设置发送超时
	setsockopt(m_socketclient, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
	//设置接收超时
	setsockopt(m_socketclient, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
}



void ZCUDP_Client::write(char *data, int size)
{
	int len = sizeof(m_server);

	int result = sendto(m_socketclient, data, size, 0, (struct sockaddr*)&m_server, len);
	if (result == -1)
		std::cout << "发送 失败" << std::endl;
}

void ZCUDP_Client::read(int *data, int size)
{
	int len = sizeof(m_server);
	char* recbuffer = new char[size];
	//if (recvfrom(m_socketclient, recbuffer, size, 0, (struct sockaddr*)&m_server, &len) != SOCKET_ERROR)
	if (recv(m_socketclient, recbuffer, size, 0) != SOCKET_ERROR)
	{
		for (int i = 0; i < size; i++)
		{
			data[i] = int(unsigned char(recbuffer[i]));//直接转int会变成补码形式
		}
	}
	//std::cout << std::endl;

	delete recbuffer;
}

//void ZCUDP_Client::clearbuf()
//{
//	int data[6];
//	for (int i = 0; i < 3; i++)
//	{
//		read(data, 6);
//	}
//}




ZCUDP_Client g_udpclient;
namespace Net_UDP{

}