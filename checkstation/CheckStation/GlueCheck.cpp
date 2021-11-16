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
	/*std::string szResult;
	int nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "�������", "��ȡ����Ҷ�", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "��ȡ����Ҷ�");
		return false;
	}

	int nStatus = KxXmlFun2::FromStringToInt(szResult, m_nthresh);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun2::SearchXmlGetValue(filePath, "�������", "������С����", szResult);
	if (!nSearchStatus)
	{
		sprintf_s(ErrorInfo, 256, "������С����");
		return false;
	}

	nStatus = KxXmlFun2::FromStringToInt(szResult, m_nmindots);
	if (!nStatus)
	{
		return false;
	}*/


	return true;

}

void CGlueCheck::checkcolordiff(const kxCImageBuf& SrcImg)
{
	/*
	  Ϳ����ɫʶ��

	  ˼·һ��	Gͨ����Rͨ����ֵ���ֿ�����������жϸÿ���ֵ����ĳ��ֵ������
		
	  ˼·����  ѧϰһ������ĸߡ���

	  ˼·���� 	// �ÿ���ķ��������ɫ��

	*/

	cv::Mat img;

	m_hFun.KxImageBufToMat(SrcImg, img);
	
	cv::Mat hsv[3], bgr[3], hsvimg;

	cv::cvtColor(img, hsvimg, cv::COLOR_BGR2HSV);

	cv::split(img, bgr);

	cv::split(hsvimg, hsv);

	cv::Mat g_b, kernel, dilateimg, erodeimg;

	cv::subtract(bgr[1], bgr[0], g_b);

	kernel = cv::Mat(5, 5, CV_32SC1, cv::Scalar(1));

	cv::erode(g_b, erodeimg, kernel);

	kernel = cv::Mat(7, 7, CV_32SC1, cv::Scalar(1));

	cv::dilate(erodeimg, dilateimg, kernel);

	cv::Mat threshimg, threshimg1;

	cv::threshold(dilateimg, threshimg, 100, 255, cv::THRESH_OTSU);

	static int nindex = 0;

	char path[64];

	sprintf_s(path, "d:\\%d.bmp", nindex);

	cv::imwrite(path, threshimg);

	nindex++;

	//cv::threshold(dilateimg, threshimg1, 8, 255, cv::THRESH_BINARY);

}

void CGlueCheck::checkyiwu(const kxCImageBuf& SrcImg)
{
	//�ö�ֵ��֮���⣬һ����һ���ͣ�����һ����Χ
}

void CGlueCheck::checkqipao(const kxCImageBuf& SrcImg)
{
	// ������
}

void CGlueCheck::checkEdge(const kxCImageBuf& SrcImg)
{
	// ��������ȡ�����roi��Ȼ�����������������ұ�������




}

int CGlueCheck::Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult)
{



	//checkresult["checkstatus"] = _Check_Err;// ����������һ���Ӵ���������Ҫ�ĵġ�
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

	checkcolordiff(SrcImg);

	cv::Mat srcmat;

	m_hFun.KxImageBufToMat(SrcImg, srcmat, false);

	cv::split(srcmat, m_matarraybgr);

	m_hFun.MatToKxImageBuf(m_matarraybgr[2], m_ImgGray);

	IppiSize zerosize = {2000, m_ImgGray.nHeight};

	ippiSet_8u_C1R(0, m_ImgGray.buf, m_ImgGray.nPitch, zerosize);

	m_hFun.KxThreshImage(m_ImgGray, m_ImgThresh, m_param.m_ndefectthresh, 255);

	m_hFun.KxOpenImage(m_ImgThresh, m_ImgOpen, 11, 11);

	m_hBlobFun.ToBlobByCV(m_ImgOpen);

	checkresult["defect num"] = 0;

	if (m_hBlobFun.GetBlobCount() > 0)
	{
		for (int i = 0; i < m_hBlobFun.GetBlobCount(); i++)
		{
			CKxBlobAnalyse::SingleBlobInfo blobinfo;

			blobinfo = m_hBlobFun.GetSortSingleBlob(i);

			if (blobinfo.m_nDots > m_param.m_ndefectthresh)
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


