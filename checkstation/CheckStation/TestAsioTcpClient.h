#pragma once


#include "stdafx.h"
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include "KxAsioTcpCommon.h"
#include "global.h"
#include <iostream>
#include "KxAlogrithm.h"


//< �̳�INetCallback������������Ӻ����ݺ�Ĳ���
class CTestAsioTcpClient : public INetCallback, public CKxAsioTcpClient
{
public:
	CTestAsioTcpClient(int nStationID, const char *c_pszAddress, int nListenPort);
	~CTestAsioTcpClient(); 
	// �Զ��嶨��ص��ӿ�
	virtual void OnNewConnection(int nStationID);
	virtual void OnRecvData(int nStationID, const unsigned char* pData, size_t nDataSize); 
	virtual void OnDisconnected(int nStationID); 
	virtual void OnRecvHeartBeat(int nStationID); 
	virtual void OnException(const char *c_pszFile, int nLine, const char *c_pszErrorInfo);//!< ��������ʱ������������Ϣ(����������վ�ϵ�ʱ)

	// �������ݣ�����Ϊվ��ID����Ϣ���ͣ������ͣ����ݰ�ͷ����չ���ݣ���չ���ݳ��ȣ��������ݺ����ݳ���
	void SendMsg(int nStationID, int nMsgType,  int nExtDataSize, const char* pExtData); 
	void SendImgMsg(int nStationID, int nMsgType,int nExtDataSize, int nWidth, int nHeight, int nPitch, int nType, const char* pExtData);
	void SendMsg(int nStationID, int nMsgType, const char *pData[], int nSize[], int nLen);  
	//void SendOscImage(const kxCImageBuf &hImage); 
public:
	// boost��ָ�룬boost::shared_ptr<T> ͬ T *
	//boost::shared_ptr<CAsioTcpServer> m_pAsioTcp;
	//boost::shared_ptr<boost::thread> m_pRunThread;  
	unsigned char* m_pSendDataBuffer; 
	int m_nCurSendBufferSize; 
	boost::mutex m_mutex; 
	int m_nCurAreaIndex;
	CKxBaseFunction m_hBaseFun;

private:
	void RecMsgHandshake();
	void RecMsgToStartCheck(const unsigned char* pExtData);
	void RecMsgToStopCheck();
	void RecMsgToStartSimulate(const unsigned char* pExtData);
	void RecMsgToOpenCamera(const unsigned char* pExtData);
	void RecMsgToCloseCamera();
	void RecMsgToSaveAllImg(const unsigned char* pExtData);
	void RecMsgToSaveBadImg(const unsigned char* pExtData);
	void RecMsgToChangeCameraParam(const unsigned char* pExtData);
	void GetA();

	void RecMsgToOpenCamera_BuildModel(const unsigned char* pExtData);
	void RecMsgToCloseCamera_BuildModel(const unsigned char* pExtData);

	void RecMsgToChangeCameraCaptureDirection(int nstatus);
	void RecMsgJustOpenCamera();
	void RecMsgPackId(const unsigned char* pExtData);//�յ�����pack id 

};

extern CTestAsioTcpClient* g_client;

namespace Net 
{
	inline CTestAsioTcpClient* SetAsioTcpClient(int nStationId, const char *c_pszAddress, int nListenPort) 
	{
		g_client = new CTestAsioTcpClient(nStationId, c_pszAddress, nListenPort); 
		return g_client; 
	}
	inline CTestAsioTcpClient* GetAsioTcpClient() 
	{
		return g_client;
	}
	inline bool IsExistNetObj()
	{
		if (NULL == g_client)
		{
			return false;
		}
		return true;
	}
}; 

