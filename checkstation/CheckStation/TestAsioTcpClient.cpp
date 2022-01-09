#include "stdafx.h"
#include "TestAsioTcpClient.h"
#include "CkxThreadManage.h"
#include "CkxEnvironment.h"
#include "KxRotateCards.h"
#include "kxCheckResult.h"
#include "Grab_Buffer.h"
#include "CkxFileRead.h"
#include "kxParameter.h"
#include "global.h"
#include "KxCheck.h"
#include "GrabPack.h"
//#include "kxPrintf.h"
#include "KxGeneralFun.h"
#include "zcudpclient.h"
#include "global.h"
#include "json.h"
using namespace Check;

CTestAsioTcpClient::CTestAsioTcpClient(int nStationID, const char *c_pszAddress, int nListenPort)
	: CKxAsioTcpClient(nStationID, this, c_pszAddress, nListenPort) 
{
	//m_pAsioTcp = boost::make_shared<CAsioTcpServer>(this, nListenPort);
	// 初始化网络线程并启动，Start是线程函数，this是传入的参数
	//m_pRunThread = boost::shared_ptr<boost::thread>(new boost::thread(Start, this));
	m_pSendDataBuffer = (unsigned char *)calloc(sizeof(NetDataHeader), sizeof(unsigned char)); 
	m_nCurSendBufferSize = sizeof(NetDataHeader);



}


CTestAsioTcpClient::~CTestAsioTcpClient()
{
	//m_pAsioTcp->Stop(); 
	//m_pRunThread->join(); 
	free(m_pSendDataBuffer); 
}

void CTestAsioTcpClient::OnNewConnection(int nStationID)
{
	printf("Server(Station %d) Is Connected!\n", nStationID); 
}


void CTestAsioTcpClient::OnDisconnected(int nStationID)
{
	Connect();
}

void CTestAsioTcpClient::OnRecvHeartBeat(int nStationID)
{
	//std::cout << "OnRecvHeartBeat" << std::endl;
}


void CTestAsioTcpClient::SendImgMsg(int nStationID, int nMsgType, int nExtDataSize, int nWidth, int nHeight, int nPitch, int nType, const char* pExtData)
{
	// HYH 2020.02.13 不用看了，你一般不会用到这个
	boost::unique_lock<boost::mutex> lock(m_mutex);

	int nDataSize = /*sizeof(NetDataHeader) + dataHeader.*/nExtDataSize;
	if (nDataSize > m_nCurSendBufferSize)
	{
		m_pSendDataBuffer = (unsigned char *)realloc(m_pSendDataBuffer, nDataSize * sizeof(unsigned char));
		m_nCurSendBufferSize = nDataSize;
	}
	memcpy(m_pSendDataBuffer + /*sizeof(NetDataHeader)*/+0 * sizeof(int), &nWidth, sizeof(int));
	memcpy(m_pSendDataBuffer + /*sizeof(NetDataHeader)*/+1 * sizeof(int), &nHeight, sizeof(int));
	memcpy(m_pSendDataBuffer + /*sizeof(NetDataHeader)*/+2 * sizeof(int), &nPitch, sizeof(int));
	memcpy(m_pSendDataBuffer + /*sizeof(NetDataHeader)*/+3 * sizeof(int), &nType, sizeof(int));
	memcpy(m_pSendDataBuffer + /*sizeof(NetDataHeader)*/+4 * sizeof(int), pExtData, /*dataHeader.*/nExtDataSize - 4 * sizeof(int));

	CKxAsioTcpClient::SendMsg(/*nStationID, */nMsgType, 0, NULL, 0, (const char *)m_pSendDataBuffer, nDataSize);
}

