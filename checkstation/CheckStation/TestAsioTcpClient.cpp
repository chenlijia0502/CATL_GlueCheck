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
	// ��ʼ�������̲߳�������Start���̺߳�����this�Ǵ���Ĳ���
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
	// HYH 2020.02.13 ���ÿ��ˣ���һ�㲻���õ����
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
	// HYH 2020.02.13 ���ÿ��ˣ���һ�㲻���õ����
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
	// �ر����
	std::cout << "��⵽��վ�����쳣���ر����" << std::endl;
	Graber::g_GetGrabPack().Stop();
	Graber::g_GetCamera()->Close();

}


void CTestAsioTcpClient::OnRecvData(int nStationID, const unsigned char* pData, size_t nDataSize)
{
	std::cout << "OnRecvData:" << pData  << " size = " << nDataSize << "bytes" << std::endl;
	

	// �ж������Ƿ���Ч��pData�����ݰ�ͷ������������sizeof(NetDataHeader)��ô��
	if (pData && nDataSize >= sizeof(NetDataHeader)) 
	{
		NetDataHeader dataHeader; 
		memcpy(&dataHeader, pData, sizeof(NetDataHeader)); 

		// ����ͷ��������Ƿ���Ч��nDataSize�ĳ���Ϊsizeof(NetDataHeader) + �������ݵĳ���
		if (dataHeader.nExtDataSize == nDataSize - sizeof(NetDataHeader)) 
		{
			// pExtDataΪ�������ݵ�ָ��
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
					Graber::g_GetCamera()->Close();// ��վ�رգ���վ�����ͷ����
					break;
				case MSG_PACK_ID://�յ�pack id
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
	//����A����
}

void CTestAsioTcpClient::SendMsg(int nStationID, int nMsgType, int nExtDataSize, const char* pExtData)
{
	/*!
	HYH 2020.02.13
	һ����ǵ���������������û����sendmsg�������ݷ�����վ��
	ע�⣬nExtDataSize�Ǳ���������ͷ���pExtData�ᱻ��Ϊ���������������β��������վ����ʱ�Ƚ�������ͷ���õ�nExtDataSize��nStationID��nMsgType
	����Щ���ݣ������pExtData�����Կ�����������뷭����ת���̵����ݣ���ô��������ΪpExtData��ֱ���͵�����ȥ��������ʲô��ʽ��Ҫ�����Լ���������
	����˵ͳһһ�ָ�ʽ������json����
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
	//ֹͣ���
	Config::g_GetParameter().m_bIsBuildModelStatus = false;

	Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
	g_Environment.StopCheck();
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("ֹͣ���"));
	//�������

	char* pt = (char*)pExtData;
	int nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pFileName = new char[nDirLen + 1];
	memset(pFileName, 0, sizeof(char)*nDirLen);  //����·��
	IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
	pFileName[nDirLen] = '\0';

	CKxBaseFunction hFun;

	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("����·����%s"), pFileName);
	//int nStatus = g_Environment.LoadAllObj(pFileName);
	int nStatus = g_Environment.LoadAllObjByXml(pFileName);
	if (!nStatus)
	{
		//kxPrintf(KX_Err, "��������ʧ�ܣ��޷���ʼ�������");
		char szInfo[1024];
		sprintf_s(szInfo, 1024, Config::g_GetParameter().g_TranslatorChinese("��������ʧ�ܣ��޷���ʼ�������"));
		std::ostringstream os;
		os.write(reinterpret_cast<const char *>(&nStatus), sizeof(int));
		os.write(reinterpret_cast<const char *>(&szInfo), sizeof(szInfo));
		std::string str = os.str();
		if (Net::IsExistNetObj())
		{
			Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_START_CHECK_IS_READY), int(str.size()), str.c_str());
		}
		return; //��������ʧ�ܣ������
	}

	Graber::g_GetGraberBuffer().Init(true);
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("����ģ�����"));
	
	Graber::g_GetCamera()->ReverseScanDirection(0);// ������������ߴ���
	g_Grabstatus.init();
	g_Environment.StartCheck();
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("��ʼ���"));
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
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("ֹͣ���"));
	g_Environment.StopCheck();
	Check::g_GetCheckCardObj().ClearIndex();
}

void CTestAsioTcpClient::RecMsgToStartSimulate(const unsigned char* pExtData)
{
	Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
	//ֹͣ���
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("ֹͣ���"));
	g_Environment.StopCheck();
	char* pt = (char*)pExtData;

	int nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pFileName = new char[nDirLen + 1];
	memset(pFileName, 0, sizeof(char)*nDirLen);  //����·��
	IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
	pFileName[nDirLen] = '\0';

	pt += sizeof(char)*nDirLen;
	nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pStimulateDir = new char[nDirLen + 1];
	memset(pStimulateDir, 0, sizeof(char)*nDirLen);  //����·��
	status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pStimulateDir, nDirLen);
	pStimulateDir[nDirLen] = '\0';

	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("����·����%s"), pFileName);

	//int nStatus = g_Environment.LoadAllObj(pFileName);
	int nStatus = g_Environment.LoadAllObjByXml(pFileName);
	if (!nStatus)
	{
		//kxPrintf(KX_Err, "��������ʧ�ܣ��޷���ʼ�������");
		std::ostringstream os;
		os.write(reinterpret_cast<const char *>(&nStatus), sizeof(int));
		std::string str = os.str();
		if (Net::IsExistNetObj())
		{
			Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_START_CHECK_IS_READY), int(str.size()), str.c_str());
		}
		return; //��������ʧ�ܣ������
	}

	Graber::g_GetGraberBuffer().Init(false);
	

	if (_access(pStimulateDir, 0) != 0)
	{
		kxPrintf(KX_WARNING, Config::g_GetParameter().g_TranslatorChinese("��ʼģ����·��������,·��Ϊ��%s"), pStimulateDir);
		return;
	}
	else
		kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("��ʼģ����_·��Ϊ��%s"), pStimulateDir);
	
	g_Grabstatus.init();
	g_Environment.StartSimulation(pStimulateDir);


	char szInfo[1024];
	sprintf_s(szInfo, 1024, Config::g_GetParameter().g_TranslatorChinese("��������ɹ��������������"));
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
	int nUIid = (int)(*((int*)pt));  //����id
	Config::g_GetParameter().m_nCurrentUIid = nUIid;
	pt += sizeof(int);
	int nExpoureTime = (int)(*((int*)pt));   //�ع�ʱ��
	//pt += sizeof(int);
	//int nbestvalue = (int)(*((int*)pt));//���������ֵ
	//pt += sizeof(int);
	//int nbestrange = (int)(*((int*)pt));//��Ѿ�����Χ
	Config::GetGlobalParam().m_nExpoureTime = nExpoureTime;
	Config::g_GetParameter().m_bChangeExpoureTimeStatus = true;
	Config::g_GetParameter().m_nSendImageCount = 0;
	Graber::g_GetGraberBuffer().Init(true);
	//��ʼ�ɼ�
	Graber::g_GetGrabPack().Stop();
	//���������
	//Graber::g_GetCamera()->OpenInternalTrigger(0);
	Graber::g_GetGrabPack().Start();
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("��ʼ�ڴ����ɼ�"));

}

void CTestAsioTcpClient::RecMsgToCloseCamera()
{
	Graber::g_GetGrabPack().Stop();
	Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
	Graber::g_GetCamera()->OpenInternalTrigger(1);
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("ֹͣ�ɼ�"));
}

void CTestAsioTcpClient::RecMsgToSaveAllImg(const unsigned char* pExtData)
{
	char* pt = (char*)pExtData;
	int nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pFileName = new char[nDirLen + 1];
	memset(pFileName, 0, sizeof(char)*nDirLen);  //����·��
	IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
	pFileName[nDirLen] = '\0';
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("��ͼ·����%s"), pFileName);
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("��ʼ��ͼ"));
	Check::g_GetCheckCardObj().SetSaveStatus(Check::g_GetCheckCardObj()._SAVEALL, pFileName);
	delete[] pFileName;
}

void CTestAsioTcpClient::RecMsgToSaveBadImg(const unsigned char* pExtData)
{
	char* pt = (char*)pExtData;
	int nDirLen = (int)(*((int*)pt));
	pt += sizeof(int);
	char* pFileName = new char[nDirLen + 1];
	memset(pFileName, 0, sizeof(char)*nDirLen);  //����·��
	IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
	pFileName[nDirLen] = '\0';
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("�滵ͼ·����%s"), pFileName);
	kxPrintf(KX_INFO, Config::g_GetParameter().g_TranslatorChinese("��ʼ��ͼ"));
	Check::g_GetCheckCardObj().SetSaveStatus(Check::g_GetCheckCardObj()._SAVEBAD, pFileName);

	delete[] pFileName;
}

void CTestAsioTcpClient::RecMsgToChangeCameraParam(const unsigned char* pExtData)
{
	unsigned char* pt = (unsigned char*)pExtData;
	int nUIid = (int)(*((int*)pt));  //����id
	Config::g_GetParameter().m_nCurrentUIid = nUIid;
	pt += sizeof(int);
	int nExpoureTime = (int)(*((int*)pt));   //�ع�ʱ��
	printf("�����ع�ʱ��Ϊ��%d\n", nExpoureTime);
	Config::GetGlobalParam().m_nExpoureTime = nExpoureTime;
}

void CTestAsioTcpClient::RecMsgToOpenCamera_BuildModel(const unsigned char* pExtData)
{
	Graber::g_GetGraberBuffer().Init(true);
	//��ʼ�ɼ�
	Graber::g_GetGrabPack().Stop();
	//���������
	Graber::g_GetCamera()->OpenInternalTrigger(1);//�ⴥ��
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
	printf_s("�ı�����ɼ�����");
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
	//��ʼ�ɼ�
	Graber::g_GetGrabPack().Stop();
	//���������
	Graber::g_GetCamera()->OpenInternalTrigger(1);//�ⴥ��
	Graber::g_GetGrabPack().Start();
}

void CTestAsioTcpClient::RecMsgPackId(const unsigned char* pExtData)
{
	// �յ�pack id��������⣬����յ�packid ������н���������㣬���¿�ʼ�µ�id


	Json::Reader reader;
	Json::Value root;
	if (reader.parse((const char*)pExtData, root))  // reader��Json�ַ���������root��root������Json��������Ԫ��  
	{
		std::string packid = root["packid"].asString();  //

		Check::g_GetCheckCardObj().SetPackID(packid);
	}
}

CTestAsioTcpClient* g_client ;
