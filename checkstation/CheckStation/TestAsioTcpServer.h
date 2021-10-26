#pragma once

#include "stdafx.h"
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include "AsioTcp.h"
#include "global.h"
#include <iostream>
#include "kxCheckTools.h"

//��Ϣ����
enum MessageType
{
	MSG_CHECK_RESULT = 1,	     //�����

	MSG_SEARCH_EDGE_TEST = 101,	 //��վ����ѱ߲��Ժ���
	MSG_PARAM_TemplateImg = 109,	//ģ�����
	MSG_GET_MASK_IMG = 113,	//��ȡ��ģͼ��
	MSG_MASK_PARAMS = 114,	//��ģ����
	MSG_SET_RESIIDUE = 116,	//���òв�ͼ��ƫ��ֵ����
	MSG_GET_RESIIDUE = 117,
	MSG_GET_BLOB_INFO = 118,	//��ȡBlob��Ϣ
	MSG_LEARN_ONE = 119,	//����ѧϰ
	MSG_LOG = 120,	//��־
	MSG_QUEUEERROR = 121,	//���д���
	MSG_LOAD_VARIINO_DATA = 122,	//������������ļ�
	MSG_LEARN_DEFECT = 123,    //����ȱ��ѧϰ
	MSG_LEARN_DEFECT_COMPLETED = 124, //����ȱ��ѧϰ���
	MSG_LEARN_ONE_COMPLETED = 125,    //����ѧϰ���


	MSG_START_CAMERA = 125,  //��ʼ�ɼ�
	MSG_STOP_CAMERA = 126,  //ֹͣ�ɼ�
	MSG_CHANGE_EXPOURE_TIME = 127,  //�����ع�ʱ��
	MSG_SEND_REAL_TIME_IMAGE = 128,  //����ʵʱͼ��


	MSG_START_CHECK = 1001,	//��ʼ���
	MSG_STOP_CHECK = 1002,	//ֹͣ���
	MSG_SAVE_IMAGE = 1003,	//��ͼ
	MSG_ONE_IMG = 1004,	//���Ų���
	MSG_ALL_IMG = 1005,	//���Ų���
	MSG_SET_CUR_MODEL = 1007,	//���õ�ǰģ��

};

//< �̳�INetCallback������������Ӻ����ݺ�Ĳ���
class CTestAsioTcpServer : public INetCallback
{
public:
	CTestAsioTcpServer(int nListenPort);
	~CTestAsioTcpServer(); 
	// �Զ��嶨��ص��ӿ�
	virtual void OnNewConnection(socket_handle newSocket);
	virtual void OnRecvData(socket_handle socket, const unsigned char* pData, size_t nDataSize); 
	// �������ݣ�����Ϊվ��ID����Ϣ���ͣ������ͣ����ݰ�ͷ����չ���ݣ���չ���ݳ��ȣ��������ݺ����ݳ���
	void SendMsg(int nStationID, int nMsgType,  int nExtDataSize, const char* pExtData); 
	void SendImgMsg(int nStationID, int nMsgType,int nExtDataSize, int nWidth, int nHeight, int nPitch, int nType, const char* pExtData);
	void SendMsg(int nStationID, int nMsgType, const char *pData[], int nSize[], int nLen);
	// �����̺߳��������տͻ������Ӻ����ݣ��ڹ��캯���������߳�
	static void Start(CTestAsioTcpServer *pServer); 
	std::string FormatIntArrayToString(const int* a, int n);
public:
	// boost��ָ�룬boost::shared_ptr<T> ͬ T *
	boost::shared_ptr<CAsioTcpServer> m_pAsioTcp;
	boost::shared_ptr<boost::thread> m_pRunThread;  
	unsigned char* m_pSendDataBuffer; 
	int m_nCurSendBufferSize; 
	boost::mutex m_mutex; 
	int m_nCurAreaIndex;

	CKxBaseFunction          m_hBaseFun;
	kxCImageBuf  m_TmpImg[3];
	kxCImageBuf  m_InvertImg;


	
};

extern CTestAsioTcpServer* g_Server;
