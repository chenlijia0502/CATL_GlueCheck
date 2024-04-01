#include "stdafx.h"
#include "TestAsioTcpServer.h"
#include "CkxThreadManage.h"

#include "CkxEnvironment.h"

#include "KxRotateCards.h"
#include "kxCheckResult.h"
#include "Grab_Buffer.h"
#include "CkxFileRead.h"
#include "kxParameter.h"
#include "global.h"
#include "KxCard.h"
#include "GrabPack.h"
//#include "kxPrintf.h"
#include "KxGeneralFun.h"
using namespace Check;

CTestAsioTcpServer::CTestAsioTcpServer(int nListenPort)
{
	/*!
	\brief 先检查端口号有没有被占用，IsPortBind返回的是占用端口的进程号
	*/
	int nPid;
	if ((nPid = KxFun::IsPortBind(nListenPort)) > 0)
	{
		fprintf(stderr, "Warning: Process %d Has Used Port %d!\n", nPid, nListenPort);
		fprintf(stderr, "Try To Kill Process %d\n", nPid);

		char szResult[128] = { 0 };
		if (KxFun::KillProcess(nPid, szResult, sizeof(szResult)))
		{
			fprintf(stderr, "%s\n", szResult);
		}
		else
		{
			fprintf(stderr, "Kill Process Failed\n");
		}
	}
	try
	{
		m_pAsioTcp = boost::make_shared<CAsioTcpServer>(this, nListenPort);
		// 初始化网络线程并启动，Start是线程函数，this是传入的参数
		m_pRunThread = boost::shared_ptr<boost::thread>(new boost::thread(Start, this));
		m_pSendDataBuffer = (unsigned char *)calloc(sizeof(DataHeader), sizeof(unsigned char));
		m_nCurSendBufferSize = sizeof(DataHeader);
	}
	catch (boost::system::system_error &ec)
	{
		fprintf(stderr, "Create Acceptor Fail, %s\n", ec.what());
	}
}

CTestAsioTcpServer::~CTestAsioTcpServer()
{
	m_pAsioTcp->Stop();
	m_pRunThread->join();
	free(m_pSendDataBuffer);
}

void CTestAsioTcpServer::OnNewConnection(socket_handle newSocket)
{
	CTcpSession *pSession = *reinterpret_cast<CTcpSession **>(newSocket);
	kxPrintf(KX_INFO, "启动网络连接");
}

