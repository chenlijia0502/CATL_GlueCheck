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
	m_nCurPackIDimgindex = 0;
	m_bIscheck = true;
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
	//for (int i = 0; i < Config::GetGlobalParam().m_nAreakNum; i++)
	delete m_hCheckTools[0];
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

bool CKxCheck::ReadParamXml(const char* filePath, char *ErrorInfo)
{

	//// 测试xmlhandle读取
	//tinyxml2::XMLDocument  xmlHandle;
	//int openstatus = KxXmlFun2::OpenXmlFile(filePath, xmlHandle);
	//if (!openstatus)
	//{
	//	return false;
	//}
	//std::string sr;
	//int tststatu = KxXmlFun2::SearchXmlGetValue(xmlHandle, "检测区域数量", sr);

	std::string szResult;
	//int nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "检测区域数量", szResult);
	//if (!nSearchStatus)
	//{
	//	sprintf_s(ErrorInfo, 256, "检测区域数量");
	//	return false;
	//}
	//int nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.m_nROINUM);
	//if (!nStatus)
	//{
	//	return false;
	//}

	int nImgW, nImgH;

	int nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "全局拍摄控制", "相机横向像素数", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "相机横向像素数");
		return false;
	}

	int nStatus = KxXmlFun2::FromStringToInt(szResult, nImgW);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "全局拍摄控制", "相机纵向像素数", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "相机纵向像素数");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, nImgH);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "全局拍摄控制", "拍摄组数", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "拍摄组数");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.m_nscantimes);
	if (!nStatus)
	{
		return false;
	}


	int ncolnum, nrownum;
	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "拼图信息", "行数", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "行数");
		return false;
	}
	nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.m_nmaxrownum);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "建模图像缩放系数", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "建模图像缩放系数");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, m_param.m_nimgscalefactor);
	if (!nStatus)
	{
		return false;
	}

	int nhighoffset = 0;

	int nlowoffset = 0;

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "检测参数", "检高灵敏度", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "检高灵敏度");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, nhighoffset);
	if (!nStatus)
	{
		return false;
	}

	nhighoffset *= 10;

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "检测参数", "检低灵敏度", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "检低灵敏度");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, nlowoffset);
	if (!nStatus)
	{
		return false;
	}

	nlowoffset *= 10;

	int ndefectthresh = 0;

	int ndefectdots = 0;

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "检测参数", "提取异物灰度", szResult);

	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "提取异物灰度");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, ndefectthresh);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "检测参数", "异物最小点数", szResult);

	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "异物最小点数");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, ndefectdots);
	if (!nStatus)
	{
		return false;
	}


	std::string groupword[6] = { "组数一", "组数二", "组数三", "组数四", "组数五", "组数六" };

	m_param.m_nROINUM = 0;

	//int ncurcheckroiid = 0;
	// 读取检测框
	for (int i = 0; i < m_param.m_nscantimes; i++)
	{
		int nchecknum = 0;

		char name[64];

		memset(name, 0, 64);

		sprintf_s(name, "检测区域数量");

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, groupword[i], name, szResult);

		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, name);
			return false;
		}

		nStatus = KxXmlFun2::FromStringToInt(szResult, nchecknum);
		if (!nStatus)
		{
			return false;
		}
		for (int j = 0; j < nchecknum; j++)
		{
			memset(name, 0, 64);

			sprintf_s(name, "检测区域%d", j+1);

			nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, groupword[i], name, "检测区域", "pos", szResult);

			if (!nSearchStatus)
			{
				sprintf_s(ErrorInfo, 256, "检测区域");
				return false;
			}

			nStatus = KxXmlFun2::FromStringToKxRect(szResult, m_param.params[m_param.m_nROINUM].m_rcCheckROI);
			if (!nStatus)
			{
				sprintf_s(ErrorInfo, 256, "检测区域");

				return false;
			}

			m_param.params[m_param.m_nROINUM].m_rcCheckROI.mulC(m_param.m_nimgscalefactor);

			nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, groupword[i], name, "屏蔽当前检测区域", szResult);

			if (!nSearchStatus)
			{
				sprintf_s(ErrorInfo, 256, "屏蔽当前检测区域");
				return false;
			}

			nStatus = KxXmlFun2::FromStringToBool(szResult, m_param.params[m_param.m_nROINUM].m_nIsnotcheck);
			if (!nStatus)
			{
				sprintf_s(ErrorInfo, 256, "屏蔽当前检测区域");
				return false;
			}

			m_param.params[m_param.m_nROINUM].m_nGrabTimes = i;

			m_param.params[m_param.m_nROINUM].m_nCurGrabID = j;

			m_param.params[m_param.m_nROINUM].m_ndefectdots = ndefectdots;

			m_param.params[m_param.m_nROINUM].m_noffsethigh = nhighoffset * 10;

			m_param.params[m_param.m_nROINUM].m_noffsetlow = nlowoffset * 10;

			m_param.m_nROINUM++;

		}

	}

	// 读取高灰度掩膜框
	for (int i = 0; i < m_param.m_nscantimes; i++)
	{
		int nchecknum = 0;

		char name[64];

		memset(name, 0, 64);

		sprintf_s(name, "高灰度掩膜数量");

		int nmasknum = 0;

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, groupword[i], name, szResult);

		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, name);
			return false;
		}

		nStatus = KxXmlFun2::FromStringToInt(szResult, nmasknum);
		if (!nStatus)
		{
			return false;
		}

		kxRect<int> *prect = new kxRect<int>[nmasknum];
		for (int j = 0; j < nmasknum; j++)
		{
			memset(name, 0, 64);

			sprintf_s(name, "高灰度掩膜区域%d", j);

			nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, groupword[i], name, "pos", szResult);

			if (!nSearchStatus)
			{
				sprintf_s(ErrorInfo, 256, "高灰度掩膜区域");
				return false;
			}

			nStatus = KxXmlFun2::FromStringToKxRect(szResult, prect[j]);
			if (!nStatus)
			{
				sprintf_s(ErrorInfo, 256, "高灰度掩膜区域");

				return false;
			}
			prect[j].mulC(m_param.m_nimgscalefactor);
			
		}

		// 将掩膜框转换为检测框内位置
		for (int j = 0; j < m_param.m_nROINUM; j++)
		{
			if (m_param.params[j].m_nGrabTimes == i)
			{
				GetRealmaskpos(m_param.params[i].m_rcCheckROI, prect, nmasknum, m_param.params[i].m_vecrcHighmaskROI);
			}
		}
		delete[]prect;

	}

	// 读取低灰度掩膜框
	for (int i = 0; i < m_param.m_nscantimes; i++)
	{
		int nchecknum = 0;

		char name[64];

		memset(name, 0, 64);

		sprintf_s(name, "低灰度掩膜数量");

		int nmasknum = 0;

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, groupword[i], name, szResult);

		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, name);
			return false;
		}

		nStatus = KxXmlFun2::FromStringToInt(szResult, nmasknum);
		if (!nStatus)
		{
			return false;
		}

		kxRect<int> *prect = new kxRect<int>[nmasknum];
		for (int j = 0; j < nmasknum; j++)
		{
			memset(name, 0, 64);

			sprintf_s(name, "低灰度掩膜区域%d", j);

			nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, groupword[i], name, "pos", szResult);

			if (!nSearchStatus)
			{
				sprintf_s(ErrorInfo, 256, "低灰度掩膜区域");
				return false;
			}

			nStatus = KxXmlFun2::FromStringToKxRect(szResult, prect[j]);
			if (!nStatus)
			{
				sprintf_s(ErrorInfo, 256, "低灰度掩膜区域");

				return false;
			}
			prect[j].mulC(m_param.m_nimgscalefactor);

		}

		// 将掩膜框转换为检测框内位置
		for (int j = 0; j < m_param.m_nROINUM; j++)
		{
			if (m_param.params[j].m_nGrabTimes == i)
			{
				GetRealmaskpos(m_param.params[j].m_rcCheckROI, prect, nmasknum, m_param.params[j].m_vecrcLowmaskROI);
			}
		}
		delete[]prect;

	}



	ncolnum = m_param.m_nscantimes;//扫描次数即为列数
	
	nrownum = m_param.m_nmaxrownum;

	int *nimgnum = new int[m_param.m_nscantimes];

	kxRect<int> * rect = new kxRect<int>[m_param.m_nscantimes * 2];

	for (int i = 0; i < m_param.m_nscantimes; i++)
	{
		char name[64];

		memset(name, 0, 64);

		sprintf_s(name, "扫描区域%d图像数量", i);

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "扫描区域", name, szResult);
		
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


	}

	for (int i = 0; i < m_param.m_nscantimes * 2; i++)
	{
		char name[64];

		memset(name, 0, 64);

		sprintf_s(name, "匹配位置%d", i);

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "扫描区域", name, "pos", szResult);

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


	// 2022.2.7 提取模板中的图作为标准模板，之后提取的再跟它进行滑动残差
	
	for (int i = 0; i < m_param.m_nscantimes; i++)
	{
		std::string readpath;

		char imgpath[32];

		memset(imgpath, 0, 32);

		sprintf_s(imgpath, "底板路径%d", i);

		nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "主站设置", imgpath, readpath);

		if (!nSearchStatus)
		{
			sprintf_s(ErrorInfo, 256, imgpath);

			return false;
		}

		kxCImageBuf bigimg;

		m_hBaseFun.LoadBMPImage_h(readpath.c_str(), bigimg);

		//pimgh[i] = bigimg.nHeight * m_param.m_nimgscalefactor;

		for (int j = 0; j < m_param.m_nROINUM; j++)
		{
			if (m_param.params[j].m_nGrabTimes == i)
			{
				kxRect<int> roi = m_param.params[j].m_rcCheckROI;

				roi.divC(m_param.m_nimgscalefactor);

				// roi 向内缩，目的是测试涂胶整体缩小效果
				//const int nsuoxiaohor = 200;

				//const int nsuoxiaover = 100;

				//roi.left += nsuoxiaohor;

				//roi.right -= nsuoxiaohor;

				//roi.top += nsuoxiaover;

				//roi.bottom -= nsuoxiaover;

				//--------------------

				m_param.params[j].m_ImgTemplate.Init(roi.Width(), roi.Height(), bigimg.nChannel);

				m_hBaseFun.KxCopyImage(bigimg, m_param.params[j].m_ImgTemplate, roi);

				GetG_Template(m_param.params[j].m_ImgTemplate);
			}
		}

	}


	int nmaxheight = 0;

	int nmaxwidth = 0;

	for (int i = 0; i < m_param.m_nROINUM; i++)
	{
		if (nmaxheight < m_param.params[i].m_rcCheckROI.Height())
		{
			nmaxheight = m_param.params[i].m_rcCheckROI.Height();
		}

		if (nmaxwidth < m_param.params[i].m_rcCheckROI.Width())
		{
			nmaxwidth = m_param.params[i].m_rcCheckROI.Width();
		}
	}

	const int nextend = 200;//考虑可能有偏移

	nmaxheight += nextend;

	nmaxwidth += nextend;

	m_nMaximgW = nmaxwidth;

	m_nMaximgH = nmaxheight;

	//初始缩略大图
	int nsingleimgh = m_nMaximgH / _SPLICING_IMG_SCALEFACTOR;

	int nsingleimgw = m_nMaximgW / _SPLICING_IMG_SCALEFACTOR;

	int nbigimgw = nsingleimgw * ncolnum;

	int nbigimgh = nsingleimgh * nrownum;

	m_ImgSplicing.Init(nbigimgw, nbigimgh, 3);

	ippsSet_8u(0, m_ImgSplicing.buf, m_ImgSplicing.nPitch * m_ImgSplicing.nHeight);

	delete[] nimgnum;

	delete[] rect;

	return true;

}

