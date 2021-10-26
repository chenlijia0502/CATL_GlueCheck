#include "stdafx.h"
#include "TestAsioTcpClient.h"
#include "KxCheck.h"
#include "kxParameter.h"
#include "GrabPack.h"
#include "KxReadXml2.h"
#include "GlueCheck.h"

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
	//InitPythonEnvironment();
}

CKxCheck::~CKxCheck()
{
	//释放protobuf内存
	ReleaseCheckMethod();
}

void CKxCheck::InitCheckMethod()
{//开发者在这写上实例化类

	m_hCheckTools[0] = new CGlueCheck;
		
	//m_hCheckTools[1].InitCheckMethod(new ...);
	//.......
}

void CKxCheck::ReleaseCheckMethod()
{//开发者在这写上释放资源
	for (int i = 0; i < Config::GetGlobalParam().m_nAreakNum; i++)
		delete m_hCheckTools[i];
	//delete 
	//delete 
}

bool CKxCheck::ReadReadJudgeStandardParaByXmlinChinese(const char* lpszFile, int nIndex)
{
	/*!
		读取质量检查参数，只有当参数设置界面存在表达式需要判断时才需要进行读取操作
	*/
	string szResult;
	int nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "质量检查标准", "检查标准组数", szResult);
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
		sprintf_s(sz, 128, "质量检查标准%d", i);
		nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "质量检查标准", sz, "缺陷名", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		strcpy_s(m_hJudgeStandard[nIndex].m_szErrName[i], sizeof(m_hJudgeStandard[nIndex].m_szErrName[i]), szResult.c_str());

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "质量检查标准", sz, "表达式", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		strcpy_s(m_hJudgeStandard[nIndex].m_szRules[i], sizeof(m_hJudgeStandard[nIndex].m_szRules[i]), szResult.c_str());

		//nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, sz, "判废数", szResult);
		//if (!nSearchStatus)
		//{
		//	return false;
		//}
		//nStatus = KxXmlFun2::FromStringToInt(szResult, m_hJudgeStandard[nIndex].m_nDefectCount[i]);
		//if (!nStatus)
		//{
		//	return false;
		//}
		//nSearchStatus = KxXmlFun::SearchXmlGetValue(lpszFile, sz, "报错方式", szResult);
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
		读取表达式参数，分中英文
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