void CTestAsioTcpClient::SendMsg(int nStationID, int nMsgType, const char *pData[], int nSize[], int nLen)
{
	// HYH 2020.02.13 不用看了，你一般不会用到这个
	boost::unique_lock<boost::mutex> lock(m_mutex);
	int nExtDataSize = 0;
	for (int i = 0; i < nLen; i++)
	{
		nExtDataSize += nSize[i];
	}
	int nDataSize = /*sizeof(NetDataHeader) + dataHeader.*/nExtDataSize;
	if (nDataSize > m_nCurSendBufferSize)
	{
		m_pSendDataBuffer = (unsigned char *)realloc(m_pSendDataBuffer, nDataSize * sizeof(unsigned char));
		m_nCurSendBufferSize = nDataSize;
	}
	//memcpy(m_pSendDataBuffer, &dataHeader, sizeof(NetDataHeader)); 
	int nOffset = 0/*sizeof(NetDataHeader)*/;
	for (int i = 0; i < nLen; i++)
	{
		memcpy(m_pSendDataBuffer + nOffset, pData[i], nSize[i]);
		nOffset += nSize[i];
	}

	CKxAsioTcpClient::SendMsg(/*nStationID, */nMsgType, 0, NULL, 0, (const char *)m_pSendDataBuffer, nDataSize);
}

void CTestAsioTcpClient::OnException(const char *c_pszFile, int nLine, const char *c_pszErrorInfo)
{
	// 关闭相机
	std::cout << "监测到主站连接异常，关闭相机" << std::endl;
	Graber::g_GetGrabPack().Stop();
	Graber::g_GetCamera()->Close();

}


void CTestAsioTcpClient::OnRecvData(int nStationID, const unsigned char* pData, size_t nDataSize)
{
	std::cout << "OnRecvData:" << pData  << " size = " << nDataSize << "bytes" << std::endl;
	

	// 判断数据是否有效，pData带数据包头，所以至少有sizeof(NetDataHeader)这么大
	if (pData && nDataSize >= sizeof(NetDataHeader)) 
	{
		NetDataHeader dataHeader; 
		memcpy(&dataHeader, pData, sizeof(NetDataHeader)); 

		// 检查包头里的数据是否有效，nDataSize的长度为sizeof(NetDataHeader) + 超长数据的长度
		if (dataHeader.nExtDataSize == nDataSize - sizeof(NetDataHeader)) 
		{
			// pExtData为超长数据的指针
			const unsigned char *pExtData = pData + sizeof(NetDataHeader); 
			if (dataHeader.nExtDataSize == 0) 
			{
				pExtData = NULL; 
			}

			// 2020.02.13 
			switch (int(dataHeader.nMsgType))
			{
				case MSG_HANDSHAKE_SEND:
					RecMsgHandshake();
					break;

				case MSG_START_CHECK:
					RecMsgToStartCheck(pExtData);
					break;

				case MSG_STOP_CHECK:
					RecMsgToStopCheck();
					break;

				case MSG_ALL_IMG:
					RecMsgToStartSimulate(pExtData);
					break;

				case MSG_START_CAMERA:
					RecMsgToOpenCamera(pExtData);
					break;

				case MSG_STOP_CAMERA:
					RecMsgToCloseCamera();
					break;

				case MSG_SAVE_IMAGE:
					RecMsgToSaveAllImg(pExtData);
					break;

				case MSG_SAVE_BAD_IMAGE:
					RecMsgToSaveBadImg(pExtData);
					break;

				case MSG_CHANGE_EXPOURE_TIME:
					RecMsgToChangeCameraParam(pExtData);
					break;

				case MSG_A:
					GetA();
					break;

				case MSG_DOT_CHECK_OPEN:
					g_bdotcheckstatus = true;
					RecMsgJustOpenCamera();
					break;
				case MSG_DOT_CHECK_CLOSE:
					g_bdotcheckstatus = false;
					RecMsgToCloseCamera();
					break;
				case MSG_JUST_OPENCAMERA_BUILDMODEL:
					RecMsgToOpenCamera_BuildModel(pExtData);
					break;
				case MSG_JUST_CLOSECAMERA_BUILDMODEL:
					RecMsgToCloseCamera_BuildModel(pExtData);
					break;
				case MSG_CHANGE_CAMERA_INFO_REVERSE:
					RecMsgToChangeCameraCaptureDirection(1);
					break;
				case MSG_RECOVER_CAMERA_INFO_REVERSE:
					RecMsgToChangeCameraCaptureDirection(0);
					break;
				case MSG_JUST_OPENCAMERA:
					RecMsgJustOpenCamera();
					break;
				case MSG_CHANGE_CAPTURE_COL:
					g_Grabstatus.nGrabTimes++;
					break;
				case MSG_CLOSECAMERA:
					Graber::g_GetCamera()->Close();// 主站关闭，子站优先释放相机
					break;
				case MSG_PACK_ID://收到pack id
					RecMsgPackId(pExtData);
					break;

				
				default:
					break;

			}
		}
	}
	
}

