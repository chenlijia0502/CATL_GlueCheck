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
	}*/


	return true;

}

void CGlueCheck::checkcolordiff(const kxCImageBuf& SrcImg)
{
	/*
	  涂胶颜色识别

	  思路一：	G通道与R通道差值，分块进行搜索，判断该块中值大于某个值的数量
		
	  思路二：  学习一块区域的高、低

	  思路三： 	// 用库里的方法，检测色差

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
	//用二值化之后检测，一个高一个低，设置一个范围
}

void CGlueCheck::checkqipao(const kxCImageBuf& SrcImg)
{
	// 检测高亮
}

void CGlueCheck::checkEdge(const kxCImageBuf& SrcImg)
{
	//得到一个空心边缘

	cv::Mat matMask = cv::Mat(m_ImgGlueMask.nHeight, m_ImgGlueMask.nWidth, CV_8UC1, m_ImgGlueMask.buf);

	cv::Rect outrect = cv::boundingRect(matMask);

	m_ImgR_Mask.Init(matMask.cols, matMask.rows);

	IppiSize imgsize = { m_ImgR_Mask.nWidth, m_ImgR_Mask.nHeight};

	ippiSub_8u_C1RSfs(m_ImgGlueMask.buf, m_ImgGlueMask.nPitch, m_ImgRGB[0].buf, m_ImgRGB[0].nPitch, m_ImgR_Mask.buf, m_ImgR_Mask.nPitch, imgsize, 0);

	const int ncstep = 10;//N个点搜一次边

	const int nthresh = 18;


	//  搜索方法统一为先搜到蓝胶的，再往蓝胶另一侧搜索
	
	//搜左侧


	int nleftpoints = outrect.height / ncstep;

	int *pleftpos_edge = new int[nleftpoints];

	int *pleftpos_gule = new int[nleftpoints];


	int i = 0;

	for (int y = outrect.y; y < outrect.y + outrect.height; y+=ncstep)
	{
		pleftpos_gule[i] = outrect.x;

		for (int x = outrect.x; x < m_ImgGlueMask.nWidth; x++)
		{
			if (m_ImgGlueMask.buf[y*m_ImgGlueMask.nPitch + x] == 255)
			{
				pleftpos_gule[i] = x;

				break;
			}
		}

		i++;
	}

	i = 0;

	const int noffset = 5;// 胶体周边会有反光，影响搜边

	for (int y = outrect.y; y < outrect.y + outrect.height; y += ncstep)
	{
		pleftpos_edge[i] = gMax(pleftpos_gule[i] - noffset, 0);

		for (int x = pleftpos_edge[i]; x >= 0; x--)
		{
			if (m_ImgR_Mask.buf[y*m_ImgR_Mask.nPitch + x] >= nthresh)
			{
				pleftpos_edge[i] = x;

				break;
			}
		}

		i++;

	}


	delete[] pleftpos_edge;

	delete[] pleftpos_gule;

	// 搜右侧

	// 搜上

	// 搜下

	//RANSAC拟合直线，计算误差





}

void CGlueCheck::GetGlueMask()
{

	//提取涂胶掩膜
	IppiSize imgsize = { m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight};

	m_ImgThresh.Init(m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight);

	m_ImgG_R.Init(m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight);

	m_ImgG_B.Init(m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight);

	m_ImgsubResult.Init(m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight);

	ippiSub_8u_C1RSfs(m_ImgRGB[0].buf, m_ImgRGB[0].nPitch, m_ImgRGB[1].buf, m_ImgRGB[1].nPitch, m_ImgG_R.buf, m_ImgG_R.nPitch, imgsize, 0);

	ippiSub_8u_C1RSfs(m_ImgRGB[2].buf, m_ImgRGB[2].nPitch, m_ImgRGB[1].buf, m_ImgRGB[1].nPitch, m_ImgG_B.buf, m_ImgG_B.nPitch, imgsize, 0);

	ippiAdd_8u_C1RSfs(m_ImgG_R.buf, m_ImgG_R.nPitch, m_ImgG_B.buf, m_ImgG_B.nPitch, m_ImgsubResult.buf, m_ImgsubResult.nPitch, imgsize, 0);

	m_hAlg.ThreshImg(m_ImgsubResult, m_ImgThresh, 50, CEmpiricaAlgorithm::_BINARY);

	m_hFun.KxCloseImage(m_ImgThresh, m_ImgClose, 11, 11);

	m_hFun.KxOpenImage(m_ImgClose, m_ImgOpen, 11, 11);
	
	m_hBlobFun.SelectMaxRegionByDots(m_ImgOpen, m_ImgmaxRegion);

	//这里只是针对颜色的提取，必须加一层外部提取，把孔闭了

	m_hAlg.FillHoles(m_ImgmaxRegion, m_ImgGlueMask);
}

int CGlueCheck::Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult)
{


	/*
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
	*/

	m_hAlg.SplitRGB(SrcImg, m_ImgRGB);

	GetGlueMask();

	//checkEdge(SrcImg);

	/*
	kxCImageBuf colorimg, andimg, otherimg;

	colorimg.Init(m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight, 3);

	andimg.Init(m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight, 3);

	otherimg.Init(m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight, 3);

	IppiSize imgsize = { m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight };

	ippiCopy_8u_C1C3R(m_ImgGlueMask.buf, m_ImgGlueMask.nPitch, colorimg.buf, colorimg.nPitch, imgsize);

	ippiSub_8u_C3RSfs(colorimg.buf, colorimg.nPitch, SrcImg.buf, SrcImg.nPitch, otherimg.buf, otherimg.nPitch, imgsize, 0);

	ippiSub_8u_C3RSfs(otherimg.buf, otherimg.nPitch, SrcImg.buf, SrcImg.nPitch, andimg.buf, andimg.nPitch, imgsize, 0);

	*/
	DstImg = SrcImg;
	 
	return 1;
}


