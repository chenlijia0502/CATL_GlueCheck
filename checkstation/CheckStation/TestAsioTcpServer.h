#pragma once

#include "stdafx.h"
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include "AsioTcp.h"
#include "global.h"
#include <iostream>
#include "kxCheckTools.h"

//消息类型
enum MessageType
{
	MSG_CHECK_RESULT = 1,	     //检测结果

	MSG_SEARCH_EDGE_TEST = 101,	 //主站点击搜边测试后发送
	MSG_PARAM_TemplateImg = 109,	//模板参数
	MSG_GET_MASK_IMG = 113,	//获取掩模图像
	MSG_MASK_PARAMS = 114,	//掩模参数
	MSG_SET_RESIIDUE = 116,	//设置残差图和偏移值参数
	MSG_GET_RESIIDUE = 117,
	MSG_GET_BLOB_INFO = 118,	//获取Blob信息
	MSG_LEARN_ONE = 119,	//单张学习
	MSG_LOG = 120,	//日志
	MSG_QUEUEERROR = 121,	//队列错误
	MSG_LOAD_VARIINO_DATA = 122,	//导入号码配置文件
	MSG_LEARN_DEFECT = 123,    //单个缺陷学习
	MSG_LEARN_DEFECT_COMPLETED = 124, //单张缺陷学习完成
	MSG_LEARN_ONE_COMPLETED = 125,    //单张学习完成


	MSG_START_CAMERA = 125,  //开始采集
	MSG_STOP_CAMERA = 126,  //停止采集
	MSG_CHANGE_EXPOURE_TIME = 127,  //调整曝光时间
	MSG_SEND_REAL_TIME_IMAGE = 128,  //发送实时图像


	MSG_START_CHECK = 1001,	//开始检测
	MSG_STOP_CHECK = 1002,	//停止检测
	MSG_SAVE_IMAGE = 1003,	//存图
	MSG_ONE_IMG = 1004,	//单张测试
	MSG_ALL_IMG = 1005,	//多张测试
	MSG_SET_CUR_MODEL = 1007,	//设置当前模板

};

//< 继承INetCallback，定义接收连接和数据后的操作
class CTestAsioTcpServer : public INetCallback
{
public:
	CTestAsioTcpServer(int nListenPort);
	~CTestAsioTcpServer(); 
	// 自定义定义回调接口
	virtual void OnNewConnection(socket_handle newSocket);
	virtual void OnRecvData(socket_handle socket, const unsigned char* pData, size_t nDataSize); 
	// 发送数据，参数为站点ID，消息类型，子类型，数据包头的扩展数据，扩展数据长度，超长数据和数据长度
	void SendMsg(int nStationID, int nMsgType,  int nExtDataSize, const char* pExtData); 
	void SendImgMsg(int nStationID, int nMsgType,int nExtDataSize, int nWidth, int nHeight, int nPitch, int nType, const char* pExtData);
	void SendMsg(int nStationID, int nMsgType, const char *pData[], int nSize[], int nLen);
	// 网络线程函数，接收客户端连接和数据，在构造函数中启动线程
	static void Start(CTestAsioTcpServer *pServer); 
	std::string FormatIntArrayToString(const int* a, int n);
public:
	// boost的指针，boost::shared_ptr<T> 同 T *
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
