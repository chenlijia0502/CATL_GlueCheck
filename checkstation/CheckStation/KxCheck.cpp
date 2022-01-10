#include "stdafx.h"
#include "TestAsioTcpClient.h"
#include "KxCheck.h"
#include "kxParameter.h"
#include "GrabPack.h"
#include "KxReadXml2.h"
#include "GlueCheck.h"
#include <ctime>

//int InitPythonEnvironment()
//{
//	Py_NoSiteFlag = 1;
//
//	Py_Initialize();
//	if (!Py_IsInitialized())
//		return -1;
//	return 0;
//}

CKxCheck::CKxCheck()
{
	m_nimgsavenum = 0;
	InitCheckMethod();
	m_echeckstatus = _OFFLINE_RUN;

}

CKxCheck::~CKxCheck()
{
	//�ͷ�protobuf�ڴ�
	ReleaseCheckMethod();
}

void CKxCheck::InitCheckMethod()
{//����������д��ʵ������

	m_hCheckTools[0] = new CGlueCheck;
		
	//m_hCheckTools[1].InitCheckMethod(new ...);
	//.......
}

void CKxCheck::ReleaseCheckMethod()
{//����������д���ͷ���Դ
	//for (int i = 0; i < Config::GetGlobalParam().m_nAreakNum; i++)
	delete m_hCheckTools[0];
	//delete 
	//delete 
}

bool CKxCheck::ReadReadJudgeStandardParaByXmlinChinese(const char* lpszFile, int nIndex)
{
	/*!
		��ȡ������������ֻ�е��������ý�����ڱ��ʽ��Ҫ�ж�ʱ����Ҫ���ж�ȡ����
	*/
	string szResult;
	int nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "��������׼", "����׼����", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus = KxXmlFun2::FromStringToInt(szResult, m_hJudgeStandard[nIndex].m_nRuleCount);
	if (!nStatus)
	{
		return false;
	}

	for (int i = 0; i < m_hJudgeStandard[nIndex].m_nRuleCount; i++)
	{
		char sz[128];
		sprintf_s(sz, 128, "��������׼%d", i);
		nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "��������׼", sz, "ȱ����", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		strcpy_s(m_hJudgeStandard[nIndex].m_szErrName[i], sizeof(m_hJudgeStandard[nIndex].m_szErrName[i]), szResult.c_str());

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "��������׼", sz, "���ʽ", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		strcpy_s(m_hJudgeStandard[nIndex].m_szRules[i], sizeof(m_hJudgeStandard[nIndex].m_szRules[i]), szResult.c_str());

		//nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, sz, "�з���", szResult);
		//if (!nSearchStatus)
		//{
		//	return false;
		//}
		//nStatus = KxXmlFun2::FromStringToInt(szResult, m_hJudgeStandard[nIndex].m_nDefectCount[i]);
		//if (!nStatus)
		//{
		//	return false;
		//}
		//nSearchStatus = KxXmlFun::SearchXmlGetValue(lpszFile, sz, "����ʽ", szResult);
		//if (!nSearchStatus)
		//{
		//	return false;
		//}
		//nStatus = KxXmlFun::FromStringToInt(szResult, m_hJudgeStandard[nIndex].m_nAlarmMode[i]);
		//if (!nStatus)
		//{
		//	return false;
		//}


	}

	return true;

}

bool CKxCheck::ReadReadJudgeStandardParaByXmlinEnglish(const char* lpszFile, int nIndex)
{
	string szResult;
	int nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "QualityInspectionStandards", "NumOfStandard", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus = KxXmlFun2::FromStringToInt(szResult, m_hJudgeStandard[nIndex].m_nRuleCount);
	if (!nStatus)
	{
		return false;
	}

	for (int i = 0; i < m_hJudgeStandard[nIndex].m_nRuleCount; i++)
	{
		char sz[128];
		sprintf_s(sz, 128, "DefectName%d", i + 1);
		nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "QualityInspectionStandards", sz, szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		strcpy_s(m_hJudgeStandard[nIndex].m_szErrName[i], sizeof(m_hJudgeStandard[nIndex].m_szErrName[i]), szResult.c_str());

		sprintf_s(sz, 128, "Expression%d", i + 1);
		nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "QualityInspectionStandards", sz, szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		strcpy_s(m_hJudgeStandard[nIndex].m_szRules[i], sizeof(m_hJudgeStandard[nIndex].m_szRules[i]), szResult.c_str());

		sprintf_s(sz, 128, "Nums%d", i + 1);
		nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "QualityInspectionStandards", sz, szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun2::FromStringToInt(szResult, m_hJudgeStandard[nIndex].m_nDefectCount[i]);
		if (!nStatus)
		{
			return false;
		}
	}

	return true;

}

bool CKxCheck::ReadReadJudgeStandardParaByXml(const char* lpszFile, int nIndex)
{
	/*!
		��ȡ���ʽ����������Ӣ��
	*/
	if (Config::g_GetParameter().m_nLanguageMode == 0)
	{
		return ReadReadJudgeStandardParaByXmlinEnglish(lpszFile, nIndex);
	}
	else
	{
		return ReadReadJudgeStandardParaByXmlinChinese(lpszFile, nIndex);
	}

}