int CKxCheck::TransferImage(const CKxCaptureImage& card)
{
	/*!
		bayer转换
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

			////Bayer转换 1通道-3通道，即CFA格式转RGB格式
			//if (Config::g_GetParameter().m_nSaveBayerImg  && Config::g_GetParameter().m_bSaveImg)
			{
				IppiRect srcRoiQ = { 0, 0, card.m_Image.nWidth, card.m_Image.nHeight};
				IppiRect dstRoiQ = { 0, 0, card.m_Image.nWidth, card.m_Image.nHeight };
				IppiSize srcSizeQ = { card.m_Image.nWidth, card.m_Image.nHeight };
				m_BayerImg.Init(srcSizeQ.width, srcSizeQ.height, 3);
				IppiBayerGrid  Byermode;  
				if (_GRBG == Config::g_GetParameter().m_nBayerMode)
				{
					//GRBG模式
					Byermode = ippiBayerGRBG;
				}
				else if (_BGGR == Config::g_GetParameter().m_nBayerMode)
				{
					//BGGR模式
					Byermode = ippiBayerBGGR;

				}
				else if (_GBRG == Config::g_GetParameter().m_nBayerMode)
				{
					//GBRG模式
					Byermode = ippiBayerGBRG;
				}
				else
				{
					//RGGB模式
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
{//清空上张图的处理结果

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
	if (m_estatus == SaveImgStatus::_SAVEALL)
	{
		if (m_nimgsavenum < Config::g_GetParameter().m_nSaveImgTotalCounts)
		{
			if (_access(m_csaveimgpath, 0))
				_mkdir(m_csaveimgpath);
			char szNewName[256];
			sprintf_s(szNewName, sizeof(szNewName), "%s\\%d.bmp", m_csaveimgpath, m_nimgsavenum);
			m_hBaseFun.SaveBMPImage_h(szNewName, m_TransferImage);
			m_nimgsavenum++;
		}
	}
	else if (m_estatus == SaveImgStatus::_SAVEBAD)
	{
		if (status > _Check_Err && m_nimgsavenum < Config::g_GetParameter().m_nSaveImgTotalCounts)
		{
			if (_access(m_csaveimgpath, 0))
				_mkdir(m_csaveimgpath);
			char szNewName[256];
			sprintf_s(szNewName, sizeof(szNewName), "%s\\%d.bmp", m_csaveimgpath, m_nimgsavenum);
			m_hBaseFun.SaveBMPImage_h(szNewName, m_TransferImage);
			m_nimgsavenum++;
		}
	}
	else
	{
		return;
	}


}

void CKxCheck::SetSaveStatus(CKxCheck::SaveImgStatus status, char* savepath)
{//选择保存状态，是
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
	// 通过表达式判定检测结果
	sendresult["defect num"] = 0;
	sendresult["checkstatus"] = _Check_Ok;

	int defectnum = checkresult["defect num"].asInt();
	for (int i = 0; i < defectnum; i++)
	{
		// 对应关系看主站 “表达式”的注释
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
				singlefeature["defect name"] = m_hJudgeStandard[0].m_szErrName[j]; //取第一个判断的缺陷名
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

	int ndefectnum = m_hCheckResultDef[x].ndefectcount();//归零
	int nrulenum = m_hJudgeStandard[x].m_nRuleCount;
	for (int i = 0; i < nrulenum; i++)
	{
	memset(m_bJudgeStatus[i], 0, sizeof(int)* ndefectnum);
	}

	//缺陷个数为0
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
	//readme: 下面看着会比较复杂，但细想需求就可明白，分两步，一是对每个缺陷判断每个规则，二是根据一的比较结果来确定是否判废
	//1.判定
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

	//2.根据上面的判定数组跟判废数来确定是否真的要判废
	for (int i = 0; i < nrulenum; i++)
	{
	int nsum = 0;
	for (int j = 0; j < ndefectnum; j++)
	{
	nsum += m_bJudgeStatus[i][j];
	}
	if (nsum >= m_hJudgeStandard[x].m_nDefectCount[i])
	{
	//-------------------报错方式--------------------//
	//  多种报错方式中以最严重的为准


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
	break;//确定给子站发的缺陷最多不超过规定项
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
	if (szName == "X坐标")
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

	//合并N张图发送结果(20180912 因为现在是主站进行报警，所以每一张的结果都应该发送给主站，让主站进行判断，但是具体发的内容待定)
	m_nIndex++;
	if (m_nIndex % Config::g_GetParameter().m_nSendInfoInterval == 0)
	{

	m_hSendResultInfo[nTmpIndex].set_nstationid(Config::g_GetParameter().m_nNetStationId);
	//ID为合并
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

	if (m_bOpenFileStatus)  //文件打开成功
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




	//将所有区域的结果比较，取最大的（结果越大，错误越严重），变量用来确定是否是坏图，用于保存图片
	m_finalcheckstatus = CheckResultStatus(std::max(int(m_finalcheckstatus), int(m_hSendResultInfo[nTmpIndex].nqualitylevel())));

	//清空结果
	for (int x = 0; x != Config::GetGlobalParam().m_nAreakNum; x++)
	{
	m_hSendResultInfo[x].Clear();
	}
	m_nIndex = 0;

	}*/

	
	// HYH 2020.02.27 这里只有一个检查，所以我只需要将0号检查结果发过去即可
	

	Json::Value sendresult;
	//JudgeCheckStaus(checkresult[0], sendresult);
	sendresult = checkresult[0];

	sendresult["id"] = nCardID;
	sendresult["imagepath"] = Config::g_GetParameter().m_szNetSaveImagePath;

	unsigned int nOffset;
	bool m_bOpenFileStatus = g_SaveImgQue.OpenFile(Config::g_GetParameter().m_szNetSaveImagePath, m_DstImg.nWidth, m_DstImg.nHeight, m_DstImg.nPitch, 500);
	if (m_bOpenFileStatus)  //文件打开成功
	{
		g_SaveImgQue.SaveImg(m_DstImg, nOffset);
		sendresult["startoffset"] = nOffset;
		sendresult["imageoffsetlen"] = m_DstImg.nHeight * m_DstImg.nPitch + 5 * 4;//这个‘值’看g_SaveImgQue.SaveImg()，存储的数据偏移+5个int
	}
	else
	{
		char word[256];
		sprintf_s(word, 256, "存图路径打开失败：%s", Config::g_GetParameter().m_szNetSaveImagePath);
		kxPrintf(KX_Err, word);
	}

	std::string sendstr = sendresult.toStyledString();
	if (Net::IsExistNetObj())
	{
		Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_CHECK_RESULT), int(sendstr.size()), sendstr.c_str());
	}

	return 1;
}

int CKxCheck::Check(const CKxCaptureImage& SrcCapImg)
{

	tick_count tbb_start, tbb_end;
	tbb_start = tick_count::now();
	
	//1.转换图像，初始化每次检测
	TransferImage(SrcCapImg);
	ClearResult(SrcCapImg.m_CardID);
	m_finalcheckstatus = CheckResultStatus::_Check_Ok;


	//2.检测
	for (int i = 0; i < Config::GetGlobalParam().m_nAreakNum; i++)
	{
		m_hCheckResult[i].clear();
		m_bCheckStatus[i] = m_hCheckTools[i]->Check(m_TransferImage, m_DstImg, m_hCheckResult[i]);

	}

	


	//// 并行版本，开发者自行选择
	//parallel_for(blocked_range<int>(0, Config::GetGlobalParam().m_nAreakNum),
	//	[&](const blocked_range<int>& range)
	//{
	//	for (int index = range.begin(); index != range.end(); index++)
	//	{
	//		m_bCheckStatus[i] = m_hCheckTools[i]->Check(m_TransferImage, m_DstImg, m_hCheckResult[i]);
	//	}
	//}, auto_partitioner());




	
	//3. 分析结果，比如用表达式进行判废（这一步以前的同事设计的时候不把它放在主站的原因是因为耗时原因），这里是对所有区域进行汇合的判废
	AnalyseCheckResult(SrcCapImg.m_CardID, m_hCheckResult);

	//total_e = tick_count::now();
	//printf("check a image  %d: ----- cost : %f ms\n", card.m_CardID, (total_e - total_s).seconds() * 1000);

	SaveImg(CheckResultStatus(m_finalcheckstatus));
	tbb_end = tick_count::now();
	printf("----- cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);
	return 1;
}



CKxCheck   g_CheckObj;
namespace Check
{

}