void CKxCheck::GetRealmaskpos(kxRect<int> CheckRect, kxRect<int>* MaskRect, int nmasknum, std::vector<kxRect<int>>& vecrect)
{
	vecrect.clear();

	for (int i = 0; i < nmasknum; i++)
	{
		if (MaskRect[i].right > CheckRect.left && MaskRect[i].left < CheckRect.right  && MaskRect[i].bottom > CheckRect.top && MaskRect[i].top < CheckRect.bottom)
		{
			int nleft = gMax(CheckRect.left, MaskRect[i].left);
			
			int nright = gMin(CheckRect.right, MaskRect[i].right);
			
			int ntop = gMax(CheckRect.top, CheckRect.top);
			
			int nbottom = gMin(CheckRect.bottom, MaskRect[i].bottom);

			kxRect<int> rcrect;

			rcrect.setup(nleft, nright, ntop, nbottom);

			rcrect.offset(CheckRect.left, CheckRect.top);

			vecrect.push_back(rcrect);
		}
	}



}

void CKxCheck::SetPackID(std::string packid)//收到pack id后会初始化所有参数
{
	// 1. 判断是否存在此文件夹，存在则增加命名

	char basepath[256];

	memset(basepath, 0, sizeof(basepath));

	time_t now = time(0);

	tm *ltm = localtime(&now);

	sprintf_s(basepath, "F:\\%d-%d-%d\\%s", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, packid.c_str());

	if (_access(basepath, 0))//文件夹不存在则为首发
	{
		m_sPackID = packid;
	}
	else
	{
		int nindex = 1;

		while (1)
		{
			sprintf_s(basepath, "F:\\%d-%d-%d\\%s-%d", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, packid.c_str(), nindex);

			if (_access(basepath, 0))//文件夹不存在则为可以储存的位置
			{
				char pathname[64];

				memset(pathname, 0, sizeof(pathname));

				sprintf_s(pathname, "%s-%d", packid.c_str(), nindex);

				m_sPackID = pathname;

				break;
			}

			nindex++;
		}


	}

	// 2. 对一些信息进行初始化处理

	m_hcombineimg.Clear();

	m_nCurPackIDimgindex = 0;

}