void CTestAsioTcpClient::GetA()
{
	//发回A数据
}

void CTestAsioTcpClient::SendMsg(int nStationID, int nMsgType, int nExtDataSize, const char* pExtData)
{
	/*!
	HYH 2020.02.13
	一般就是调用这个函数来调用基类的sendmsg，将数据发给主站。
	注意，nExtDataSize是被塞进数据头里，而pExtData会被作为额外的数据塞进包尾，所以主站解析时先解析数据头，得到nExtDataSize、nStationID、nMsgType
	等这些数据，后解析pExtData。所以开发者如果不想翻看中转进程的内容，那么你可以理解为pExtData被直接送到界面去，至于是什么格式就要看你自己定义的命令，
	或者说统一一种格式，比如json这种
	*/

	boost::unique_lock<boost::mutex> lock(m_mutex);

	int nDataSize = /*sizeof(NetDataHeader) + dataHeader.*/nExtDataSize;
	if (nDataSize > m_nCurSendBufferSize)
	{
		m_pSendDataBuffer = (unsigned char *)realloc(m_pSendDataBuffer, nDataSize * sizeof(unsigned char));
		m_nCurSendBufferSize = nDataSize;
	}
	//memcpy(m_pSendDataBuffer, &dataHeader, sizeof(NetDataHeader)); 
	memcpy(m_pSendDataBuffer/* + sizeof(NetDataHeader)*/, pExtData, /*dataHeader.*/nExtDataSize);

	CKxAsioTcpClient::SendMsg(/*nStationID, */nMsgType, 0, NULL, 0, (const char *)m_pSendDataBuffer, nDataSize);
}

void CTestAsioTcpClient::RecMsgHandshake()
{
	if (Net::IsExistNetObj())
	{
		string szNet;
		szNet = "handshake";
		Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_HANDSHAKE_SEND), int(szNet.size()), szNet.c_str());
	}
}

void CTestAsioTcpClient::RecMsgToStartCheck(const unsigned char* pExtData)
{
	//停止检测
	Config::g_GetParameter().m_bIsBuildModelStatus = false;

	Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
	g_Environment.StopCheck();
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("停止检测"));
	//载入参数

	char* pt = (char*)pExtData;
	int nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pFileName = new char[nDirLen + 1];
	memset(pFileName, 0, sizeof(char)*nDirLen);  //参数路径
	IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
	pFileName[nDirLen] = '\0';

	CKxBaseFunction hFun;

	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("参数路径：%s"), pFileName);
	//int nStatus = g_Environment.LoadAllObj(pFileName);
	int nStatus = g_Environment.LoadAllObjByXml(pFileName);
	if (!nStatus)
	{
		//kxPrintf(KX_Err, "参数载入失败，无法开始正常检查");
		char szInfo[1024];
		sprintf_s(szInfo, 1024, Config::g_GetParameter().g_TranslatorChinese("参数载入失败，无法开始正常检查"));
		std::ostringstream os;
		os.write(reinterpret_cast<const char *>(&nStatus), sizeof(int));
		os.write(reinterpret_cast<const char *>(&szInfo), sizeof(szInfo));
		std::string str = os.str();
		if (Net::IsExistNetObj())
		{
			Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_START_CHECK_IS_READY), int(str.size()), str.c_str());
		}
		return; //参数加载失败，不检查
	}

	Graber::g_GetGraberBuffer().Init(true);
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("载入模板完成"));
	
	Graber::g_GetCamera()->ReverseScanDirection(0);// 代表相机正着走触发
	g_Grabstatus.init();
	g_Environment.StartCheck();
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("开始检测"));
	std::ostringstream os;
	nStatus = 1;
	os.write(reinterpret_cast<const char *>(&nStatus), sizeof(int));
	std::string str = os.str();
	if (Net::IsExistNetObj())
	{
		Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_START_CHECK_IS_READY), int(str.size()), str.c_str());
	}
	delete[]pFileName;

}