void CTestAsioTcpServer::OnRecvData(socket_handle socket, const unsigned char* pData, size_t nDataSize)
{
	kxPrintf(KX_DEBUG, "收到网络数据包：大小 = %d bytes", nDataSize);

	// 判断数据是否有效，pData带数据包头，所以至少有sizeof(DataHeader)这么大
	if (pData && nDataSize >= sizeof(DataHeader))
	{
		DataHeader dataHeader;
		memcpy(&dataHeader, pData, sizeof(DataHeader));

		// 检查包头里的数据是否有效，nDataSize的长度为sizeof(DataHeader) + 超长数据的长度
		if (dataHeader.nExtDataSize == nDataSize - sizeof(DataHeader))
		{
			// pExtData为超长数据的指针
			const unsigned char *pExtData = pData + sizeof(DataHeader);
			if (dataHeader.nExtDataSize == 0)
				pExtData = NULL;
			//// 可以判断消息类型来调用相应的处理函数了，在这里添加
			if (dataHeader.nMsgType == int(MSG_SEARCH_EDGE_TEST))
			{

				unsigned char* pt = (unsigned char*)pExtData;
				Check::g_GetCheckCardObj().GetAutoSearchCardObj().Read(pt);
				kxCImageBuf ImgTemp;
				//CKxBaseFunction hBaseFun;
				m_hBaseFun.readImgBufFromMemory(ImgTemp, pt);
				Check::g_GetCheckCardObj().GetAutoSearchCardObj().SetWarpSize(Config::g_GetParameter().m_nStandardWidth, Config::g_GetParameter().m_nStandardHeight);

				bool bStatus = Check::g_GetCheckCardObj().GetAutoSearchCardObj().Check(ImgTemp, true);

				const char* send[3];
				int   sendLen[3];
				CKxSearchCards::Result resTemp;
				resTemp = Check::g_GetCheckCardObj().GetAutoSearchCardObj().GetResult();
				send[0] = (char*)&resTemp;
				sendLen[0] = sizeof(CKxSearchCards::Result);
				kxCImageBuf  TmpBuf;
				if (!bStatus)  //搜卡错误发黑图
				{
					TmpBuf.Init(Config::g_GetParameter().m_nStandardWidth, Config::g_GetParameter().m_nStandardHeight, ImgTemp.nChannel);
					memset(TmpBuf.buf, 0, TmpBuf.nPitch * TmpBuf.nHeight);
				}
				else
				{
					TmpBuf.buf = Check::g_GetCheckCardObj().GetAutoSearchCardObj().GetModelImg(TmpBuf.nWidth, TmpBuf.nHeight, TmpBuf.nPitch, TmpBuf.nChannel);
				}
				kxCImageBuf CopyImg;
				CopyImg.Init(TmpBuf.nWidth, TmpBuf.nHeight, TmpBuf.nChannel);
				if (Config::g_GetParameter().m_nIsperiodicCheck)  //凹坑预处理
				{
					m_hBaseFun.KxFFtCheckPeriodic(TmpBuf, CopyImg);
				}
				else
				{
					kxRect<int> rc;
					rc.setup(0, 0, TmpBuf.nWidth - 1, TmpBuf.nHeight - 1);
					m_hBaseFun.KxCopyImage(TmpBuf, CopyImg, rc);
				}

				//m_hBaseFun.SaveBMPImage_h("d:\\Search.bmp", CopyImg);

				string str = m_hBaseFun.FormatImageToString(CopyImg);
				send[1] = str.c_str();
				sendLen[1] = int(str.size());

				string str1 = m_hBaseFun.FormatIntToString(Config::g_GetParameter().m_nBeltStartX) + m_hBaseFun.FormatIntToString(Config::g_GetParameter().m_nBeltEndX);
				send[2] = str1.c_str();
				sendLen[2] = int(str1.size());

				SendMsg(Config::GetGlobelParam().m_nStationID, dataHeader.nMsgType, send, sendLen, 3);


			}

			if (dataHeader.nMsgType == int(MSG_START_CHECK))
			{
				//停止检测
				//kxPrintf(KX_INFO, "停止检测");
				Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
				g_Environment.StopCheck();
				//载入参数
				char* pFileName = (char*)pExtData;
				kxPrintf(KX_DEBUG, "参数路径：%s", pFileName);
				Config::g_GetParameter().m_szModelSaveDir = pFileName;

				int nStatus = g_Environment.LoadAllObj(pFileName);
				if (!nStatus)
				{
					kxPrintf(KX_ERR, "参数载入失败，无法开始正常检查");
					return; //参数加载失败，不检查
				}

				Graber::g_GetGraberBuffer().Init(true);
				kxPrintf(KX_DEBUG, "载入模板完成");
				
				Config::SetSaveImageStatus(false);
				Check::g_GetCheckCardObj().ClearCardStatus();
				g_Environment.StartCheck();
				kxPrintf(KX_INFO, "开始检测");
				//默认开启软件踢坏卡模式
				Graber::g_GetCamera()->OpenSoftwareControl();
			}
			if (dataHeader.nMsgType == int(MSG_STOP_CHECK))
			{
				kxPrintf(KX_INFO, "停止检测");
				g_Environment.StopCheck();
				Config::g_GetParameter().m_bSaveImg = false;
				Config::g_GetParameter().m_nVariableJugeFlag = 0;
				Graber::g_GetCamera()->CloseSoftwareControl();
				//Graber::g_GetCamera()->SetActiveTrigger();

			}
			if (dataHeader.nMsgType == int(MSG_SAVE_IMAGE))
			{
				char* pFileName = (char*)pExtData;
				kxPrintf(KX_DEBUG, "存图路径：%s", pFileName);
				strcpy_s(Config::g_GetParameter().m_szSavePath, pFileName);

				kxPrintf(KX_INFO, "开始存图");
				Config::g_GetParameter().m_nSaveTotalCounts = 1000;
				Config::g_GetParameter().m_nSaveCount = 0;
				Config::g_GetParameter().m_bSaveImg = true;
			}
			if (dataHeader.nMsgType == int(MSG_ONE_IMG))
			{
				const char* pFileName = (char*)pExtData;

				Config::SetSaveImageStatus(true);

				Graber::g_GetFileRead().readOnePic(pFileName);
				Thread::g_GetThreadManage().startAllThread();
			}
			if (dataHeader.nMsgType == int(MSG_ALL_IMG))
			{
				//停止检测
				kxPrintf(KX_INFO, "停止检测");
				g_Environment.StopCheck();
				//载入参数

				char* pFileName = (char*)pExtData;
				kxPrintf(KX_INFO, "参数路径：%s", pFileName);

				Config::g_GetParameter().m_szModelSaveDir = pFileName;

				int nStatus = g_Environment.LoadAllObj(pFileName);
				if (!nStatus)
				{
					kxPrintf(KX_ERR, "参数载入失败，无法开始正常检查");
					return; //参数加载失败，不检查
				}

				g_Environment.InitAllObj(0);
				Graber::g_GetGraberBuffer().Init(false);
				//kxPrintf(KX_INFO, "初始化完成");
		
				//开始模拟检测		
				Config::SetSaveImageStatus(false);
				const char* pFileName1 = (char*)pExtData;
				pFileName1 += strlen(pFileName1) + 1;

				if (_access(pFileName1, 0) != 0)
					kxPrintf(KX_ERR, "开始模拟检查路径不存在,路径为：%s", pFileName1);
				else
				{
					kxPrintf(KX_INFO, "开始模拟检查_路径为：%s", pFileName1);
					g_Environment.StartSimulation(pFileName1);
				}


			}

			if (dataHeader.nMsgType == MSG_SET_CUR_MODEL)
			{
				g_Environment.ClearAllObj();
				char* pFileName = (char*)pExtData;
				kxPrintf(KX_DEBUG, "参数路径：%s", pFileName);

				Config::g_GetParameter().m_szModelSaveDir = pFileName;
				//string sz;

				//for (int i = 0; i < strlen(pFileName); i++)
				//{
				//	if ((pFileName[i] != '\\') || (i < 2))
				//		sz += pFileName[i];
				//	else
				//		break;
				//}
				//Config::g_GetParameter().m_szSaveBmpDir = sz;

				g_Environment.LoadAllObj(pFileName);
				g_Environment.InitAllObj(0);
				kxPrintf(KX_DEBUG, "载入模板完成");
			}

			if (dataHeader.nMsgType == int(MSG_GET_MASK_IMG))
			{
				unsigned char* pt = (unsigned char*)pExtData;
				int nAreaId = (int)(*((int*)pt));
				pt += sizeof(int);
				CkxCheckTool hCheckTool;
				hCheckTool.ReadParaFromNet(pt);
				kxCImageBuf ImageMask;
				hCheckTool.m_hSurfaceCheck.GetImageMaskImage(ImageMask);
				int nNumberId = hCheckTool.m_hSurfaceCheck.GetParameter().m_nAreaNumber;
				CKxBaseFunction hFun;
				string str = hFun.FormatIntToString(nNumberId) + hFun.FormatImageToString(ImageMask);
				SendMsg(Config::GetGlobelParam().m_nStationID, MSG_MASK_PARAMS, int(str.size()), str.c_str());
			}

			if (dataHeader.nMsgType == MSG_GET_BLOB_INFO)
			{

			}

			if (dataHeader.nMsgType == MSG_SET_RESIIDUE)
			{
				kxPrintf(KX_INFO, "调试偏移值");
				unsigned char* pt = (unsigned char*)pExtData;
				int nAreaId = (int)(*((int*)pt));            //区域号
				pt += sizeof(int);
				CKxBaseFunction  hFun;
				kxCImageBuf  ImgTemp;
				hFun.readImgBufFromMemory(ImgTemp, pt);     //原始图像

				int nOffset[5];                             //对应5个偏移值
				memcpy(nOffset, pt, sizeof(int)* 5);
				pt += sizeof(int)*5;

				CkxCheckTool* m_hCheckObj = new CkxCheckTool;

				int nDirLen = (int)(*((int*)pt));
				pt += sizeof(int);
				char* pFileName = new char[nDirLen + 1];
				memset(pFileName, 0, sizeof(char)*nDirLen);  //参数路径
				{
					IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
					kxIppLog(status);
				}
				pFileName[nDirLen] = '\0';
				int nStatus = g_Environment.LoadOneObj(pFileName, nAreaId, m_hCheckObj);
				delete[]pFileName;
				string str;
				int nOpenCheckStatus = m_hCheckObj->m_nOpenCheckStatus;
				int nNumberId = nAreaId;
				
				if (nStatus < 0)
				{
					nStatus = -2;
					str = hFun.FormatIntToString(nNumberId) + hFun.FormatIntToString(nStatus);
					SendMsg(Config::GetGlobelParam().m_nStationID, MSG_GET_RESIIDUE, int(str.size()), str.c_str());
					char sz[128];
					sprintf_s(sz, 128, "参数载入出错_%d", nStatus);
					kxPrintf(KX_INFO, sz);
				}
				
				int nOff[6];
				for (int i = 0; i < 5; i++)
			    {
					nOff[i] = nOffset[i];
			    }
				nOff[5] = 10;

				kxCImageBuf DstImg;
				KxCallStatus hCall;
				nStatus = m_hCheckObj->ShowResidualsImage(ImgTemp, DstImg, nOff, hCall);
				if (nStatus == 0)
				{
					char sz[128];
					sprintf_s(sz, 128, "彩色残差图合并出错_%s", hCall.szErrInfo);
					kxPrintf(KX_INFO, sz);
				}
				
	            //获取Blob分析结果
				enum
				{
					_BLOB_FEATURES = 24,//每一个Blob特征数
					_MAX_BLOB = 8,		//总的Blob数
				};
				int nBlobInfo[_MAX_BLOB*_BLOB_FEATURES + 1];
				memset(nBlobInfo, 0, sizeof(int)*(_MAX_BLOB*_BLOB_FEATURES + 1));
				if (2 == nOpenCheckStatus)
				{
					nBlobInfo[0] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_nCount;
				}
				else if (4 == nOpenCheckStatus)
				{
					nBlobInfo[0] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_nCount;
				}
				else if (5 == nOpenCheckStatus)
				{
					nBlobInfo[0] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_nCount;
				}
				else if (6 == nOpenCheckStatus)
				{
					nBlobInfo[0] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_nCount;
				}
				else
				{
					nBlobInfo[0] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_nCount;
				}

				
				for (int i = 0; i < nBlobInfo[0]; i++)
				{
					if (1 == nOpenCheckStatus)
					{
						nBlobInfo[1 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nDots;
						nBlobInfo[2 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nEnergy;
						nBlobInfo[3 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nLeft;
						nBlobInfo[4 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nTop;
						nBlobInfo[5 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobWidth;
						nBlobInfo[6 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobHeight;

						nBlobInfo[7 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nWHRatio;
						nBlobInfo[8 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgEnergy;
						nBlobInfo[9 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgArea;

						nBlobInfo[10 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectWidth;
						nBlobInfo[11 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectHeight;


					}
					if (2 == nOpenCheckStatus)
					{
						nBlobInfo[1 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_hBlobInfo[i].m_nDots;
						nBlobInfo[2 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_hBlobInfo[i].m_nEnergy;
						nBlobInfo[3 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_hBlobInfo[i].m_nLeft;
						nBlobInfo[4 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_hBlobInfo[i].m_nTop;
						nBlobInfo[5 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobWidth;
						nBlobInfo[6 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobHeight;

						nBlobInfo[7 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_hBlobInfo[i].m_nWHRatio;
						nBlobInfo[8 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgEnergy;
						nBlobInfo[9 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSignatureCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgArea;

						nBlobInfo[10 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectWidth;
						nBlobInfo[11 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectHeight;


					}
					if (4 == nOpenCheckStatus)
					{
						nBlobInfo[1 + _BLOB_FEATURES*i] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_hBlobInfo[i].m_nDots;
						nBlobInfo[2 + _BLOB_FEATURES*i] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_hBlobInfo[i].m_nEnergy;
						nBlobInfo[3 + _BLOB_FEATURES*i] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_hBlobInfo[i].m_nLeft;
						nBlobInfo[4 + _BLOB_FEATURES*i] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_hBlobInfo[i].m_nTop;
						nBlobInfo[5 + _BLOB_FEATURES*i] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobWidth;
						nBlobInfo[6 + _BLOB_FEATURES*i] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobHeight;

						nBlobInfo[7 + _BLOB_FEATURES*i] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_hBlobInfo[i].m_nWHRatio;
						nBlobInfo[8 + _BLOB_FEATURES*i] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgEnergy;
						nBlobInfo[9 + _BLOB_FEATURES*i] = m_hCheckObj->m_hZhuguangCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgArea;

						nBlobInfo[10 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectWidth;
						nBlobInfo[11 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectHeight;

					}

					if (5 == nOpenCheckStatus)
					{
						nBlobInfo[1 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nDots;
						nBlobInfo[2 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nEnergy;
						nBlobInfo[3 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nLeft;
						nBlobInfo[4 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nTop;
						nBlobInfo[5 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobWidth;
						nBlobInfo[6 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobHeight;
						nBlobInfo[7 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nWHRatio;
						nBlobInfo[8 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgEnergy;
						nBlobInfo[9 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgArea;
						nBlobInfo[10 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectWidth;
						nBlobInfo[11 + _BLOB_FEATURES*i] = m_hCheckObj->m_hSuperCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectHeight;

					}
					if (6 == nOpenCheckStatus)
					{
						nBlobInfo[1 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nDots;
						nBlobInfo[2 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nEnergy;
						nBlobInfo[3 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nLeft;
						nBlobInfo[4 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nTop;
						nBlobInfo[5 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobWidth;
						nBlobInfo[6 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobHeight;
						nBlobInfo[7 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nWHRatio;
						nBlobInfo[8 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgEnergy;
						nBlobInfo[9 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nAvgArea;
						nBlobInfo[10 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectWidth;
						nBlobInfo[11 + _BLOB_FEATURES*i] = m_hCheckObj->m_hQuanxibiaoCheck.GetCheckResult().m_hBlobInfo[i].m_nMinRectHeight;

					}

				}

				str = hFun.FormatIntToString(nNumberId) + hFun.FormatIntToString(nStatus)
					+ hFun.FormatImageToString(DstImg) + FormatIntArrayToString(nBlobInfo, _MAX_BLOB*_BLOB_FEATURES + 1);


				//hFun.SaveBMPImage_h("d:\\DstImg.bmp",DstImg);
				SendMsg(Config::GetGlobelParam().m_nStationID, MSG_GET_RESIIDUE, int(str.size()), str.c_str());

				delete m_hCheckObj;
	
			}

			if (dataHeader.nMsgType == MSG_LEARN_DEFECT)
			{
				kxPrintf(KX_DEBUG, "单个缺陷学习");
				unsigned char* pt = (unsigned char*)pExtData;
				int nAreaId = (int)(*((int*)pt));  //区域号
				pt += sizeof(int);
				CKxBaseFunction  hFun;
				kxCImageBuf  ImgTemp;             //四角拉伸图

				int nOffset = (int)(*((int*)pt));  //区域号
				pt += sizeof(int);
				g_SaveImgQue.ReadImgFromFileByOffset(nOffset, ImgTemp);

				IppiSize roiSize = { ImgTemp.nWidth, ImgTemp.nHeight };
				if (ImgTemp.nChannel == _Type_G24)
				{
					int nDstOder[3] = { 2, 1, 0 };
					ippiSwapChannels_8u_C3IR(ImgTemp.buf, ImgTemp.nPitch, roiSize, nDstOder);
				}

				//hFun.SaveBMPImage_h("d:\\123.bmp", ImgTemp);
				int nBlobIndex;                     //缺陷的标号
				memcpy(&nBlobIndex, pt, sizeof(int)* 1);
				pt += sizeof(int);

				int nDirLen = (int)(*((int*)pt));   //参数路径
				pt += sizeof(int);
				char* pFileName = new char[nDirLen + 1];
				memset(pFileName, 0, sizeof(char)*nDirLen);
				{
					IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
					kxIppLog(status);
				}
				pFileName[nDirLen] = '\0';
				
				int nIndex = 0;
				for (int i = 0; i < _AreaCount; i++)
				{
					if (Config::GetGlobelParam().m_nUsedArea[i])
					{
						if (nAreaId == i)
						{
							break;
						}
						else
							nIndex++;
					}
				}

				CkxCheckTool& m_hCheckObj = (Check::g_GetCheckCardObj().GetCheckTools(nIndex));

				char szModelPath[256];
				sprintf_s(szModelPath, sizeof(szModelPath), "%s\\KxModel%d.dat", pFileName, nAreaId);

				int nStatus;
				string str;
	
				KxCallStatus hCall;
				nStatus = m_hCheckObj.SingleDefectLearn(ImgTemp, nBlobIndex, hCall);
				if (nStatus == 0)
				{
					char sz[128];
					sprintf_s(sz, 128, "单个缺陷学习_%s,缺陷学习失败", hCall.szErrInfo);
					kxPrintf(KX_INFO, sz);
				}
				else
				{
					m_hCheckObj.SaveLearnTemplateImg(szModelPath);
					{
						char sz[128];
						sprintf_s(sz, 128, "区域%d的%d号缺陷局部学习完成_%s", nAreaId, nBlobIndex, hCall.szErrInfo);
						kxPrintf(KX_INFO, sz);
					}
				}

				string  szNet = m_hBaseFun.FormatIntToString(nAreaId);
				g_Server->SendMsg(Config::GetGlobelParam().m_nStationID, MSG_LEARN_DEFECT_COMPLETED, int(szNet.size()), szNet.c_str());

				delete[]pFileName;
			}


			if (dataHeader.nMsgType == MSG_LEARN_ONE)
			{
				kxPrintf(KX_DEBUG, "单个缺陷学习");
				unsigned char* pt = (unsigned char*)pExtData;
				int nAreaId = (int)(*((int*)pt));  //区域号
				pt += sizeof(int);
				CKxBaseFunction  hFun;
				kxCImageBuf  ImgTemp;             //四角拉伸图

				int nOffset = (int)(*((int*)pt));  //偏移值
				pt += sizeof(int);
				g_SaveImgQue.ReadImgFromFileByOffset(nOffset, ImgTemp);

				IppiSize roiSize = { ImgTemp.nWidth, ImgTemp.nHeight };
				if (ImgTemp.nChannel == _Type_G24)
				{
					int nDstOder[3] = { 2, 1, 0 };
					ippiSwapChannels_8u_C3IR(ImgTemp.buf, ImgTemp.nPitch, roiSize, nDstOder);
				}

				//hFun.SaveBMPImage_h("d:\\123.bmp", ImgTemp);
				//int nBlobIndex;                     //缺陷的标号
				//memcpy(&nBlobIndex, pt, sizeof(int)* 1);
				//pt += sizeof(int);

				int nDirLen = (int)(*((int*)pt));   //参数路径
				pt += sizeof(int);
				char* pFileName = new char[nDirLen + 1];
				memset(pFileName, 0, sizeof(char)*nDirLen);
				{
					IppStatus status = ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
					kxIppLog(status);
				}
				pFileName[nDirLen] = '\0';

				int nIndex = 0;
				for (int i = 0; i < _AreaCount; i++)
				{
					if (Config::GetGlobelParam().m_nUsedArea[i])
					{
						if (nAreaId == i)
						{
							break;
						}
						else
							nIndex++;
					}
				}

				CkxCheckTool& m_hCheckObj = (Check::g_GetCheckCardObj().GetCheckTools(nIndex));

				char szModelPath[256];
				sprintf_s(szModelPath, sizeof(szModelPath), "%s\\KxModel%d.dat", pFileName, nAreaId);

				int nStatus;
				string str;

				KxCallStatus hCall;
				nStatus = m_hCheckObj.SingleImageLearn(ImgTemp, hCall);
				if (nStatus == 0)
				{
					char sz[128];
					sprintf_s(sz, 128, "单张学习_%s, 学习失败", hCall.szErrInfo);
					kxPrintf(KX_INFO, sz);
				}
				else
				{
					m_hCheckObj.SaveLearnTemplateImg(szModelPath);
					{
						char sz[128];
						sprintf_s(sz, 128, "区域%d的单张学习完成_%s", nAreaId,  hCall.szErrInfo);
						kxPrintf(KX_INFO, sz);
					}
				}

				string  szNet = m_hBaseFun.FormatIntToString(nAreaId);
				g_Server->SendMsg(Config::GetGlobelParam().m_nStationID, MSG_LEARN_ONE_COMPLETED, int(szNet.size()), szNet.c_str());

				delete[]pFileName;
			}


			//导入号码配置文件
			if (dataHeader.nMsgType == MSG_LOAD_VARIINO_DATA)
			{
				unsigned char* pt = (unsigned char*)pExtData;
				int nAreaId = (int)(*((int*)pt));   //区域号
				pt += sizeof(int);
				int nDirLen = (int)(*((int*)pt));   //区域号
				pt += sizeof(int);

				char* pFileName = new char[nDirLen + 1];
				memset(pFileName, 0, sizeof(char)*nDirLen);
				ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
				pFileName[nDirLen] = '\0';

				g_SaveImgQue.OpenVariableInfoFile(pFileName);
				Config::g_GetParameter().m_nVariableJugeFlag = 1;
				kxPrintf(KX_DEBUG, "号码配置路径 %s", pFileName);
				kxPrintf(KX_INFO, "载入号码配置成功");
			}

			if (dataHeader.nMsgType == MSG_START_CAMERA)
			{
				unsigned char* pt = (unsigned char*)pExtData;
				int nExpoureTime = (int)(*((int*)pt));   //曝光时间
				Config::GetGlobelParam().m_nExpoureTime = nExpoureTime;
				Config::g_GetParameter().m_bChangeExpoureTimeStatus = true;
				Config::g_GetParameter().m_nSendImageCount = 0;
				Graber::g_GetGraberBuffer().Init(true);
				//开始采集
				Graber::g_GetGrabPack().Stop();
				//给相机触发
				Graber::g_GetCamera()->OpenInternalTrigger(0);
				Graber::g_GetGrabPack().Start();
				kxPrintf(KX_INFO, "开始内触发采集");

			}

			if (dataHeader.nMsgType == MSG_STOP_CAMERA)
			{
				
				Graber::g_GetGrabPack().Stop();
				Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
				Graber::g_GetCamera()->OpenInternalTrigger(1);
				kxPrintf(KX_INFO, "停止采集");

			}

			if (dataHeader.nMsgType == MSG_CHANGE_EXPOURE_TIME)
			{
				unsigned char* pt = (unsigned char*)pExtData;
				int nExpoureTime = (int)(*((int*)pt));   //曝光时间
				Config::GetGlobelParam().m_nExpoureTime = nExpoureTime;
			}

		}
	}
}

void CTestAsioTcpServer::Start(CTestAsioTcpServer *pServer)
{
	pServer->m_pAsioTcp->Start();
}

void CTestAsioTcpServer::SendMsg(int nStationID, int nMsgType, int nExtDataSize, const char* pExtData)
{
	boost::unique_lock<boost::mutex> lock(m_mutex);
	DataHeader dataHeader;
	dataHeader.nStationID = nStationID;
	dataHeader.nMsgType = nMsgType;
	dataHeader.nExtDataSize = nExtDataSize;
	// 	memcpy(dataHeader.szHeaderExtData, pHeadExtData, sizeof(char) * dataHeader.nHeaderExtDataSize); 

	int nDataSize = sizeof(DataHeader)+dataHeader.nExtDataSize;
	if (nDataSize > m_nCurSendBufferSize)
	{
		m_pSendDataBuffer = (unsigned char *)realloc(m_pSendDataBuffer, nDataSize * sizeof(unsigned char));
		m_nCurSendBufferSize = nDataSize;
	}
	memcpy(m_pSendDataBuffer, &dataHeader, sizeof(DataHeader));
	memcpy(m_pSendDataBuffer + sizeof(DataHeader), pExtData, dataHeader.nExtDataSize);

	m_pAsioTcp->SendMsg(nStationID, (const char *)m_pSendDataBuffer, nDataSize);
}


void CTestAsioTcpServer::SendImgMsg(int nStationID, int nMsgType, int nExtDataSize, int nWidth, int nHeight, int nPitch, int nType, const char* pExtData)
{
	boost::unique_lock<boost::mutex> lock(m_mutex);
	DataHeader dataHeader;
	dataHeader.nStationID = nStationID;
	dataHeader.nMsgType = nMsgType;
	dataHeader.nExtDataSize = nExtDataSize;
	// 	memcpy(dataHeader.szHeaderExtData, pHeadExtData, sizeof(char) * dataHeader.nHeaderExtDataSize); 

	int nDataSize = sizeof(DataHeader)+dataHeader.nExtDataSize;
	if (nDataSize > m_nCurSendBufferSize)
	{
		m_pSendDataBuffer = (unsigned char *)realloc(m_pSendDataBuffer, nDataSize * sizeof(unsigned char));
		m_nCurSendBufferSize = nDataSize;
	}
	memcpy(m_pSendDataBuffer, &dataHeader, sizeof(DataHeader));
	memcpy(m_pSendDataBuffer + sizeof(DataHeader)+0 * sizeof(int), &nWidth, sizeof(int));
	memcpy(m_pSendDataBuffer + sizeof(DataHeader)+1 * sizeof(int), &nHeight, sizeof(int));
	memcpy(m_pSendDataBuffer + sizeof(DataHeader)+2 * sizeof(int), &nPitch, sizeof(int));
	memcpy(m_pSendDataBuffer + sizeof(DataHeader)+3 * sizeof(int), &nType, sizeof(int));
	memcpy(m_pSendDataBuffer + sizeof(DataHeader)+4 * sizeof(int), pExtData, dataHeader.nExtDataSize - 4 * sizeof(int));

	m_pAsioTcp->SendMsg(nStationID, (const char *)m_pSendDataBuffer, nDataSize);
}

void CTestAsioTcpServer::SendMsg(int nStationID, int nMsgType, const char *pData[], int nSize[], int nLen)
{
	boost::unique_lock<boost::mutex> lock(m_mutex);
	int nExtDataSize = 0;
	for (int i = 0; i < nLen; i++)
		nExtDataSize += nSize[i];
	DataHeader dataHeader;
	dataHeader.nStationID = nStationID;
	dataHeader.nMsgType = nMsgType;
	dataHeader.nExtDataSize = nExtDataSize;
	// 	memcpy(dataHeader.szHeaderExtData, pHeadExtData, sizeof(char) * dataHeader.nHeaderExtDataSize); 

	int nDataSize = sizeof(DataHeader)+dataHeader.nExtDataSize;
	if (nDataSize > m_nCurSendBufferSize)
	{
		m_pSendDataBuffer = (unsigned char *)realloc(m_pSendDataBuffer, nDataSize * sizeof(unsigned char));
		m_nCurSendBufferSize = nDataSize;
	}
	memcpy(m_pSendDataBuffer, &dataHeader, sizeof(DataHeader));
	int nOffset = sizeof(DataHeader);
	for (int i = 0; i < nLen; i++)
	{
		memcpy(m_pSendDataBuffer + nOffset, pData[i], nSize[i]);
		nOffset += nSize[i];
	}

	m_pAsioTcp->SendMsg(nStationID, (const char *)m_pSendDataBuffer, nDataSize);
}

std::string CTestAsioTcpServer::FormatIntArrayToString(const int* a, int n)
{
	std::ostringstream os;
	for (int i = 0; i < n; i++)
		os.write(reinterpret_cast<const char *>(&a[i]), sizeof(int));

	if (os.fail())
		return "";
	return os.str();
}

CTestAsioTcpServer* g_Server = NULL;