bool CKxCheck::ReadParamXml(const char* filePath, char *ErrorInfo)
{
	
	std::string szResult;
	int nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "�����������", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "�����������");
		return false;
	}

	int nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.m_nROINUM);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "��ģͼ������ϵ��", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "��ģͼ������ϵ��");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.m_nimgscalefactor);
	if (!nStatus)
	{
		return false;
	}

	int nhighoffset = 0;

	int nlowoffset = 0;

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "������", "���������", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "���������");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, nhighoffset);
	if (!nStatus)
	{
		return false;
	}

	nhighoffset *= 10;

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "������", "���������", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "���������");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, nlowoffset);
	if (!nStatus)
	{
		return false;
	}

	nlowoffset *= 10;


	for (int nindex = 0; nindex < m_param.m_nROINUM; nindex++)
	{

		m_param.params[nindex].m_noffsethigh = nhighoffset;

		m_param.params[nindex].m_noffsetlow = nlowoffset;

		char name[64];

		memset(name, 0, 64);

		sprintf_s(name, "�������%d", nindex);

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, name, "�������", "pos", szResult);
		
		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, "�������");
			return false;
		}

		nStatus = KxXmlFun2::FromStringToKxRect(szResult, m_param.params[nindex].m_rcCheckROI);
		if (!nStatus)
		{
			return false;
		}

		m_param.params[nindex].m_rcCheckROI.mulC(m_param.m_nimgscalefactor);

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, name, "ɨ�����", szResult);

		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, "ɨ�����");
			return false;
		}

		nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.params[nindex].m_nGrabTimes);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, name, "��ǰ��ID", szResult);

		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, "��ǰ��ID");
			return false;
		}

		nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.params[nindex].m_nCurGrabID);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, name, "��ȡ����Ҷ�", szResult);

		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, "��ȡ����Ҷ�");
			return false;
		}

		nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.params[nindex].m_ndefectthresh);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, name, "������С����", szResult);

		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, "������С����");
			return false;
		}

		nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.params[nindex].m_ndefectdots);
		if (!nStatus)
		{
			return false;
		}

	}



	int nImgW, nImgH;

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "ȫ���������", "�������������", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "�������������");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, nImgW);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "ȫ���������", "�������������", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "�������������");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, nImgH);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "ȫ���������", "��������", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "��������");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.m_nscantimes);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "ƴͼ��Ϣ", "�������", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "�������");
		return false;
	}
	int nmaxheight = 0;

	nStatus = KxXmlFun2::FromStringToInt(szResult, nmaxheight);
	if (!nStatus)
	{
		return false;
	}


	m_ImgMaxSizeA.Init(nImgW, nmaxheight, 3);

	m_ImgMaxSizeB.Init(nImgW, nmaxheight, 3);

	int ncolnum, nrownum;
	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "ƴͼ��Ϣ", "����", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "����");
		return false;
	}
	nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.m_nmaxrownum);
	if (!nStatus)
	{
		return false;
	}

	ncolnum = m_param.m_nscantimes;//ɨ�������Ϊ����
	
	nrownum = m_param.m_nmaxrownum;
	//nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "ƴͼ��Ϣ", "����", szResult);
	//if (!nSearchStatus)
	//{
	//	sprintf_s(ErrorInfo, 256, "����");
	//	return false;
	//}
	//nStatus = KxXmlFun2::FromStringToInt(szResult, ncolnum);
	//if (!nStatus)
	//{
	//	return false;
	//}

	//��ʼ���Դ�ͼ
	int nsingleimgh = nmaxheight / _SPLICING_IMG_SCALEFACTOR;

	int nsingleimgw = nImgW / _SPLICING_IMG_SCALEFACTOR;

	int nbigimgw = nsingleimgw * ncolnum;

	int nbigimgh = nsingleimgh * nrownum;

	m_ImgSplicing.Init(nbigimgw, nbigimgh, 3);

	ippsSet_8u(0, m_ImgSplicing.buf, m_ImgSplicing.nPitch * m_ImgSplicing.nHeight);


	int *nimgnum = new int[m_param.m_nscantimes];

	kxRect<int> * rect = new kxRect<int>[m_param.m_nscantimes];

	for (int i = 0; i < m_param.m_nscantimes; i++)
	{
		char name[64];

		memset(name, 0, 64);

		sprintf_s(name, "ɨ������%dͼ������", i);

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "ɨ������", name, szResult);
		
		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, name);
			return false;
		}

		nStatus = KxXmlFun2::FromStringToInt(szResult, nimgnum[i]);
		if (!nStatus)
		{
			return false;
		}

		memset(name, 0, 64);

		sprintf_s(name, "ƥ��λ��%d", i);

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "ɨ������", name, "pos", szResult);

		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, name);

			return false;
		}

		nStatus = KxXmlFun2::FromStringToKxRect(szResult, rect[i]);
		if (!nStatus)
		{
			return false;
		}

		rect[i].mulC(m_param.m_nimgscalefactor);
	}


	m_hcombineimg.Init(nImgW, nImgH, m_param.m_nscantimes, nimgnum, rect);

	delete[] nimgnum;

	delete[] rect;

}