void CTestAsioTcpClient::RecMsgToStopCheck()
{
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("停止检测"));
	g_Environment.StopCheck();
	Check::g_GetCheckCardObj().ClearIndex();
}

void CTestAsioTcpClient::RecMsgToStartSimulate(const unsigned char* pExtData)
{
	Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
	//停止检测
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("停止检测"));
	g_Environment.StopCheck();
	char* pt = (char*)pExtData;

	int nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pFileName = new char[nDirLen + 1];
	memset(pFileName, 0, sizeof(char)*nDirLen);  //参数路径
	IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
	pFileName[nDirLen] = '\0';

	pt += sizeof(char)*nDirLen;
	nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pStimulateDir = new char[nDirLen + 1];
	memset(pStimulateDir, 0, sizeof(char)*nDirLen);  //参数路径
	status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pStimulateDir, nDirLen);
	pStimulateDir[nDirLen] = '\0';

	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("参数路径：%s"), pFileName);

	//int nStatus = g_Environment.LoadAllObj(pFileName);
	int nStatus = g_Environment.LoadAllObjByXml(pFileName);
	if (!nStatus)
	{
		//kxPrintf(KX_Err, "参数载入失败，无法开始正常检查");
		std::ostringstream os;
		os.write(reinterpret_cast<const char *>(&nStatus), sizeof(int));
		std::string str = os.str();
		if (Net::IsExistNetObj())
		{
			Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_START_CHECK_IS_READY), int(str.size()), str.c_str());
		}
		return; //参数加载失败，不检查
	}

	Graber::g_GetGraberBuffer().Init(false);
	

	if (_access(pStimulateDir, 0) != 0)
	{
		kxPrintf(KX_WARNING, Config::g_GetParameter().g_TranslatorChinese("开始模拟检查路径不存在,路径为：%s"), pStimulateDir);
		return;
	}
	else
		kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("开始模拟检查_路径为：%s"), pStimulateDir);
	
	g_Grabstatus.init();
	g_Environment.StartSimulation(pStimulateDir);


	char szInfo[1024];
	sprintf_s(szInfo, 1024, Config::g_GetParameter().g_TranslatorChinese("参数载入成功，可以正常检查"));
	std::ostringstream os;
	nStatus = 1;
	os.write(reinterpret_cast<const char *>(&nStatus), sizeof(int));
	os.write(reinterpret_cast<const char *>(&szInfo), sizeof(szInfo));
	std::string str = os.str();
	if (Net::IsExistNetObj())
	{
		Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_START_CHECK_IS_READY), int(str.size()), str.c_str());
	}
	delete[] pFileName;
	delete[] pStimulateDir;
}

void CTestAsioTcpClient::RecMsgToOpenCamera(const unsigned char* pExtData)
{
	unsigned char* pt = (unsigned char*)pExtData;
	int nUIid = (int)(*((int*)pt));  //界面id
	Config::g_GetParameter().m_nCurrentUIid = nUIid;
	pt += sizeof(int);
	int nExpoureTime = (int)(*((int*)pt));   //曝光时间
	//pt += sizeof(int);
	//int nbestvalue = (int)(*((int*)pt));//最佳清晰度值
	//pt += sizeof(int);
	//int nbestrange = (int)(*((int*)pt));//最佳精调范围
	Config::GetGlobalParam().m_nExpoureTime = nExpoureTime;
	Config::g_GetParameter().m_bChangeExpoureTimeStatus = true;
	Config::g_GetParameter().m_nSendImageCount = 0;
	Graber::g_GetGraberBuffer().Init(true);
	//开始采集
	Graber::g_GetGrabPack().Stop();
	//给相机触发
	//Graber::g_GetCamera()->OpenInternalTrigger(0);
	Graber::g_GetGrabPack().Start();
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("开始内触发采集"));

}

