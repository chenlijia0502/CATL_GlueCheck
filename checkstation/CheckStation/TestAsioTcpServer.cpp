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
	\brief �ȼ��˿ں���û�б�ռ�ã�IsPortBind���ص���ռ�ö˿ڵĽ��̺�
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
		// ��ʼ�������̲߳�������Start���̺߳�����this�Ǵ���Ĳ���
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
	kxPrintf(KX_INFO, "������������");
}

void CTestAsioTcpServer::OnRecvData(socket_handle socket, const unsigned char* pData, size_t nDataSize)
{
	kxPrintf(KX_DEBUG, "�յ��������ݰ�����С = %d bytes", nDataSize);

	// �ж������Ƿ���Ч��pData�����ݰ�ͷ������������sizeof(DataHeader)��ô��
	if (pData && nDataSize >= sizeof(DataHeader))
	{
		DataHeader dataHeader;
		memcpy(&dataHeader, pData, sizeof(DataHeader));

		// ����ͷ��������Ƿ���Ч��nDataSize�ĳ���Ϊsizeof(DataHeader) + �������ݵĳ���
		if (dataHeader.nExtDataSize == nDataSize - sizeof(DataHeader))
		{
			// pExtDataΪ�������ݵ�ָ��
			const unsigned char *pExtData = pData + sizeof(DataHeader);
			if (dataHeader.nExtDataSize == 0)
				pExtData = NULL;
			//// �����ж���Ϣ������������Ӧ�Ĵ������ˣ����������
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
				if (!bStatus)  //�ѿ����󷢺�ͼ
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
				if (Config::g_GetParameter().m_nIsperiodicCheck)  //����Ԥ����
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
				//ֹͣ���
				//kxPrintf(KX_INFO, "ֹͣ���");
				Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
				g_Environment.StopCheck();
				//�������
				char* pFileName = (char*)pExtData;
				kxPrintf(KX_DEBUG, "����·����%s", pFileName);
				Config::g_GetParameter().m_szModelSaveDir = pFileName;

				int nStatus = g_Environment.LoadAllObj(pFileName);
				if (!nStatus)
				{
					kxPrintf(KX_ERR, "��������ʧ�ܣ��޷���ʼ�������");
					return; //��������ʧ�ܣ������
				}

				Graber::g_GetGraberBuffer().Init(true);
				kxPrintf(KX_DEBUG, "����ģ�����");
				
				Config::SetSaveImageStatus(false);
				Check::g_GetCheckCardObj().ClearCardStatus();
				g_Environment.StartCheck();
				kxPrintf(KX_INFO, "��ʼ���");
				//Ĭ�Ͽ�������߻���ģʽ
				Graber::g_GetCamera()->OpenSoftwareControl();
			}
			if (dataHeader.nMsgType == int(MSG_STOP_CHECK))
			{
				kxPrintf(KX_INFO, "ֹͣ���");
				g_Environment.StopCheck();
				Config::g_GetParameter().m_bSaveImg = false;
				Config::g_GetParameter().m_nVariableJugeFlag = 0;
				Graber::g_GetCamera()->CloseSoftwareControl();
				//Graber::g_GetCamera()->SetActiveTrigger();

			}
			if (dataHeader.nMsgType == int(MSG_SAVE_IMAGE))
			{
				char* pFileName = (char*)pExtData;
				kxPrintf(KX_DEBUG, "��ͼ·����%s", pFileName);
				strcpy_s(Config::g_GetParameter().m_szSavePath, pFileName);

				kxPrintf(KX_INFO, "��ʼ��ͼ");
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
				//ֹͣ���
				kxPrintf(KX_INFO, "ֹͣ���");
				g_Environment.StopCheck();
				//�������

				char* pFileName = (char*)pExtData;
				kxPrintf(KX_INFO, "����·����%s", pFileName);

				Config::g_GetParameter().m_szModelSaveDir = pFileName;

				int nStatus = g_Environment.LoadAllObj(pFileName);
				if (!nStatus)
				{
					kxPrintf(KX_ERR, "��������ʧ�ܣ��޷���ʼ�������");
					return; //��������ʧ�ܣ������
				}

				g_Environment.InitAllObj(0);
				Graber::g_GetGraberBuffer().Init(false);
				//kxPrintf(KX_INFO, "��ʼ�����");
		
				//��ʼģ����		
				Config::SetSaveImageStatus(false);
				const char* pFileName1 = (char*)pExtData;
				pFileName1 += strlen(pFileName1) + 1;

				if (_access(pFileName1, 0) != 0)
					kxPrintf(KX_ERR, "��ʼģ����·��������,·��Ϊ��%s", pFileName1);
				else
				{
					kxPrintf(KX_INFO, "��ʼģ����_·��Ϊ��%s", pFileName1);
					g_Environment.StartSimulation(pFileName1);
				}


			}

			if (dataHeader.nMsgType == MSG_SET_CUR_MODEL)
			{
				g_Environment.ClearAllObj();
				char* pFileName = (char*)pExtData;
				kxPrintf(KX_DEBUG, "����·����%s", pFileName);

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
				kxPrintf(KX_DEBUG, "����ģ�����");
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
				kxPrintf(KX_INFO, "����ƫ��ֵ");
				unsigned char* pt = (unsigned char*)pExtData;
				int nAreaId = (int)(*((int*)pt));            //�����
				pt += sizeof(int);
				CKxBaseFunction  hFun;
				kxCImageBuf  ImgTemp;
				hFun.readImgBufFromMemory(ImgTemp, pt);     //ԭʼͼ��

				int nOffset[5];                             //��Ӧ5��ƫ��ֵ
				memcpy(nOffset, pt, sizeof(int)* 5);
				pt += sizeof(int)*5;

				CkxCheckTool* m_hCheckObj = new CkxCheckTool;

				int nDirLen = (int)(*((int*)pt));
				pt += sizeof(int);
				char* pFileName = new char[nDirLen + 1];
				memset(pFileName, 0, sizeof(char)*nDirLen);  //����·��
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
					sprintf_s(sz, 128, "�����������_%d", nStatus);
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
					sprintf_s(sz, 128, "��ɫ�в�ͼ�ϲ�����_%s", hCall.szErrInfo);
					kxPrintf(KX_INFO, sz);
				}
				
	            //��ȡBlob�������
				enum
				{
					_BLOB_FEATURES = 24,//ÿһ��Blob������
					_MAX_BLOB = 8,		//�ܵ�Blob��
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
				kxPrintf(KX_DEBUG, "����ȱ��ѧϰ");
				unsigned char* pt = (unsigned char*)pExtData;
				int nAreaId = (int)(*((int*)pt));  //�����
				pt += sizeof(int);
				CKxBaseFunction  hFun;
				kxCImageBuf  ImgTemp;             //�Ľ�����ͼ

				int nOffset = (int)(*((int*)pt));  //�����
				pt += sizeof(int);
				g_SaveImgQue.ReadImgFromFileByOffset(nOffset, ImgTemp);

				IppiSize roiSize = { ImgTemp.nWidth, ImgTemp.nHeight };
				if (ImgTemp.nChannel == _Type_G24)
				{
					int nDstOder[3] = { 2, 1, 0 };
					ippiSwapChannels_8u_C3IR(ImgTemp.buf, ImgTemp.nPitch, roiSize, nDstOder);
				}

				//hFun.SaveBMPImage_h("d:\\123.bmp", ImgTemp);
				int nBlobIndex;                     //ȱ�ݵı��
				memcpy(&nBlobIndex, pt, sizeof(int)* 1);
				pt += sizeof(int);

				int nDirLen = (int)(*((int*)pt));   //����·��
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
					sprintf_s(sz, 128, "����ȱ��ѧϰ_%s,ȱ��ѧϰʧ��", hCall.szErrInfo);
					kxPrintf(KX_INFO, sz);
				}
				else
				{
					m_hCheckObj.SaveLearnTemplateImg(szModelPath);
					{
						char sz[128];
						sprintf_s(sz, 128, "����%d��%d��ȱ�ݾֲ�ѧϰ���_%s", nAreaId, nBlobIndex, hCall.szErrInfo);
						kxPrintf(KX_INFO, sz);
					}
				}

				string  szNet = m_hBaseFun.FormatIntToString(nAreaId);
				g_Server->SendMsg(Config::GetGlobelParam().m_nStationID, MSG_LEARN_DEFECT_COMPLETED, int(szNet.size()), szNet.c_str());

				delete[]pFileName;
			}


			if (dataHeader.nMsgType == MSG_LEARN_ONE)
			{
				kxPrintf(KX_DEBUG, "����ȱ��ѧϰ");
				unsigned char* pt = (unsigned char*)pExtData;
				int nAreaId = (int)(*((int*)pt));  //�����
				pt += sizeof(int);
				CKxBaseFunction  hFun;
				kxCImageBuf  ImgTemp;             //�Ľ�����ͼ

				int nOffset = (int)(*((int*)pt));  //ƫ��ֵ
				pt += sizeof(int);
				g_SaveImgQue.ReadImgFromFileByOffset(nOffset, ImgTemp);

				IppiSize roiSize = { ImgTemp.nWidth, ImgTemp.nHeight };
				if (ImgTemp.nChannel == _Type_G24)
				{
					int nDstOder[3] = { 2, 1, 0 };
					ippiSwapChannels_8u_C3IR(ImgTemp.buf, ImgTemp.nPitch, roiSize, nDstOder);
				}

				//hFun.SaveBMPImage_h("d:\\123.bmp", ImgTemp);
				//int nBlobIndex;                     //ȱ�ݵı��
				//memcpy(&nBlobIndex, pt, sizeof(int)* 1);
				//pt += sizeof(int);

				int nDirLen = (int)(*((int*)pt));   //����·��
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
					sprintf_s(sz, 128, "����ѧϰ_%s, ѧϰʧ��", hCall.szErrInfo);
					kxPrintf(KX_INFO, sz);
				}
				else
				{
					m_hCheckObj.SaveLearnTemplateImg(szModelPath);
					{
						char sz[128];
						sprintf_s(sz, 128, "����%d�ĵ���ѧϰ���_%s", nAreaId,  hCall.szErrInfo);
						kxPrintf(KX_INFO, sz);
					}
				}

				string  szNet = m_hBaseFun.FormatIntToString(nAreaId);
				g_Server->SendMsg(Config::GetGlobelParam().m_nStationID, MSG_LEARN_ONE_COMPLETED, int(szNet.size()), szNet.c_str());

				delete[]pFileName;
			}


			//������������ļ�
			if (dataHeader.nMsgType == MSG_LOAD_VARIINO_DATA)
			{
				unsigned char* pt = (unsigned char*)pExtData;
				int nAreaId = (int)(*((int*)pt));   //�����
				pt += sizeof(int);
				int nDirLen = (int)(*((int*)pt));   //�����
				pt += sizeof(int);

				char* pFileName = new char[nDirLen + 1];
				memset(pFileName, 0, sizeof(char)*nDirLen);
				ippsCopy_8u((Ipp8u*)pt, (Ipp8u*)pFileName, nDirLen);
				pFileName[nDirLen] = '\0';

				g_SaveImgQue.OpenVariableInfoFile(pFileName);
				Config::g_GetParameter().m_nVariableJugeFlag = 1;
				kxPrintf(KX_DEBUG, "��������·�� %s", pFileName);
				kxPrintf(KX_INFO, "����������óɹ�");
			}

			if (dataHeader.nMsgType == MSG_START_CAMERA)
			{
				unsigned char* pt = (unsigned char*)pExtData;
				int nExpoureTime = (int)(*((int*)pt));   //�ع�ʱ��
				Config::GetGlobelParam().m_nExpoureTime = nExpoureTime;
				Config::g_GetParameter().m_bChangeExpoureTimeStatus = true;
				Config::g_GetParameter().m_nSendImageCount = 0;
				Graber::g_GetGraberBuffer().Init(true);
				//��ʼ�ɼ�
				Graber::g_GetGrabPack().Stop();
				//���������
				Graber::g_GetCamera()->OpenInternalTrigger(0);
				Graber::g_GetGrabPack().Start();
				kxPrintf(KX_INFO, "��ʼ�ڴ����ɼ�");

			}

			if (dataHeader.nMsgType == MSG_STOP_CAMERA)
			{
				
				Graber::g_GetGrabPack().Stop();
				Config::g_GetParameter().m_bChangeExpoureTimeStatus = false;
				Graber::g_GetCamera()->OpenInternalTrigger(1);
				kxPrintf(KX_INFO, "ֹͣ�ɼ�");

			}

			if (dataHeader.nMsgType == MSG_CHANGE_EXPOURE_TIME)
			{
				unsigned char* pt = (unsigned char*)pExtData;
				int nExpoureTime = (int)(*((int*)pt));   //�ع�ʱ��
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