int CKxCheck::TransferImage(const CKxCaptureImage& card)
{
	/*!
		bayerת��
	*/
	IppiSize Roi;
	switch (card.m_Type)
	{
	case _Type_G24:
		m_TransferImage.SetImageBuf(card.m_Image.buf, card.m_Image.nWidth, card.m_Image.nHeight, card.m_Image.nPitch, card.m_Image.nChannel, true);
		break;
	case _Type_G32:

		if (Config::g_GetParameter().m_nImgType == 0)
		{
			Roi.width = card.m_Image.nWidth;
			Roi.height = card.m_Image.nHeight;
			m_TransferImage.Init(Roi.width, Roi.height, 3);
			ippiCopy_8u_AC4C3R(card.m_Image.buf, card.m_Image.nWidth * 4, m_TransferImage.buf, card.m_Image.nPitch, Roi);
		}
		else
		{
			Roi.width = card.m_Image.nWidth / 2;
			Roi.height = card.m_Image.nHeight / 2;
			m_TransferImage.Init(Roi.width, Roi.height, 3);
			ippiCopy_8u_AC4C3R(card.m_Image.buf, card.m_Image.nWidth / 2 * 4, m_TransferImage.buf, card.m_Image.nPitch / 2, Roi);
		}
		break;


	case _Type_G8:
		if (Config::g_GetParameter().m_nImgType == _Type_G24)
		{
			IppiSize srcSizeQ = { card.m_Image.nWidth, card.m_Image.nHeight };
			IppiRect srcRoiQ = { 0, 0, card.m_Image.nWidth, card.m_Image.nHeight };
			IppiRect dstRoiQ = { 0, 0, card.m_Image.nWidth, card.m_Image.nHeight };

			int nTransferWidth = card.m_Image.nWidth;
			int nTransferHeight = card.m_Image.nHeight;

			m_TransferImage.Init(nTransferWidth, nTransferHeight, 3);

			unsigned char* pBayerData = const_cast<unsigned char*>(card.m_Image.buf);
			unsigned char* pDstData = m_TransferImage.buf;

			int nSrcPitch = card.m_Image.nWidth;
			int nDstPitch = m_TransferImage.nPitch;

			////Bayerת�� 1ͨ��-3ͨ������CFA��ʽתRGB��ʽ
			//if (Config::g_GetParameter().m_nSaveBayerImg  && Config::g_GetParameter().m_bSaveImg)
			{
				IppiRect srcRoiQ = { 0, 0, card.m_Image.nWidth, card.m_Image.nHeight};
				IppiRect dstRoiQ = { 0, 0, card.m_Image.nWidth, card.m_Image.nHeight };
				IppiSize srcSizeQ = { card.m_Image.nWidth, card.m_Image.nHeight };
				m_BayerImg.Init(srcSizeQ.width, srcSizeQ.height, 3);
				IppiBayerGrid  Byermode;  
				if (_GRBG == Config::g_GetParameter().m_nBayerMode)
				{
					//GRBGģʽ
					Byermode = ippiBayerGRBG;
				}
				else if (_BGGR == Config::g_GetParameter().m_nBayerMode)
				{
					//BGGRģʽ
					Byermode = ippiBayerBGGR;

				}
				else if (_GBRG == Config::g_GetParameter().m_nBayerMode)
				{
					//GBRGģʽ
					Byermode = ippiBayerGBRG;
				}
				else
				{
					//RGGBģʽ
					Byermode = ippiBayerRGGB;
				}
				ippiCFAToRGB_8u_C1C3R(pBayerData, srcRoiQ, srcSizeQ, card.m_Image.nPitch, m_TransferImage.buf, m_TransferImage.nPitch, Byermode, 0);

			}

		}
		else
			m_TransferImage.SetImageBuf(card.m_Image.buf, card.m_Image.nWidth, card.m_Image.nHeight, card.m_Image.nPitch, card.m_Image.nChannel, true);
		break;
	default:
		m_TransferImage.SetImageBuf(card.m_Image.buf, card.m_Image.nWidth, card.m_Image.nHeight, card.m_Image.nPitch, card.m_Image.nChannel, true);
		break;
	}


	return 1;

}

int CKxCheck::ClearResult(int nCardId)
{//�������ͼ�Ĵ�����

	for (int i = 0; i < Config::GetGlobalParam().m_nAreakNum; i++)
	{
		//m_hCheckResultDef[i].Clear();
	}


	return 1;
}

int CKxCheck::ClearIndex()
{
	SetSaveStatus(SaveImgStatus::_DEFAULT);
	return 1;
}