void CTestAsioTcpClient::RecMsgToCloseCamera()
{
	Graber::g_GetGrabPack().Stop();
	Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
	Graber::g_GetCamera()->OpenInternalTrigger(1);
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("停止采集"));
}

void CTestAsioTcpClient::RecMsgToSaveAllImg(const unsigned char* pExtData)
{
	char* pt = (char*)pExtData;
	int nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pFileName = new char[nDirLen + 1];
	memset(pFileName, 0, sizeof(char)*nDirLen);  //参数路径
	IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
	pFileName[nDirLen] = '\0';
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("存图路径：%s"), pFileName);
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("开始存图"));
	Check::g_GetCheckCardObj().SetSaveStatus(Check::g_GetCheckCardObj()._SAVEALL, pFileName);
	delete[] pFileName;
}

void CTestAsioTcpClient::RecMsgToSaveBadImg(const unsigned char* pExtData)
{
	char* pt = (char*)pExtData;
	int nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pFileName = new char[nDirLen + 1];
	memset(pFileName, 0, sizeof(char)*nDirLen);  //参数路径
	IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
	pFileName[nDirLen] = '\0';
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("存坏图路径：%s"), pFileName);
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("开始存图"));
	Check::g_GetCheckCardObj().SetSaveStatus(Check::g_GetCheckCardObj()._SAVEBAD, pFileName);

	delete[] pFileName;
}

void CTestAsioTcpClient::RecMsgToChangeCameraParam(const unsigned char* pExtData)
{
	unsigned char* pt = (unsigned char*)pExtData;
	int nUIid = (int)(*((int*)pt));  //界面id
	Config::g_GetParameter().m_nCurrentUIid = nUIid;
	pt += sizeof(int);
	int nExpoureTime = (int)(*((int*)pt));   //曝光时间
	printf("设置曝光时间为：%d\n", nExpoureTime);
	Config::GetGlobalParam().m_nExpoureTime = nExpoureTime;
}

void CTestAsioTcpClient::RecMsgToOpenCamera_BuildModel(const unsigned char* pExtData)
{
	Graber::g_GetGraberBuffer().Init(true);
	//开始采集
	Graber::g_GetGrabPack().Stop();
	//给相机触发
	Graber::g_GetCamera()->OpenInternalTrigger(1);//外触发
	Graber::g_GetGrabPack().Start();
	Config::g_GetParameter().m_bIsBuildModelStatus = true;
}

void CTestAsioTcpClient::RecMsgToCloseCamera_BuildModel(const unsigned char* pExtData)
{
	Config::g_GetParameter().m_bIsBuildModelStatus = false;
	RecMsgToCloseCamera();
}

void CTestAsioTcpClient::RecMsgToChangeCameraCaptureDirection(int nstatus)
{
	printf_s("改变相机采集方向");
	if (nstatus > 0)
	{
		Graber::g_GetCamera()->ReverseScanDirection(1);
	}
	else
	{
		Graber::g_GetCamera()->ReverseScanDirection(0);
	}
}

void CTestAsioTcpClient::RecMsgJustOpenCamera()
{
	//Graber::g_GetGraberBuffer().Init(true);
	//开始采集
	Graber::g_GetGrabPack().Stop();
	//给相机触发
	Graber::g_GetCamera()->OpenInternalTrigger(1);//外触发
	Graber::g_GetGrabPack().Start();
}

void CTestAsioTcpClient::RecMsgPackId(const unsigned char* pExtData)
{
	// 收到pack id，交给检测，检测收到packid 会对现有结果进行清零，重新开始新的id


	Json::Reader reader;
	Json::Value root;
	if (reader.parse((const char*)pExtData, root))  // reader将Json字符串解析到root，root将包含Json里所有子元素  
	{
		std::string packid = root["packid"].asString();  //

		Check::g_GetCheckCardObj().SetPackID(packid);
	}
}

CTestAsioTcpClient* g_client ;