void CKxCheck::GetG_Template(kxCImageBuf& SrcDstImg)
{
	//通过 2G - R - B的方法得到一张灰度图，再进行低阈值二值化

	kxCImageBuf rgb[3];

	m_hAlg.SplitRGB(SrcDstImg, rgb);

	kxCImageBuf g_b, g_r;

	g_b.Init(SrcDstImg.nWidth, SrcDstImg.nHeight);

	g_r.Init(SrcDstImg.nWidth, SrcDstImg.nHeight);

	IppiSize imgsize = { SrcDstImg.nWidth, SrcDstImg.nHeight };

	ippiSub_8u_C1RSfs(rgb[0].buf, rgb[0].nPitch, rgb[1].buf, rgb[1].nPitch, g_r.buf, g_r.nPitch, imgsize, 0);

	ippiSub_8u_C1RSfs(rgb[2].buf, rgb[2].nPitch, rgb[1].buf, rgb[1].nPitch, g_b.buf, g_b.nPitch, imgsize, 0);

	kxCImageBuf addimg, threshimg;

	addimg.Init(SrcDstImg.nWidth, SrcDstImg.nHeight);

	threshimg.Init(SrcDstImg.nWidth, SrcDstImg.nHeight);

	SrcDstImg.Init(g_b.nWidth, g_b.nHeight, 1);

	ippiAdd_8u_C1RSfs(g_r.buf, g_r.nPitch, g_b.buf, g_b.nPitch, addimg.buf, addimg.nPitch, imgsize, 0);

	m_hAlg.ThreshImg(addimg, threshimg, 10, CEmpiricaAlgorithm::_BINARY);

	m_hBaseFun.KxCloseImage(threshimg, SrcDstImg, 5, 5);

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
		sprintf_s(goodpath, "F:\\%d-%d-%d\\%s\\原图\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str());
		if (_access(goodpath, 0))
			_mkdir(goodpath);
		sprintf_s(goodpath, "F:\\%d-%d-%d\\%s\\原图\\%d.bmp", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str(), m_nCurPackIDimgindex);

		m_hBaseFun.SaveBMPImage_h(goodpath, m_TransferImage);
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
	
	bool m_bOpenFileStatus = g_SaveImgQue.OpenFile(Config::g_GetParameter().m_szNetSaveImagePath, m_DstImg.nWidth, m_DstImg.nHeight, m_DstImg.nPitch, 60);
	
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
		sprintf_s(badpath, "F:\\%d-%d-%d\\%s\\缺陷图\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str());
		if (_access(badpath, 0))
			_mkdir(badpath);

		for (int i = 0; i < ndefectnum; i++)
		{
			Json::Value single = sendresult["defect feature"][i];

			std::string a = single["defectid"].asString();

			char path[32];

			sprintf_s(path, "%s.bmp", a.c_str());

			sprintf_s(badpath, "F:\\%d-%d-%d\\%s\\缺陷图\\%s", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str(), path);

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
	
	//1.转换图像，初始化每次检测
	//TransferImage(SrcCapImg);
	m_TransferImage.SetImageBuf(SrcCapImg.m_Image.buf, SrcCapImg.m_Image.nWidth, SrcCapImg.m_Image.nHeight, SrcCapImg.m_Image.nPitch, SrcCapImg.m_Image.nChannel, true);
	
	SaveImg(_Check_Ok);

	ClearResult(SrcCapImg.m_CardID);

	m_finalcheckstatus = CheckResultStatus::_Check_Ok;

	// 2. 排序图像，进行检测
	kxCImageBuf bigimgA, bigimgB;

	if (m_bIscheck)
	{

		int nresult = m_hcombineimg.appendImg(m_TransferImage, m_nCurPackIDimgindex);
		if (nresult == 0)
		{
			// 发送消息告知子站检测异常
			std::string sendstr = "";
			if (Net::IsExistNetObj())
			{
				Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_CHECK_MATCHERROR), int(sendstr.size()), sendstr.c_str());
			}
		}


		m_nCurPackIDimgindex++;

		for (int j = 0; j < m_param.m_nscantimes; j++)
		{
			if (m_hcombineimg.IsColFull(j))
			{
				for (int i = 0; i < m_param.m_nROINUM; i++)
				{
					// 判断roi是否是这个扫描组里的，是的话裁剪进行检测   2021.12.14
					if ((m_param.params[i].m_nGrabTimes == j) && (m_param.params[i].m_nIsnotcheck != 1))
					{
						m_hCheckResult[0].clear();

						m_hCheckResult[0]["ISFULL"] = 0;

						m_hCheckResult[0]["currow"] = m_param.params[i].m_nCurGrabID;

						m_hCheckResult[0]["curcol"] = m_param.params[i].m_nGrabTimes;

						m_hcombineimg.GetImg(bigimgA, bigimgB, j);

						m_hCheckTools[0]->SetParam(&m_param.params[i]);

						m_hCheckTools[0]->SetCheckBlobID(i);

						m_DstImg.Init(m_nMaximgW, m_nMaximgH, bigimgA.nChannel);

						ippsSet_8u(0, m_DstImg.buf, m_DstImg.nHeight * m_DstImg.nPitch);

						m_hCheckResult[0]["defect num"] = 0;

						m_hCheckResult[0]["blockarea"] = 0;

						m_hCheckResult[0]["area"] = 0;

						m_bCheckStatus[0] = m_hCheckTools[0]->Check(bigimgA, bigimgB, m_DstImg, m_hCheckResult[0]);

						SaveBadImg(m_DstImg, m_hCheckResult[0]);

						if (i == m_param.m_nROINUM - 1)//表示全部检测完成,主站收到这个标志位会进行
						{
							m_hCheckResult[0]["ISFULL"] = 1;
						}

						CopyImg2SplicImg(m_DstImg, m_param.params[i].m_nCurGrabID, m_param.params[i].m_nGrabTimes);

						AnalyseCheckResult(i, m_hCheckResult);

					}
				}

				m_hcombineimg.ResetCol(j);
			}
		}
	}


	// 存缩略图，以及发送主站当前PACK检测完成 （缺一个packID 号）

	if (m_hcombineimg.IsAllFull())
	{
		m_hcombineimg.Clear();

		std::string sendstr = "1";
		if (Net::IsExistNetObj())
		{
			Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_CHECK_RESULT_FINISH), int(sendstr.size()), sendstr.c_str());
		}


		if (m_echeckstatus == _ONLINE_RUN)
		{
			//保存缩略图
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
			sprintf_s(suoluetu, "F:\\%d-%d-%d\\%s\\缩略图\\", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str());
			if (_access(suoluetu, 0))
				_mkdir(suoluetu);
			sprintf_s(suoluetu, "F:\\%d-%d-%d\\%s\\缩略图\\%d.bmp", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, m_sPackID.c_str(), nsaveindex);
			m_hBaseFun.SaveBMPImage_h(suoluetu, m_ImgSplicing);

			// 保存后清除缩略图
			ippsSet_8u(0, m_ImgSplicing.buf, m_ImgSplicing.nPitch * m_ImgSplicing.nHeight);
		}

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

	//if (bhascheck && (m_hCheckResult[0]["defect num"].asInt() != 0))
	//{
	//	//3. 分析结果，比如用表达式进行判废（这一步以前的同事设计的时候不把它放在主站的原因是因为耗时原因），这里是对所有区域进行汇合的判废
	//}


	//total_e = tick_count::now();
	//printf("check a image  %d: ----- cost : %f ms\n", card.m_CardID, (total_e - total_s).seconds() * 1000);
	
		
	tbb_end = tick_count::now();
	printf_s("----- id %d cost : %f ms\n", SrcCapImg.m_ImageID, (tbb_end - tbb_start).seconds() * 1000);



	return 1;
}




CKxCheck   g_CheckObj;

namespace Check
{

}