void CKxCheck::SaveImg(CheckResultStatus status)
{
	if (m_echeckstatus == _ONLINE_RUN)
	{
		time_t now = time(0);
		tm *ltm = localtime(&now);
		char goodpath[128];
		memset(goodpath, 0, sizeof(goodpath));
		sprintf_s(goodpath, "F:\\%d-%d-%d\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
		if (_access(goodpath, 0))
			_mkdir(goodpath);
		sprintf_s(goodpath, "F:\\%d-%d-%d\\%s\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str());
		if (_access(goodpath, 0))
			_mkdir(goodpath);
		sprintf_s(goodpath, "F:\\%d-%d-%d\\%s\\ԭͼ\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str());
		if (_access(goodpath, 0))
			_mkdir(goodpath);
		sprintf_s(goodpath, "F:\\%d-%d-%d\\%s\\ԭͼ\\%d.bmp", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str(), m_nCurPackIDimgindex);

		m_hBaseFun.SaveBMPImage_h(goodpath, m_TransferImage);
	}


}

void CKxCheck::SetSaveStatus(CKxCheck::SaveImgStatus status, char* savepath)
{//ѡ�񱣴�״̬����
	m_nimgsavenum = 0;
	m_estatus = status;
	memset(m_csaveimgpath, 0, 1<<8);
	if (savepath != NULL)
	{
		int len = strlen(savepath);
		strcpy_s(m_csaveimgpath, len + 1, savepath);
	}
}

void CKxCheck::JudgeCheckStaus(const Json::Value& checkresult, Json::Value& sendresult)
{
	// ͨ�����ʽ�ж������
	sendresult["defect num"] = 0;
	sendresult["checkstatus"] = _Check_Ok;

	int defectnum = checkresult["defect num"].asInt();
	for (int i = 0; i < defectnum; i++)
	{
		// ��Ӧ��ϵ����վ �����ʽ����ע��
		float featurearray[kxErrorMsg::_FEATURE_COUNT];
		memset(featurearray, 0, sizeof(float) * kxErrorMsg::_FEATURE_COUNT);
		Json::Value singlefeature = checkresult["defect feature"][i];
		featurearray[0] = singlefeature["Dots"].asInt();
		featurearray[1] = singlefeature["Energy"].asInt();
		for (int nindex = 0; nindex < 4; nindex++)
		{
			featurearray[2 + nindex] = singlefeature["pos"][nindex].asInt();
		}


		for (int j = 0; j < m_hJudgeStandard[0].m_nRuleCount; j++)
		{
			if (1)
			{
				//if (sendresult["defect num"].asInt() == 0)
				singlefeature["defect name"] = m_hJudgeStandard[0].m_szErrName[j]; //ȡ��һ���жϵ�ȱ����
				sendresult["defect num"] = sendresult["defect num"].asInt() + 1;
				sendresult["defect feature"].append(singlefeature);
				sendresult["checkstatus"] = _Check_Err;
				break;
			}
		}
	}

}

int CKxCheck::AnalyseCheckResult(int nCardID, Json::Value* checkresult)
{
	/*
	int ALARM_MODE = _Check_Err;
	//int ALARM_MODE2 = 0;
	for (int x = 0; x != Config::GetGlobalParam().m_nAreakNum; x++)
	{

	int	hTempQualityLevelSet[KxJudgeStandard::MAX_DEFECT_TYPE_COUNT];
	memset(hTempQualityLevelSet, 0, sizeof(int)*KxJudgeStandard::MAX_DEFECT_TYPE_COUNT);


	int nStatus[KxJudgeStandard::MAX_DEFECT_TYPE_COUNT][_MAX_BLOB_COUNT];
	memset(nStatus, 0, sizeof(int)*(KxJudgeStandard::MAX_DEFECT_TYPE_COUNT)*(_MAX_BLOB_COUNT));
	int nCount[KxJudgeStandard::MAX_DEFECT_TYPE_COUNT];
	memset(nCount, 0, sizeof(int)*(KxJudgeStandard::MAX_DEFECT_TYPE_COUNT));


	m_hSendResultInfo[x].set_nedgestart(m_hCheckResultDef[x].nedgestart());
	m_hSendResultInfo[x].set_nedgeend(m_hCheckResultDef[x].nedgeend());
	m_hSendResultInfo[x].set_nwhitelinepos(m_hCheckResultDef[x].nwhitelinepos());
	m_hSendResultInfo[x].set_szchekinfo(m_hCheckResultDef[x].szchekinfo());
	m_hSendResultInfo[x].set_ncheckstatus(m_hCheckResultDef[x].ncheckstatus());

	int ndefectnum = m_hCheckResultDef[x].ndefectcount();//����
	int nrulenum = m_hJudgeStandard[x].m_nRuleCount;
	for (int i = 0; i < nrulenum; i++)
	{
	memset(m_bJudgeStatus[i], 0, sizeof(int)* ndefectnum);
	}

	//ȱ�ݸ���Ϊ0
	if (ndefectnum == 0)
	{
	switch (m_hCheckResultDef[x].ncheckstatus())
	{
	case _Check_Ok:
	{
	m_hSendResultInfo[x].set_nqualitylevel(_Check_Ok);
	m_hSendResultInfo[x].set_sztye("OK");
	m_hSendResultInfo[x].set_ndefectcount(0);
	break;
	}
	default:
	break;

	}
	}
	else
	{
	//readme: ���濴�Ż�Ƚϸ��ӣ���ϸ������Ϳ����ף���������һ�Ƕ�ÿ��ȱ���ж�ÿ�����򣬶��Ǹ���һ�ıȽϽ����ȷ���Ƿ��з�
	//1.�ж�
	for (int i = 0; i < m_hCheckResultDef[x].ndefectcount(); i++)
	{
	for (int j = 0; j < nrulenum; j++)
	{
	int nFeatureSize = m_hCheckResultDef[x].hdefectmsg(i).hfeaturelist_size();
	float* pFeatures = new float[nFeatureSize];
	for (int k = 0; k < nFeatureSize; k++)
	{
	pFeatures[k] = m_hCheckResultDef[x].hdefectmsg(i).hfeaturelist(k).ffeaturevaule();
	}
	if (m_hSparse[0].Sparse(nFeatureSize, pFeatures, m_hJudgeStandard[x].m_szRules[j]))
	{
	m_bJudgeStatus[j][i] = 1;
	delete[] pFeatures;
	}
	else
	{
	delete[] pFeatures;
	}


	}
	}

	//2.����������ж�������з�����ȷ���Ƿ����Ҫ�з�
	for (int i = 0; i < nrulenum; i++)
	{
	int nsum = 0;
	for (int j = 0; j < ndefectnum; j++)
	{
	nsum += m_bJudgeStatus[i][j];
	}
	if (nsum >= m_hJudgeStandard[x].m_nDefectCount[i])
	{
	//-------------------����ʽ--------------------//
	//  ���ֱ���ʽ���������ص�Ϊ׼


	if (m_hJudgeStandard[x].m_nAlarmMode[i] > ALARM_MODE)
	{
	ALARM_MODE = m_hJudgeStandard[x].m_nAlarmMode[i];
	//if (m_hJudgeStandard[x].m_nAlarmMode[i] == _Check_Err_Alarm)
	//{
	//	ALARM_MODE1 = _Check_Err_Alarm;
	//}
	//else
	//{
	//	ALARM_MODE2 = _Check_Err_Mark;
	//}
	}
	for (int k = 0; k < ndefectnum; k++)
	{
	if (m_hSendResultInfo[x].hdefectmsg_size() >= _MAX_SEND_DEFECTLEN)
	{
	break;//ȷ������վ����ȱ����಻�����涨��
	}
	if (m_bJudgeStatus[i][k] == 1)
	{
	Result::CheckResultInfo_DefectInfo* defectinfo = m_hSendResultInfo[x].add_hdefectmsg();
	defectinfo->set_szdefectname(m_hJudgeStandard[x].m_szErrName[i]);
	defectinfo->set_ndefecttype(i);
	int nFeatureSize = m_hCheckResultDef[x].hdefectmsg(k).hfeaturelist_size();
	for (int g = 0; g < nFeatureSize; g++)
	{
	Result::CheckResultInfo_FeatureAttribute* feature = defectinfo->add_hfeaturelist();
	std::string szName = m_hCheckResultDef[x].hdefectmsg(k).hfeaturelist(g).szfeaturename();
	feature->set_szfeaturename(szName);
	if (szName == "X����")
	{
	feature->set_ffeaturevaule(m_hCheckResultDef[x].hdefectmsg(k).hfeaturelist(g).ffeaturevaule() + m_nIndex * m_DstImg.nWidth);
	}
	else
	{
	feature->set_ffeaturevaule(m_hCheckResultDef[x].hdefectmsg(k).hfeaturelist(g).ffeaturevaule());
	}

	}
	}
	}
	}
	}
	}
	}





	int nTmpQuality = -MAXINT;
	int nTmpIndex = 0;
	for (int x = 0; x<Config::GetGlobalParam().m_nAreakNum; x++)
	{
	if (m_hSendResultInfo[x].nqualitylevel() > nTmpQuality)
	{
	nTmpQuality = m_hSendResultInfo[x].nqualitylevel();
	nTmpIndex = x;
	}
	}

	m_hSendResultInfo[nTmpIndex].set_ndefectcount(m_hSendResultInfo[nTmpIndex].hdefectmsg_size());

	m_BigImg.Init(m_DstImg.nWidth * Config::g_GetParameter().m_nSendInfoInterval, m_DstImg.nHeight);
	IppiSize roiSize = { m_DstImg.nWidth, m_DstImg.nHeight };
	ippiCopy_8u_C1R(m_DstImg.buf, m_DstImg.nPitch, m_BigImg.buf + m_nIndex*m_DstImg.nWidth, m_BigImg.nPitch, roiSize);

	//�ϲ�N��ͼ���ͽ��(20180912 ��Ϊ��������վ���б���������ÿһ�ŵĽ����Ӧ�÷��͸���վ������վ�����жϣ����Ǿ��巢�����ݴ���)
	m_nIndex++;
	if (m_nIndex % Config::g_GetParameter().m_nSendInfoInterval == 0)
	{

	m_hSendResultInfo[nTmpIndex].set_nstationid(Config::g_GetParameter().m_nNetStationId);
	//IDΪ�ϲ�
	m_hSendResultInfo[nTmpIndex].set_nid(nCardID / Config::g_GetParameter().m_nSendInfoInterval);
	m_hSendResultInfo[nTmpIndex].set_nsubid(nCardID);

	if (m_hSendResultInfo[nTmpIndex].hdefectmsg_size() > 0)
	{
	m_hSendResultInfo[nTmpIndex].set_nqualitylevel(ALARM_MODE);
	m_hSendResultInfo[nTmpIndex].set_sztye("NG");
	m_hSendResultInfo[nTmpIndex].set_ndefectcount(m_hSendResultInfo[nTmpIndex].hdefectmsg_size());
	}
	else
	{
	m_hSendResultInfo[nTmpIndex].set_nqualitylevel(_Check_Ok);
	m_hSendResultInfo[nTmpIndex].set_sztye("OK");
	m_hSendResultInfo[nTmpIndex].set_ndefectcount(0);
	}

	unsigned int nOffset;
	//if (g_SaveImgQue.m_fp == NULL)
	m_bOpenFileStatus = g_SaveImgQue.OpenFile(Config::g_GetParameter().m_szNetSaveImagePath, m_BigImg.nWidth, m_BigImg.nHeight, m_BigImg.nPitch, 500);

	if (m_bOpenFileStatus)  //�ļ��򿪳ɹ�
	{
	g_SaveImgQue.SaveImg(m_BigImg, nOffset);
	m_hSendResultInfo[nTmpIndex].set_szreadimgpath(Config::g_GetParameter().m_szNetSaveImagePath);
	m_hSendResultInfo[nTmpIndex].set_noffset(nOffset);
	m_hSendResultInfo[nTmpIndex].set_nwidth(m_BigImg.nWidth);
	m_hSendResultInfo[nTmpIndex].set_nheight(m_BigImg.nHeight);
	m_hSendResultInfo[nTmpIndex].set_npitch(m_BigImg.nPitch);
	}


	string strFormat;
	m_hSendResultInfo[nTmpIndex].SerializeToString(&strFormat);

	if (Net::IsExistNetObj())
	{
	Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_CHECK_RESULT), int(strFormat.size()), strFormat.c_str());
	}




	//����������Ľ���Ƚϣ�ȡ���ģ����Խ�󣬴���Խ���أ�����������ȷ���Ƿ��ǻ�ͼ�����ڱ���ͼƬ
	m_finalcheckstatus = CheckResultStatus(std::max(int(m_finalcheckstatus), int(m_hSendResultInfo[nTmpIndex].nqualitylevel())));

	//��ս��
	for (int x = 0; x != Config::GetGlobalParam().m_nAreakNum; x++)
	{
	m_hSendResultInfo[x].Clear();
	}
	m_nIndex = 0;

	}*/

	
	// HYH 2020.02.27 ����ֻ��һ����飬������ֻ��Ҫ��0�ż��������ȥ����
	

	Json::Value sendresult;
	//JudgeCheckStaus(checkresult[0], sendresult);
	sendresult = checkresult[0];

	sendresult["id"] = nCardID;
	sendresult["imagepath"] = Config::g_GetParameter().m_szNetSaveImagePath;

	unsigned int nOffset;
	
	bool m_bOpenFileStatus = g_SaveImgQue.OpenFile(Config::g_GetParameter().m_szNetSaveImagePath, m_DstImg.nWidth, m_DstImg.nHeight, m_DstImg.nPitch, 60);
	
	if (m_bOpenFileStatus)  //�ļ��򿪳ɹ�
	{
		g_SaveImgQue.SaveImg(m_DstImg, nOffset);
		sendresult["startoffset"] = nOffset;
		sendresult["imageoffsetlen"] = m_DstImg.nHeight * m_DstImg.nPitch + 5 * 4;//�����ֵ����g_SaveImgQue.SaveImg()���洢������ƫ��+5��int
	}
	else
	{
		char word[256];
		sprintf_s(word, 256, "��ͼ·����ʧ�ܣ�%s", Config::g_GetParameter().m_szNetSaveImagePath);
		kxPrintf(KX_Err, word);
	}

	std::string sendstr = sendresult.toStyledString();
	if (Net::IsExistNetObj())
	{
		Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_CHECK_RESULT), int(sendstr.size()), sendstr.c_str());
	}

	return 1;
}

void CKxCheck::DotCheckImg(const kxCImageBuf& SrcImg)
{

}

void CKxCheck::SaveBadImg(const kxCImageBuf& SrcImg, Json::Value& sendresult)
{

	if (m_echeckstatus == _ONLINE_RUN)
	{

		int ndefectnum = sendresult["defect num"].asInt();
		time_t now = time(0);
		tm *ltm = localtime(&now);

		//
		char badpath[128];
		sprintf_s(badpath, "F:\\%d-%d-%d\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
		if (_access(badpath, 0))
			_mkdir(badpath);
		sprintf_s(badpath, "F:\\%d-%d-%d\\%s", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str());
		if (_access(badpath, 0))
			_mkdir(badpath);
		sprintf_s(badpath, "F:\\%d-%d-%d\\%s\\ȱ��ͼ\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str());
		if (_access(badpath, 0))
			_mkdir(badpath);

		for (int i = 0; i < ndefectnum; i++)
		{
			Json::Value single = sendresult["defect feature"][i];

			std::string a = single["defectid"].asString();

			char path[32];

			sprintf_s(path, "%s.bmp", a.c_str());

			sprintf_s(badpath, "F:\\%d-%d-%d\\%s\\ȱ��ͼ\\%s", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str(), path);

			int pos[4];//left,top,width,height

			for (int nindex = 0; nindex < 4; nindex++)
			{
				pos[nindex] = single["pos"][nindex].asInt();
			}
			int cutpos[4];

			cutpos[0] = max(0, pos[0] - 50);

			cutpos[1] = max(0, pos[1] - 50);

			cutpos[2] = min(SrcImg.nWidth - 1, pos[0] + pos[2] + 50 - 1);

			cutpos[3] = min(SrcImg.nHeight - 1, pos[1] + pos[3] + 50 - 1);



			kxRect<int> cutRect;

			cutRect.setup(cutpos[0], cutpos[1], cutpos[2], cutpos[3]);

			kxCImageBuf cutimg;

			cutimg.Init(cutRect.Width(), cutRect.Height(), SrcImg.nChannel);

			m_hBaseFun.KxCopyImage(SrcImg, cutimg, cutRect);

			m_hBaseFun.SaveBMPImage_h(badpath, cutimg);
		}

	}

}

void CKxCheck::CopyImg2SplicImg(const kxCImageBuf& SrcImg, int nrow, int ncol)
{

	int nsingleW = SrcImg.nWidth / _SPLICING_IMG_SCALEFACTOR;

	int nsingleH = SrcImg.nHeight / _SPLICING_IMG_SCALEFACTOR;

	cv::Mat srcmat = cv::Mat(SrcImg.nHeight, SrcImg.nWidth, CV_8UC(SrcImg.nChannel), SrcImg.buf, SrcImg.nPitch);

	IppiSize imgsize = {nsingleW, nsingleH};

	m_ImgResize.Init(nsingleW, nsingleH, 3);

	m_hBaseFun.KxResizeImage(SrcImg, m_ImgResize);

	ippiCopy_8u_C3R(m_ImgResize.buf, m_ImgResize.nPitch, m_ImgSplicing.buf +
		ncol * nsingleW * m_ImgSplicing.nChannel + nrow * nsingleH * m_ImgSplicing.nPitch, m_ImgSplicing.nPitch, imgsize);

	//static int nsaveidx = 0;

	//char savepath[32];

	//sprintf_s(savepath, "d:\\suoluetu%d.bmp", nsaveidx);

	//nsaveidx++;

	//m_hBaseFun.SaveBMPImage_h(savepath, SrcImg);


}

int CKxCheck::Check(const CKxCaptureImage& SrcCapImg)
{
	
	tick_count tbb_start, tbb_end;
	tbb_start = tick_count::now();
	
	//1.ת��ͼ�񣬳�ʼ��ÿ�μ��
	//TransferImage(SrcCapImg);
	m_TransferImage.SetImageBuf(SrcCapImg.m_Image.buf, SrcCapImg.m_Image.nWidth, SrcCapImg.m_Image.nHeight, SrcCapImg.m_Image.nPitch, SrcCapImg.m_Image.nChannel, true);
	
	SaveImg(_Check_Ok);

	
	m_hcombineimg.appendImg(m_TransferImage, m_nCurPackIDimgindex);
	//m_hcombineimg.appendImg(m_TransferImage, SrcCapImg.m_ImageID - 1);

	ClearResult(SrcCapImg.m_CardID);
	
	m_finalcheckstatus = CheckResultStatus::_Check_Ok;
	

	m_nCurPackIDimgindex++;

	// 2. ����ͼ�񣬽��м��
	kxCImageBuf bigimgA, bigimgB;


	for (int j = 0; j < m_param.m_nscantimes; j++)
	{
		if (m_hcombineimg.IsColFull(j))
		{
			for (int i = 0; i < m_param.m_nROINUM; i++)
			{
				// �ж�roi�Ƿ������ɨ������ģ��ǵĻ��ü����м��   2021.12.14
				if (m_param.params[i].m_nGrabTimes == j)
				{
					m_hCheckResult[0].clear();

					m_hCheckResult[0]["ISFULL"] = 0;

					m_hCheckResult[0]["currow"] = m_param.params[i].m_nCurGrabID;

					m_hCheckResult[0]["curcol"] = m_param.params[i].m_nGrabTimes;

					m_hcombineimg.GetImg(bigimgA, bigimgB, j);

					kxCImageBuf checkimgA, checkimgB;

					checkimgA.Init(m_TransferImage.nWidth, m_param.params[i].m_rcCheckROI.Height(), 3);

					checkimgB.Init(m_TransferImage.nWidth, m_param.params[i].m_rcCheckROI.Height(), 3);

					kxRect<int> copyrect;

					copyrect.setup(0, m_param.params[i].m_rcCheckROI.top, m_TransferImage.nWidth - 1, m_param.params[i].m_rcCheckROI.bottom);
 
					m_hBaseFun.KxCopyImage(bigimgA, checkimgA, copyrect);

					//char savepath[32];

					//sprintf_s(savepath, "d:\\1.bmp");

					//m_hBaseFun.SaveBMPImage_h(savepath, bigimgB);

					m_hBaseFun.KxCopyImage(bigimgB, checkimgB, copyrect);

					m_hCheckTools[0]->SetParam(&m_param.params[i]);

					m_hCheckTools[0]->SetCheckBlobID(i);

					ippsSet_8u(0, m_ImgMaxSizeA.buf, m_ImgMaxSizeA.nPitch * m_ImgMaxSizeA.nHeight);

					ippsSet_8u(0, m_ImgMaxSizeB.buf, m_ImgMaxSizeB.nPitch * m_ImgMaxSizeB.nHeight);

					IppiSize copysize = { checkimgA.nWidth, m_param.params[i].m_rcCheckROI.Height()};

					ippiCopy_8u_C3R(checkimgA.buf, checkimgA.nPitch, m_ImgMaxSizeA.buf, m_ImgMaxSizeA.nPitch, copysize);

					ippiCopy_8u_C3R(checkimgB.buf, checkimgB.nPitch, m_ImgMaxSizeB.buf, m_ImgMaxSizeB.nPitch, copysize);

					m_bCheckStatus[0] = m_hCheckTools[0]->Check(m_ImgMaxSizeA, m_ImgMaxSizeB, m_DstImg, m_hCheckResult[0]);

					char savepath[32];

					sprintf_s(savepath, "d:\\dstimg.bmp", m_DstImg);

					m_hBaseFun.SaveBMPImage_h(savepath, m_DstImg);

					SaveBadImg(m_DstImg, m_hCheckResult[0]);

					if (i == m_param.m_nROINUM - 1)// ��ʾȫ��������,��վ�յ������־λ�����
					{
						m_hCheckResult[0]["ISFULL"] = 1;
					}

					CopyImg2SplicImg(m_DstImg, m_param.params[i].m_nCurGrabID, m_param.params[i].m_nGrabTimes);


					AnalyseCheckResult(i, m_hCheckResult);




				}

				
				//if (m_struct2check[i].m_bCanCheck)
				//{
				//	m_hCheckTools[0]->SetParam(&m_param.params[i]);
				//	m_bCheckStatus[0] = m_hCheckTools[0]->Check(m_struct2check[i].m_BigImg, m_DstImg, m_hCheckResult[0]);
				//	m_struct2check[i].m_bCanCheck = false;
				//}
			}

			m_hcombineimg.ResetCol(j);
		}
	}


	// ������ͼ���Լ�������վ��ǰPACK������ ��ȱһ��packID �ţ�

	if (m_hcombineimg.IsAllFull())
	{
		m_hcombineimg.Clear();

		if (m_echeckstatus == _ONLINE_RUN)
		{
			//��������ͼ
			time_t now = time(0);
			tm *ltm = localtime(&now);
			static int nsaveindex = 0;

			char suoluetu[128];
			memset(suoluetu, 0, sizeof(suoluetu));
			sprintf_s(suoluetu, "F:\\%d-%d-%d\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
			if (_access(suoluetu, 0))
				_mkdir(suoluetu);
			sprintf_s(suoluetu, "F:\\%d-%d-%d\\%s\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str());
			if (_access(suoluetu, 0))
				_mkdir(suoluetu);
			sprintf_s(suoluetu, "F:\\%d-%d-%d\\%s\\����ͼ\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str());
			if (_access(suoluetu, 0))
				_mkdir(suoluetu);
			sprintf_s(suoluetu, "F:\\%d-%d-%d\\%s\\����ͼ\\%d.bmp", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str(), nsaveindex);
			m_hBaseFun.SaveBMPImage_h(suoluetu, m_ImgSplicing);

			// ������������ͼ
			ippsSet_8u(0, m_ImgSplicing.buf, m_ImgSplicing.nPitch * m_ImgSplicing.nHeight);
		}

	}


	//// ���а汾������������ѡ��
	//parallel_for(blocked_range<int>(0, Config::GetGlobalParam().m_nAreakNum),
	//	[&](const blocked_range<int>& range)
	//{
	//	for (int index = range.begin(); index != range.end(); index++)
	//	{
	//		m_bCheckStatus[i] = m_hCheckTools[i]->Check(m_TransferImage, m_DstImg, m_hCheckResult[i]);
	//	}
	//}, auto_partitioner());

	//if (bhascheck && (m_hCheckResult[0]["defect num"].asInt() != 0))
	//{
	//	//3. ��������������ñ��ʽ�����зϣ���һ����ǰ��ͬ����Ƶ�ʱ�򲻰���������վ��ԭ������Ϊ��ʱԭ�򣩣������Ƕ�����������л�ϵ��з�
	//}


	//total_e = tick_count::now();
	//printf("check a image  %d: ----- cost : %f ms\n", card.m_CardID, (total_e - total_s).seconds() * 1000);
	
	
	SaveImg(CheckResultStatus(m_finalcheckstatus));
	
	tbb_end = tick_count::now();
	printf_s("----- id %d cost : %f ms\n", SrcCapImg.m_ImageID, (tbb_end - tbb_start).seconds() * 1000);



	return 1;
}




CKxCheck   g_CheckObj;

namespace Check
{

}