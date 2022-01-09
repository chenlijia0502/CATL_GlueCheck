#pragma once


#include "stdafx.h"
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include "KxAsioTcpCommon.h"
#include "global.h"
#include <iostream>
#include "KxAlogrithm.h"


//< 继承INetCallback，定义接收连接和数据后的操作
class CTestAsioTcpClient : public INetCallback, public CKxAsioTcpClient
{
public:
	CTestAsioTcpClient(int nStationID, const char *c_pszAddress, int nListenPort);
	~CTestAsioTcpClient(); 
	// 自定义定义回调接口
	virtual void OnNewConnection(int nStationID);
	virtual void OnRecvData(int nStationID, const unsigned char* pData, size_t nDataSize); 
	virtual void OnDisconnected(int nStationID); 
	virtual void OnRecvHeartBeat(int nStationID); 
	virtual void OnException(const char *c_pszFile, int nLine, const char *c_pszErrorInfo);//!< 发生错误时，传出错误信息(现在用于子站断掉时)

	// 发送数据，参数为站点ID，消息类型，子类型，数据包头的扩展数据，扩展数据长度，超长数据和数据长度
	void SendMsg(int nStationID, int nMsgType,  int nExtDataSize, const char* pExtData); 
	void SendImgMsg(int nStationID, int nMsgType,int nExtDataSize, int nWidth, int nHeight, int nPitch, int nType, const char* pExtData);
	void SendMsg(int nStationID, int nMsgType, const char *pData[], int nSize[], int nLen);  
	//void SendOscImage(const kxCImageBuf &hImage); 
public:
	// boost的指针，boost::shared_ptr<T> 同 T *
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
	void RecMsgPackId(const unsigned char* pExtData);//收到托盘pack id 

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

