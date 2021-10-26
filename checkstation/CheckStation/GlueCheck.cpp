#include "stdafx.h"
#include "GlueCheck.h"
#include "global.h"
#include "KxReadXml2.h"


CGlueCheck::CGlueCheck()
{

}

CGlueCheck::~CGlueCheck()
{

}

bool CGlueCheck::ReadParamXml(const char* filePath, char *ErrorInfo, const char * templatepath)
{
	std::string szResult;
	int nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "检测设置", "提取异物灰度", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "提取异物灰度");
		return false;
	}

	int nStatus = KxXmlFun2::FromStringToInt(szResult, m_nthresh);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "检测设置", "异物最小点数", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "异物最小点数");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, m_nmindots);
	if (!nStatus)
	{
		return false;
	}


	return true;

}


void CGlueCheck::checkcolordiff(const kxCImageBuf& SrcImg)
{
	/*
	  涂胶颜色识别

	  思路一：	G通道与R通道差值，分块进行搜索，判断该块中值大于某个值的数量
		
	  思路二：  学习一块区域的高、低
	*/

	

}


int CGlueCheck::Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult)
{



	//checkresult["checkstatus"] = _Check_Err;// 我这里属于一棒子打死，后面要改的。
	//int nstartx = m_hWarpStrech.GetParameter().m_rcCheckArea.left;
	//int nstarty = m_hWarpStrech.GetParameter().m_rcCheckArea.top;
	//int nstartx = 0;
	//int nstarty = 0;
	//if (nStatus == 0)
	//{
	//	Json::Value single;
	//	single["Dots"] = 100000;
	//	single["Energy"] = 100000;
	//	single["pos"].append(nstartx);
	//	single["pos"].append(nstarty);
	//	single["pos"].append(m_hWarpStrech.GetParameter().m_rcCheckArea.Width());
	//	single["pos"].append(m_hWarpStrech.GetParameter().m_rcCheckArea.Height());
	//	//single["posX"] = m_hWarpStrech.GetParameter().m_rcCheckArea.left;
	//	//single["posY"] = m_hWarpStrech.GetParameter().m_rcCheckArea.top;
	//	//single["width"] = m_hWarpStrech.GetParameter().m_rcCheckArea.Width();
	//	//single["height"] = m_hWarpStrech.GetParameter().m_rcCheckArea.Height();
	//	checkresult["defect feature"].append(single);
	//	checkresult["defect num"] = 1;
	//}
	//else
	//{
	//	checkresult["defect num"] = m_hSurfaceCheck.GetCheckResult().m_nCount;
	//	for (int i = 0; i < m_hSurfaceCheck.GetCheckResult().m_nCount; i++)
	//	{
	//		Json::Value single;
	//		single["Dots"] = m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nDots;
	//		single["Energy"] = m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nEnergy;
	//		single["pos"].append(nstartx + m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nLeft);
	//		single["pos"].append(nstarty + m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nTop);
	//		single["pos"].append(m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobWidth);
	//		single["pos"].append(m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobHeight);
	//		//single["posX"] = m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nLeft;
	//		//single["posY"] = m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nTop;;
	//		//single["width"] = m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobWidth;
	//		//single["height"] = m_hSurfaceCheck.GetCheckResult().m_hBlobInfo[i].m_nBlobHeight;
	//		checkresult["defect feature"].append(single);
	//	}
	//}

	cv::Mat srcmat;

	m_hFun.KxImageBufToMat(SrcImg, srcmat, false);

	cv::split(srcmat, m_matarraybgr);

	m_hFun.MatToKxImageBuf(m_matarraybgr[2], m_ImgGray);

	m_hFun.KxThreshImage(m_ImgGray, m_ImgThresh, m_nthresh, 255);

	m_hFun.KxOpenImage(m_ImgThresh, m_ImgOpen, 11, 11);

	m_hBlobFun.ToBlobParallel(m_ImgOpen);

	checkresult["defect num"] = 0;

	if (m_hBlobFun.GetBlobCount() > 0)
	{
		for (int i = 0; i < m_hBlobFun.GetBlobCount(); i++)
		{
			CKxBlobAnalyse::SingleBlobInfo blobinfo;
			blobinfo = m_hBlobFun.GetSortSingleBlob(i);
			if (blobinfo.m_nDots > m_nmindots)
			{
				Json::Value single;
				single["Dots"] = blobinfo.m_nDots;
				single["Energy"] = 0;
				single["pos"].append(blobinfo.m_rc.left);
				single["pos"].append(blobinfo.m_rc.top);
				single["pos"].append(blobinfo.m_rc.Width());
				single["pos"].append(blobinfo.m_rc.Height());
				checkresult["defect feature"].append(single);
				checkresult["defect num"] = checkresult["defect num"].asInt() + 1;
			}
		}

	}

	DstImg = SrcImg;

	return 1;
}


