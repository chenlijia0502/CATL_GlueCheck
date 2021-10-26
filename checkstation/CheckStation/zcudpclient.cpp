#include "stdafx.h"
#include "zcudpclient.h"

ZCUDP_Client::ZCUDP_Client()
{
	int iErrorCode;
	if (WSAStartup(MAKEWORD(2, 1), &m_wsaData)) //����Windows Sockets DLL 
	{
		printf("Winsock�޷���ʼ��!\n");
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
	m_server.sin_port = htons(listenport); ///server�ļ����˿� 
	m_server.sin_addr.s_addr = inet_addr(hardwareip); ///server�ĵ�ַ 
	// bind�ͻ��˶˿�(������Ϊ�˷���Ӳ����ͬ�£����ǵķ�װ���޷���ȡ�׽�����Ķ˿ڵ�ַ��
	//��������ֻ�ܸ��ݽ��յ����ݵ�������ָ���ظ���Щ�ض��Ŀڣ�����ͻ��˸�Ӳ������������
	//���������ֽ���01����������Ϊ��ɶ���֮��Ҫ��01�˿ںŻظ�������bind����˿ںž���Ϊ��
	//���վ�ܽ��յ�) 
	m_local.sin_family = AF_INET;
	m_local.sin_port = htons(localport);
	m_local.sin_addr.s_addr = inet_addr(localip); ///client�ĵ�ַ 
	bind(m_socketclient, (struct sockaddr*)&m_local, sizeof(m_local)); //�����ǿͻ��˻��Ƿ���������bind

	struct timeval timeout = { 1, 0 };
	//���÷��ͳ�ʱ
	setsockopt(m_socketclient, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
	//���ý��ճ�ʱ
	setsockopt(m_socketclient, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
}



void ZCUDP_Client::write(char *data, int size)
{
	int len = sizeof(m_server);

	int result = sendto(m_socketclient, data, size, 0, (struct sockaddr*)&m_server, len);
	if (result == -1)
		std::cout << "���� ʧ��" << std::endl;
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
			data[i] = int(unsigned char(recbuffer[i]));//ֱ��תint���ɲ�����ʽ
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