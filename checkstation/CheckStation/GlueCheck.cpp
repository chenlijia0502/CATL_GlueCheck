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


void CGlueCheck::checkyiwu(const kxCImageBuf& SrcImg, Json::Value &checkresult)
{
	//1，创建基础模板
	CreateBaseModel(SrcImg);

	IppiSize imgsize = { SrcImg.nWidth, SrcImg.nHeight };

	kxCImageBuf CheckImg;

	CheckImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	ippiAnd_8u_C3R(SrcImg.buf, SrcImg.nPitch, m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, CheckImg.buf, CheckImg.nPitch, imgsize);

	// 2. 读取灵敏度
	unsigned char poffsetlow[3] = { m_param.m_noffsetlow, m_param.m_noffsetlow, m_param.m_noffsetlow };// 从模板读，RGB

	unsigned char poffsethigh[3] = { m_param.m_noffsethigh, m_param.m_noffsethigh, m_param.m_noffsethigh };// 从模板读，RGB

	//3，得到低值
	m_ImgCheckLow.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);

	ippiSub_8u_C3RSfs(CheckImg.buf, CheckImg.nPitch, m_ImgBaseTemplate.buf, m_ImgBaseTemplate.nPitch, m_ImgCheckLow.buf, m_ImgCheckLow.nPitch, imgsize, 0);

	ippiSubC_8u_C3IRSfs(poffsetlow, m_ImgCheckLow.buf, m_ImgCheckLow.nPitch, imgsize, 0);

	//4, 得到高值
	m_ImgCheckHigh.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);

	ippiSub_8u_C3RSfs(m_ImgBaseTemplate.buf, m_ImgBaseTemplate.nPitch, CheckImg.buf, CheckImg.nPitch, m_ImgCheckHigh.buf, m_ImgCheckHigh.nPitch, imgsize, 0);

	ippiSubC_8u_C3IRSfs(poffsethigh, m_ImgCheckHigh.buf, m_ImgCheckHigh.nPitch, imgsize, 0);


	// 彩色图转换为灰度图,取最大
	m_ImgCheckHighGray.Init(m_ImgCheckHigh.nWidth, m_ImgCheckHigh.nHeight);

	m_ImgCheckLowGray.Init(m_ImgCheckLow.nWidth, m_ImgCheckLow.nHeight);

	GetMaxGray(m_ImgCheckHigh, m_ImgCheckHighGray);
	
	GetMaxGray(m_ImgCheckLow, m_ImgCheckLowGray);

	m_hFun.KxAddImage(m_ImgCheckLowGray, m_ImgCheckHighGray);

	kxCImageBuf checkopenimg;

	checkopenimg.Init(m_ImgCheckHighGray.nWidth, m_ImgCheckHighGray.nHeight, m_ImgCheckHighGray.nChannel);

	m_hFun.KxOpenImage(m_ImgCheckHighGray, checkopenimg, 1, 7);


	CutImg2MulImg(checkopenimg);

	ParallelBlob(checkresult);
}


void CGlueCheck::ParallelBlob(Json::Value &checkresult)
{
	parallel_for(blocked_range<int>(0, m_nblobimgnum),
	[&](const blocked_range<int>& range)
	{
		for (int index = range.begin(); index != range.end(); index++)
		{
			m_phblob[index].ToBlobByCV(m_pBLOBIMG[index], CKxBlobAnalyse::_SORT_BYDOTS, 100);
		}
	}, auto_partitioner());

	checkresult["defect num"] = 0;

	int nsingleimgstep = _SINGLE_BLOBIMG_H - _IMG_OVERLAP;// 将分图坐标转换成全局坐标

	int nblobindex = 0;

	for (int nimgindex = 0; nimgindex < m_nblobimgnum; nimgindex++)
	{
		if (m_phblob[nimgindex].GetBlobCount() > 0)
		{
			for (int i = 0; i < m_phblob[nimgindex].GetBlobCount(); i++)
			{
				CKxBlobAnalyse::SingleBlobInfo blobinfo;

				blobinfo = m_phblob[nimgindex].GetSortSingleBlob(i);

				if (blobinfo.m_nDots > m_param.m_ndefectdots)
				{
					Json::Value single;
					single["Dots"] = blobinfo.m_nDots;
					single["Energy"] = 0;
					char defectid[32];
					sprintf_s(defectid, "%d_%d", m_nID, nblobindex);
					std::cout << defectid << std::endl;
					nblobindex++;
					single["defectid"] = defectid;
					single["pos"].append(blobinfo.m_rc.left);
					single["pos"].append(blobinfo.m_rc.top + nsingleimgstep * nimgindex);
					single["pos"].append(blobinfo.m_rc.Width());
					single["pos"].append(blobinfo.m_rc.Height());
					checkresult["defect feature"].append(single);
					checkresult["defect num"] = checkresult["defect num"].asInt() + 1;
				}
			}
		}
	}


	if (m_param.m_nGrabTimes == 0 && m_param.m_nCurGrabID == 0)
	{
		Json::Value single;
		single["Dots"] = 200;
		single["Energy"] = 0;
		char defectid[32];
		sprintf_s(defectid, "%d_%d", m_nID, nblobindex);
		std::cout << defectid << std::endl;
		nblobindex++;
		single["defectid"] = defectid;
		single["pos"].append(2720);
		single["pos"].append(2990);
		single["pos"].append(400);
		single["pos"].append(200);
		checkresult["defect feature"].append(single);
		checkresult["defect num"] = checkresult["defect num"].asInt() + 1;
	}


	std::cout << " 第 " << m_nID << " 检测到缺陷数量： " << checkresult["defect num"].asInt() << std::endl;

}


void CGlueCheck::CreateBaseModel(const kxCImageBuf& CheckImg)
{
	
	/* 方案一：  设定一个基础范围，检测高低


	// 1. 以下的值后面从主界面设置获取
	


	const unsigned char Rrange[2] = { 10, 40 };

	const unsigned char Grange[2] = { 20, 60 };

	const unsigned char Brange[2] = { 10, 50 };


	// 2.赋值上下限图

	kxCImageBuf ImgR[2], ImgB[2], ImgG[2];
	
	for (int i = 0; i < 2; i++)
	{
		ImgR[i].Init(CheckImg.nWidth, CheckImg.nHeight);

		ImgG[i].Init(CheckImg.nWidth, CheckImg.nHeight);

		ImgB[i].Init(CheckImg.nWidth, CheckImg.nHeight);

		ippsSet_8u(Rrange[i], ImgR[i].buf, ImgR[i].nPitch * ImgR[i].nHeight);

		ippsSet_8u(Grange[i], ImgG[i].buf, ImgG[i].nPitch * ImgG[i].nHeight);

		ippsSet_8u(Brange[i], ImgB[i].buf, ImgB[i].nPitch * ImgB[i].nHeight);
		
	}

	unsigned char * pbuf1[3] = { ImgR[0].buf,  ImgG[0].buf, ImgB[0].buf};

	unsigned char * pbuf2[3] = { ImgR[1].buf,  ImgG[1].buf, ImgB[1].buf };

	m_ImgTemplateLow.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);

	m_ImgTemplateHigh.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);

	IppiSize imgsize = { m_ImgTemplateLow.nWidth, m_ImgTemplateLow.nHeight};

	ippiCopy_8u_P3C3R(pbuf1, ImgR[0].nPitch, m_ImgTemplateLow.buf, m_ImgTemplateLow.nPitch, imgsize);

	ippiCopy_8u_P3C3R(pbuf2, ImgR[1].nPitch, m_ImgTemplateHigh.buf, m_ImgTemplateHigh.nPitch, imgsize);

	*/

	/*
		方案二：设定基础色，然后残差结果减去灵敏度
	*/
	
	const unsigned char Rbase = 30;

	const unsigned char Gbase = 50;

	const unsigned char Bbase = 20;

	kxCImageBuf ImgR, ImgB, ImgG;

	ImgR.Init(CheckImg.nWidth, CheckImg.nHeight);

	ImgG.Init(CheckImg.nWidth, CheckImg.nHeight);

	ImgB.Init(CheckImg.nWidth, CheckImg.nHeight);

	ippsSet_8u(Rbase, ImgR.buf, ImgR.nPitch * ImgR.nHeight);

	ippsSet_8u(Gbase, ImgG.buf, ImgG.nPitch * ImgG.nHeight);

	ippsSet_8u(Bbase, ImgB.buf, ImgB.nPitch * ImgB.nHeight);

	unsigned char * pbuf1[3] = { ImgR.buf,  ImgG.buf, ImgB.buf };

	m_ImgBaseTemplate.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);

	IppiSize imgsize = { m_ImgBaseTemplate.nWidth, m_ImgBaseTemplate.nHeight };

	ippiCopy_8u_P3C3R(pbuf1, ImgR.nPitch, m_ImgBaseTemplate.buf, m_ImgBaseTemplate.nPitch, imgsize);

	ippiAnd_8u_C3IR(m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, m_ImgBaseTemplate.buf, m_ImgBaseTemplate.nPitch, imgsize);

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


void CGlueCheck::SearchEdge(const kxCImageBuf& GrayImg, int ndir, int nThreshvalue, int& nedge1, int& nedge2)
{

	const int nContinuityTimes = 10;//TODO: 这个值需要留意，看是否需要开放成接口进行设置，万一有的边框无法高亮

	cv::Mat projectimg;

	if (ndir == _Vertical_Project_Dir)//投影的缩放系数是检测框的大小，原因是GrayImg是一张统一尺寸的大图，真正的检测区域只有m_rcCheckROI的大小
	{
		m_hAlg.CreatProjectImg(GrayImg, projectimg, ndir, m_param.m_rcCheckROI.Height());
	}
	else
	{
		m_hAlg.CreatProjectImg(GrayImg, projectimg, ndir, GrayImg.nWidth);//宽度是全拷贝
	}


	//----------------临时插入测试-------------------------//

	//cv::Mat project16s = cv::Mat(projectimg.size(), CV_16SC1);

	//IppiSize copysize = { projectimg.cols, projectimg.rows };
	//ippiConvert_32f16s_C1R((Ipp32f*)projectimg.data, projectimg.step, (Ipp16s*)project16s.data, project16s.step, copysize, ippRndZero);
	//
	//cv::Mat sumwindow32f = cv::Mat(projectimg.size(), CV_32FC1);
	//ippsSumWindow_16s32f((Ipp16s*)project16s.data, (Ipp32f*)sumwindow32f.data, sumwindow32f.cols, 100);


	//------------------------------------------------------//




	int ntimes = 0;

	nedge1 = 0;

	nedge2 = projectimg.cols - 1;

	for (int i = projectimg.cols / 2; i > 0; i--)
	{
		if (projectimg.at<float>(0, i) > nThreshvalue)
		{
			ntimes++;

			if (ntimes > nContinuityTimes)
			{
				nedge1 = i + ntimes;
			
				break;
			}
		}
		else
		{
			ntimes = 0;
		}
	}

	for (int i = projectimg.cols / 2; i < projectimg.cols; i++)
	{
		if (projectimg.at<float>(0, i) > nThreshvalue)
		{
			ntimes++;

			if (ntimes > nContinuityTimes)
			{
				nedge2 = i - ntimes;
				
				break;
			}
		}
		else
		{
			ntimes = 0;
		}
	}




}


void CGlueCheck::GetEdgePoints(const kxCImageBuf& GrayImg, int nDir, int nthresh, kxPoint<int> * searchresult, int nnums)
{

	kxCImageBuf filterimg, blurimg;

	filterimg.Init(GrayImg.nWidth, GrayImg.nHeight);

	blurimg.Init(GrayImg.nWidth, GrayImg.nHeight);


	if (nDir == 0)
	{
		Ipp16s pMasks[5] = {-1, 0, 0, 0, 1};

		m_hFun.KxGeneralFilterImage8u(GrayImg, filterimg, 5, 1, pMasks);

		m_hFun.KxParallelBoxFilterImage(filterimg, blurimg, 1, 7);

		int nstep = blurimg.nHeight / nnums;

		for (int i = 0; i < nnums; i++)
		{
			int y = i * nstep;

			searchresult[i].x = 0;

			searchresult[i].y = y;

			for (int x = 0; x < blurimg.nWidth; x++)
			{
				if (blurimg.buf[y * blurimg.nPitch + x] > nthresh)
				{
					searchresult[i].x = x;

					searchresult[i].y = y;

					break;
				}
			}
		}



	}
	else
	{
		Ipp16s pMasks[5] = { 1, 0, 0, 0, -1 };

		m_hFun.KxGeneralFilterImage8u(GrayImg, filterimg, 5, 1, pMasks);

		m_hFun.KxParallelBoxFilterImage(filterimg, blurimg, 1, 7);

		int nstep = blurimg.nHeight / nnums;

		for (int i = 0; i < nnums; i++)
		{
			int y = i * nstep;

			searchresult[i].x = 0;

			searchresult[i].y = y;

			for (int x = blurimg.nWidth - 1; x >= 0; x--)
			{
				if (blurimg.buf[y * blurimg.nPitch + x] > nthresh)
				{
					searchresult[i].x = x;

					searchresult[i].y = y;

					break;
				}
			}
		}
	}

}

void CGlueCheck::GetGlueMask(const kxCImageBuf* RGB)
{


	/* 方案一: 2G - R - B 结果二值化，提取最大blob

	//提取涂胶掩膜
	IppiSize imgsize = { m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight};

	m_ImgThresh.Init(m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight);

	m_ImgG_R.Init(m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight);

	m_ImgG_B.Init(m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight);

	m_ImgsubResult.Init(m_ImgRGB[2].nWidth, m_ImgRGB[2].nHeight);

	ippiSub_8u_C1RSfs(m_ImgRGB[0].buf, m_ImgRGB[0].nPitch, m_ImgRGB[1].buf, m_ImgRGB[1].nPitch, m_ImgG_R.buf, m_ImgG_R.nPitch, imgsize, 0);

	ippiSub_8u_C1RSfs(m_ImgRGB[2].buf, m_ImgRGB[2].nPitch, m_ImgRGB[1].buf, m_ImgRGB[1].nPitch, m_ImgG_B.buf, m_ImgG_B.nPitch, imgsize, 0);

	ippiAdd_8u_C1RSfs(m_ImgG_R.buf, m_ImgG_R.nPitch, m_ImgG_B.buf, m_ImgG_B.nPitch, m_ImgsubResult.buf, m_ImgsubResult.nPitch, imgsize, 0);

	m_hAlg.ThreshImg(m_ImgsubResult, m_ImgThresh, 40, CEmpiricaAlgorithm::_BINARY);

	m_hFun.KxCloseImage(m_ImgThresh, m_ImgClose, 11, 11);

	m_hFun.KxOpenImage(m_ImgClose, m_ImgOpen, 11, 11);
	
	m_hBlobFun.SelectMaxRegionByDots(m_ImgOpen, m_ImgmaxRegion);

	*/
	


	//1. 提取亮度较高的边框, 根据灰度投影得出

	int ntop, nright, nbottom, nleft;

	SearchEdge(RGB[0], _Vertical_Project_Dir, 180, nleft, nright);

	SearchEdge(RGB[0], _Horizontal_Project_Dir, 150, ntop, nbottom);

	
	//2. 得到较高的边缘后，对左右两侧进行精定位

	//(1) 裁剪目标区域
	kxCImageBuf edgeimg1, edgeimg2;

	const int noffset = 500;

	int edgeright = min(nleft + noffset, RGB[1].nWidth - 1);

	int edgeleft = max(nright - noffset, 0);

	IppiSize cutsize = {edgeright - nleft + 1, RGB[1].nHeight};

	edgeimg1.Init(cutsize.width, cutsize.height);

	ippiCopy_8u_C1R(RGB[1].buf + nleft, RGB[1].nPitch, edgeimg1.buf, edgeimg1.nPitch, cutsize);

	cutsize = {nright - edgeleft + 1, RGB[1].nHeight };
	
	edgeimg2.Init(cutsize.width, cutsize.height);

	ippiCopy_8u_C1R(RGB[1].buf + edgeleft, RGB[1].nPitch, edgeimg2.buf, edgeimg2.nPitch, cutsize);

	// (2) 搜点
	const int npointnum = 20;

	kxPoint<int> leftpoint[npointnum];

	kxPoint<int> rightpoint[npointnum];

	GetEdgePoints(edgeimg1, 1, 10, leftpoint, npointnum);

	GetEdgePoints(edgeimg2, 0, 10, rightpoint, npointnum);
	 
	// (3) 针对搜到的点进行拟合直线，然后取一个y的中间值得到x值是多少（目前这里直接取平均值）

	int naverageleft = 0;

	int naverageright = 0;
	
	for (int i = 0; i < npointnum; i++)
	{
		naverageleft += leftpoint[i].x;

		naverageright += rightpoint[i].x;
	}

	naverageleft = naverageleft / npointnum;

	naverageright = edgeimg2.nWidth - naverageright / npointnum;



	int hoffset = 0;

	nleft = min(nleft + naverageleft, RGB[0].nWidth / 2);

	nright = max(nright - naverageright, RGB[0].nWidth / 2);

	ntop = min(ntop + hoffset, RGB[0].nHeight / 2);

	nbottom = max(nbottom - hoffset, RGB[0].nHeight / 2);


	//2. 根据前一步骤提取的框内，进行绿色分割
	m_recttarget.setup(nleft, ntop, nright, nbottom);

	//kxCImageBuf cutrgb[3];
	//for (int i = 0; i < 3; i++)
	//{
	//	// 初始化，剪切，调用
	//	cutrgb[i].Init(targetroi.Width(), targetroi.Height());
	//	
	//	m_hFun.KxCopyImage(RGB[i], cutrgb[i], targetroi);
	//}
	//SliderMatch(cutrgb, m_param.m_ImgTemplate);
	//

	m_ImgGlueMask.Init(RGB[0].nWidth, RGB[0].nHeight, RGB[0].nChannel);

	ippsSet_8u(0, m_ImgGlueMask.buf, m_ImgGlueMask.nPitch * m_ImgGlueMask.nHeight);

	IppiSize copysize = { nright - nleft + 1, nbottom - ntop + 1 };

	ippiSet_8u_C1R(255, m_ImgGlueMask.buf + m_ImgGlueMask.nPitch * ntop + nleft, m_ImgGlueMask.nPitch, copysize);


	//肇庆方案
	//ExtractGreen(RGB, targetroi, m_ImgGlueMask);

	/*
	//2. 对边框内图案进行提取
	
	m_hAlg.ThreshImg(m_ImgHSV[1], m_ImgThresh, 95, CEmpiricaAlgorithm::_BINARY);
	
	m_hFun.KxOpenImage(m_ImgThresh, m_ImgOpen, 11, 11);

	m_hFun.KxCloseImage(m_ImgOpen, m_ImgClose, 11, 11);

	m_hBlobFun.SelectMaxRegionByDots(m_ImgClose, m_ImgmaxRegion);


	//这里只是针对颜色的提取，必须加一层外部提取，把孔闭了

	m_hAlg.FillHoles(m_ImgmaxRegion, m_ImgGlueMask);

	//最下面那段颜色不行，所以裁剪掉 2021.12.16 后面要换掉

	cv::Mat srcimg = cv::Mat(m_ImgGlueMask.nHeight, m_ImgGlueMask.nWidth, CV_8UC1, m_ImgGlueMask.buf);

	cv::Rect rect = cv::boundingRect(srcimg);

	int x = rect.x + rect.width / 10;

	int y = rect.y + rect.height / 10;

	int width = rect.width / 10 * 8;

	int height = rect.height / 10 * 8;

	cv::Mat cutimg = srcimg(cv::Rect(x, y, width, height)).clone();

	ippsSet_8u(0, srcimg.data, srcimg.rows * srcimg.step);

	IppiSize copysize = { cutimg.cols, cutimg.rows };

	ippiCopy_8u_C1R(cutimg.data, cutimg.step, srcimg.data + x + srcimg.step * y, srcimg.step, copysize);
	*/


	m_ImgColorGlueMask.Init(m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight, 3);

	unsigned char *pbuf[3] = { m_ImgGlueMask.buf , m_ImgGlueMask.buf , m_ImgGlueMask.buf };

	IppiSize imgsize = { m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight };

	ippiCopy_8u_P3C3R(pbuf, m_ImgGlueMask.nPitch, m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, imgsize);


}

//int CGlueCheck::Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult)
//{
//
//
//	/*
//	checkcolordiff(SrcImg);
//
//	cv::Mat srcmat;
//
//	m_hFun.KxImageBufToMat(SrcImg, srcmat, false);
//
//	cv::split(srcmat, m_matarraybgr);
//
//	m_hFun.MatToKxImageBuf(m_matarraybgr[2], m_ImgGray);
//
//	IppiSize zerosize = {2000, m_ImgGray.nHeight};
//
//	ippiSet_8u_C1R(0, m_ImgGray.buf, m_ImgGray.nPitch, zerosize);
//
//	m_hFun.KxThreshImage(m_ImgGray, m_ImgThresh, m_param.m_ndefectthresh, 255);
//
//	m_hFun.KxOpenImage(m_ImgThresh, m_ImgOpen, 11, 11);
//
//	m_hBlobFun.ToBlobByCV(m_ImgOpen);
//
//	checkresult["defect num"] = 0;
//
//	if (m_hBlobFun.GetBlobCount() > 0)
//	{
//		for (int i = 0; i < m_hBlobFun.GetBlobCount(); i++)
//		{
//			CKxBlobAnalyse::SingleBlobInfo blobinfo;
//
//			blobinfo = m_hBlobFun.GetSortSingleBlob(i);
//
//			if (blobinfo.m_nDots > m_param.m_ndefectthresh)
//			{
//				Json::Value single;
//				single["Dots"] = blobinfo.m_nDots;
//				single["Energy"] = 0;
//				single["pos"].append(blobinfo.m_rc.left);
//				single["pos"].append(blobinfo.m_rc.top);
//				single["pos"].append(blobinfo.m_rc.Width());
//				single["pos"].append(blobinfo.m_rc.Height());
//				checkresult["defect feature"].append(single);
//				checkresult["defect num"] = checkresult["defect num"].asInt() + 1;
//			}
//		}
//
//	}
//	*/
//
//	m_hAlg.SplitRGB(SrcImg, m_ImgRGB);
//
//	GetGlueMask();
//
//	//checkEdge(SrcImg);
//
//	/*
//	kxCImageBuf colorimg, andimg, otherimg;
//
//	colorimg.Init(m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight, 3);
//
//	andimg.Init(m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight, 3);
//
//	otherimg.Init(m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight, 3);
//
//	IppiSize imgsize = { m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight };
//
//	ippiCopy_8u_C1C3R(m_ImgGlueMask.buf, m_ImgGlueMask.nPitch, colorimg.buf, colorimg.nPitch, imgsize);
//
//	ippiSub_8u_C3RSfs(colorimg.buf, colorimg.nPitch, SrcImg.buf, SrcImg.nPitch, otherimg.buf, otherimg.nPitch, imgsize, 0);
//
//	ippiSub_8u_C3RSfs(otherimg.buf, otherimg.nPitch, SrcImg.buf, SrcImg.nPitch, andimg.buf, andimg.nPitch, imgsize, 0);
//
//	*/
//	DstImg = SrcImg;
//	 
//	return 1;
//}

void CGlueCheck::GetMaxGray(const kxCImageBuf& ColorImg, kxCImageBuf& DstGrayImg)
{
	kxCImageBuf gray[3];
	
	m_hAlg.SplitRGB(ColorImg, gray);

	m_hFun.KxMaxEvery(gray[0], gray[1]);

	m_hFun.KxMaxEvery(gray[1], gray[2]);

	DstGrayImg.SetImageBuf(gray[2], true);
}


void CGlueCheck::MergeImg(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg)
{
	kxCImageBuf gray1, threshimg, dilateimg;

	gray1.Init(SrcImgA.nWidth, SrcImgA.nHeight);

	dilateimg.Init(SrcImgA.nWidth, SrcImgA.nHeight);

	IppiSize imgsize = { SrcImgA.nWidth, SrcImgA.nHeight };

	//取srcimgA里RGB最大值
	GetMaxGray(SrcImgA, gray1);

	m_hAlg.ThreshImg(gray1, threshimg, 100, CEmpiricaAlgorithm::_BINARY);

	dilateimg.Init(threshimg.nWidth, threshimg.nHeight);

	m_hFun.KxDilateImage(threshimg, dilateimg, 5, 5);

	threshimg.SetImageBuf(dilateimg);

	kxCImageBuf img1, img2;

	img1.Init(threshimg.nWidth, threshimg.nHeight, 3);

	img2.Init(threshimg.nWidth, threshimg.nHeight, 3);

	kxCImageBuf guobaomaskimg;

	guobaomaskimg.Init(threshimg.nWidth, threshimg.nHeight, 3);

	unsigned char* pbuf[3] = { threshimg.buf, threshimg.buf, threshimg.buf};

	ippiCopy_8u_P3C3R(pbuf, threshimg.nPitch, guobaomaskimg.buf, guobaomaskimg.nPitch, imgsize);

	// A - mask 
	ippiSub_8u_C3RSfs(guobaomaskimg.buf, guobaomaskimg.nPitch, SrcImgA.buf, SrcImgA.nPitch, img1.buf, img1.nPitch, imgsize, 0);

	//B 与 mask相与
	ippiAnd_8u_C3R(guobaomaskimg.buf, guobaomaskimg.nPitch, SrcImgB.buf, SrcImgB.nPitch, img2.buf, img2.nPitch, imgsize);

	DstImg.Init(threshimg.nWidth, threshimg.nHeight, 3);

	ippiAdd_8u_C3RSfs(img1.buf, img1.nPitch, img2.buf, img2.nPitch, DstImg.buf, DstImg.nPitch, imgsize, 0);

	ippiAnd_8u_C3IR(m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, DstImg.buf, DstImg.nPitch, imgsize);


}


void CGlueCheck::CutImg2MulImg(const kxCImageBuf& CheckImg)
{
	/*
		将一张大图分成N张有重叠区域的图，方便进行blob分析
	*/

	// -1 后 +1是向上取整
	m_nblobimgnum = int((CheckImg.nHeight - _IMG_OVERLAP - 1) / (_SINGLE_BLOBIMG_H - _IMG_OVERLAP)) + 1;// 向上取整

	if (m_nblobimgnum > _MAX_BLOBIMG)
	{
		printf_s("roi框的大小超过设置的参数！！！！");
		// KXPRINTF(); 先不做处理
		m_nblobimgnum = _MAX_BLOBIMG;
	}

	int nh2copy = (_SINGLE_BLOBIMG_H - _IMG_OVERLAP) * m_nblobimgnum + _IMG_OVERLAP;


	m_ImgZero2split.Init(CheckImg.nWidth, nh2copy, 1);

	ippsSet_8u(0, m_ImgZero2split.buf, m_ImgZero2split.nPitch * m_ImgZero2split.nHeight);

	IppiSize copysize = {CheckImg.nWidth, CheckImg.nHeight};

	ippiCopy_8u_C1R(CheckImg.buf, CheckImg.nPitch, m_ImgZero2split.buf, m_ImgZero2split.nPitch, copysize);

	for (int i = 0; i < m_nblobimgnum; i++)
	{
		unsigned char *copybuf = m_ImgZero2split.buf + i * (_SINGLE_BLOBIMG_H - _IMG_OVERLAP) * m_ImgZero2split.nPitch;

		m_pBLOBIMG[i].Init(CheckImg.nWidth, _SINGLE_BLOBIMG_H);

		IppiSize smallsize = { CheckImg.nWidth, _SINGLE_BLOBIMG_H };

		ippiCopy_8u_C1R(copybuf, m_ImgZero2split.nPitch, m_pBLOBIMG[i].buf, m_pBLOBIMG[i].nPitch, smallsize);
	}
}

void CGlueCheck::ExtractGreen(const kxCImageBuf* RGB, kxRect<int> roi, kxCImageBuf& DstImg)
{
	
	// 1. 将图像转换为32f
	kxCImageBuf dilateR;
	
	dilateR.Init(RGB[2].nWidth, RGB[2].nHeight);

	m_hFun.KxDilateImage(RGB[2], dilateR, 3, 3);

	IppiSize imgsize = { RGB[2].nWidth, RGB[2].nHeight};

	cv::Mat convertimgR = cv::Mat(RGB[2].nHeight, RGB[2].nWidth, CV_32FC1);

	ippiConvert_8u32f_C1R(dilateR.buf, dilateR.nPitch, (Ipp32f*)convertimgR.data, convertimgR.step, imgsize);

	cv::Mat convertimgR32f = cv::Mat(RGB[2].nHeight, RGB[2].nWidth, CV_32FC1);

	ippiDivC_32f_C1R((Ipp32f*)convertimgR.data, convertimgR.step, 255, (Ipp32f*)convertimgR32f.data, convertimgR32f.step, imgsize);

	cv::Mat convertimgG = cv::Mat(RGB[1].nHeight, RGB[1].nWidth, CV_32FC1);

	ippiConvert_8u32f_C1R(RGB[1].buf, RGB[1].nPitch, (Ipp32f*)convertimgG.data, convertimgG.step, imgsize);

	cv::Mat convertimgG32f = cv::Mat(RGB[1].nHeight, RGB[1].nWidth, CV_32FC1);

	ippiDivC_32f_C1R((Ipp32f*)convertimgG.data, convertimgG.step,  255, (Ipp32f*)convertimgG32f.data, convertimgG.step, imgsize);


	// 2. G - R
	cv::Mat submat32f = cv::Mat(RGB[1].nHeight, RGB[1].nWidth, CV_32FC1);

	ippiSub_32f_C1R((Ipp32f*)convertimgR32f.data, convertimgR32f.step, (Ipp32f*)convertimgG32f.data, convertimgG32f.step, (Ipp32f*)submat32f.data, submat32f.step, imgsize);


	// 3. 获取最大最小值，转化为 0-255（注意这个时候最小值可能是负的）
	double minval = 0;

	double maxval = 0;

	cv::minMaxLoc(submat32f, &minval, &maxval);

	// 4.(val - minval) / (maxval - minval)  * 255 
	if (abs(minval - maxval) < 1e-6)
	{
		// TODO 异常处理，赋值dstimg并返回
	}

	double factor = 255.0 / (maxval - minval);

	cv::Mat subcresult = cv::Mat(RGB[1].nHeight, RGB[1].nWidth, CV_32FC1);

	ippiSubC_32f_C1R((Ipp32f*)submat32f.data, submat32f.step, minval, (Ipp32f*)subcresult.data, subcresult.step, imgsize);

	cv::Mat divresult = cv::Mat(RGB[1].nHeight, RGB[1].nWidth, CV_32FC1);

	ippiMulC_32f_C1R((Ipp32f*)subcresult.data, subcresult.step, factor, (Ipp32f*)divresult.data, divresult.step, imgsize);

	cv::Mat gray = cv::Mat(RGB[1].nHeight, RGB[1].nWidth, CV_8UC1);

	cv::convertScaleAbs(divresult, gray);

	cv::Mat threshimg;

	cv::threshold(gray, threshimg, 130, 255, cv::THRESH_BINARY);

	DstImg.Init(threshimg.cols, threshimg.rows);

	ippiSet_8u_C1R(0, DstImg.buf, DstImg.nPitch, imgsize);

	IppiSize cutsize = {roi.Width(), roi.Height()};

	kxCImageBuf solveresult, openimg, closeimg;

	solveresult.Init(threshimg.cols, threshimg.rows);
	openimg.Init(threshimg.cols, threshimg.rows);
	closeimg.Init(threshimg.cols, threshimg.rows);


	ippiSet_8u_C1R(0, solveresult.buf, solveresult.nPitch, imgsize);


	ippiCopy_8u_C1R(threshimg.data + roi.left + roi.top * threshimg.step, threshimg.step, solveresult.buf + roi.left + roi.top * solveresult.nPitch, solveresult.nPitch, cutsize);


	m_hFun.KxOpenImage(solveresult, openimg, 7, 7);

	m_hFun.KxCloseImage(openimg, closeimg, 5, 5);

	kxCImageBuf maxregion, fillholesimg;

	m_hBlobFun.SelectMaxRegionByDots(closeimg, maxregion);

	fillholesimg.Init(maxregion.nWidth, maxregion.nHeight);

	m_hAlg.FillHoles(maxregion, fillholesimg);

	ippiAdd_8u_C1RSfs(closeimg.buf, closeimg.nPitch, fillholesimg.buf, fillholesimg.nPitch, DstImg.buf, DstImg.nPitch, imgsize, 0);

}

void CGlueCheck::SliderMatch(kxCImageBuf& SrcImg, kxCImageBuf& Templateimg)
{

	//1. 通过 2G - R - B的方法得到一张灰度图，再进行低阈值二值化

	int nwidth = SrcImg.nWidth;

	int nheight = SrcImg.nHeight;

	kxCImageBuf rgb[3];

	m_hAlg.SplitRGB(SrcImg, rgb);

	kxCImageBuf g_b, g_r;

	g_b.Init(nwidth, nheight);

	g_r.Init(nwidth, nheight);

	IppiSize imgsize = { nwidth, nheight };

	ippiSub_8u_C1RSfs(rgb[0].buf, rgb[0].nPitch, rgb[1].buf, rgb[1].nPitch, g_r.buf, g_r.nPitch, imgsize, 0);

	ippiSub_8u_C1RSfs(rgb[2].buf, rgb[2].nPitch, rgb[1].buf, rgb[1].nPitch, g_b.buf, g_b.nPitch, imgsize, 0);

	kxCImageBuf addimg, threshimg, closeimg;

	addimg.Init(nwidth, nheight);

	threshimg.Init(nwidth, nheight);

	closeimg.Init(nwidth, nheight);

	ippiAdd_8u_C1RSfs(g_r.buf, g_r.nPitch, g_b.buf, g_b.nPitch, addimg.buf, addimg.nPitch, imgsize, 0);

	m_hAlg.ThreshImg(addimg, threshimg, 10, CEmpiricaAlgorithm::_BINARY);

	m_hFun.KxCloseImage(threshimg, closeimg, 5, 5);

	// 2. 归一化图像
	int basew = Templateimg.nWidth;

	int baseh = Templateimg.nHeight;

	kxCImageBuf resizesrc;

	resizesrc.Init(basew, baseh);

	m_hFun.KxResizeImage(closeimg, resizesrc);

	// 3. 测试部分，先对模板图像进行腐蚀
	kxCImageBuf erodetemplate;

	erodetemplate.Init(Templateimg.nWidth, Templateimg.nHeight);

	m_hAlg.ZSErodeImage(Templateimg, erodetemplate, 9, 5, NULL, ippBorderConst, 0);

	for (int i = 0; i < 5; i++)
	{
		m_hAlg.ZSErodeImage(erodetemplate, erodetemplate, 9, 5, NULL, ippBorderConst, 0);
	}

	//kxCImageBuf subresult;

	//subresult.Init(erodetemplate.nWidth, erodetemplate.nHeight);

	//IppiSize imgsizeresize = { erodetemplate.nWidth, erodetemplate.nHeight };

	//ippiSub_8u_C1RSfs(resizesrc.buf, resizesrc.nPitch, erodetemplate.buf, erodetemplate.nPitch, subresult.buf, subresult.nPitch, imgsizeresize, 0);

	//4. 重点部分，对检测图进行N次分割成小图（单向分割）,再用这N张小图在模板图上进行滑动对减

	const int nsplittimes = 16;

	for (int i = 0; i < nsplittimes; i++)
	{
		kxCImageBuf smallimg;

		kxRect<int> cutrect;

		int nstep = erodetemplate.nHeight / nsplittimes;

		cutrect.setup(0, nstep * i, erodetemplate.nWidth -1, nstep * (i + 1) - 1);

		

	}





}

int CGlueCheck::Check(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg, Json::Value &checkresult)
{


	tick_count tbb_start, tbb_end;

	checkresult["defect num"] = 0;

	m_hAlg.SplitRGB(SrcImgA, m_ImgRGB);

	tbb_start = tick_count::now();

	GetGlueMask(m_ImgRGB);

	tbb_end = tick_count::now();

	printf_s("getmask cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);

	tbb_start = tick_count::now();


	cv::Mat targetimg = cv::Mat(m_ImgGlueMask.nHeight, m_ImgGlueMask.nWidth, CV_8UC1, m_ImgGlueMask.buf, m_ImgGlueMask.nPitch);

	int narea = cv::countNonZero(targetimg);

	checkresult["area"] = narea;

	MergeImg(SrcImgA, SrcImgB, m_ImgCheck);

	tbb_end = tick_count::now();

	printf_s("mergeimg cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);

	tbb_start = tick_count::now();



	static int nindex = 0;

	char savepath[64];

	sprintf_s(savepath, "d:\\%d.bmp", nindex++);

	m_hFun.SaveBMPImage_h(savepath, m_ImgCheck);


	kxCImageBuf cutimg;

	cutimg.Init(m_recttarget.Width(), m_recttarget.Height(), m_ImgCheck.nChannel);

	m_hFun.KxCopyImage(m_ImgCheck, cutimg, m_recttarget);

	SliderMatch(cutimg, m_param.m_ImgTemplate);



	checkyiwu(m_ImgCheck, checkresult);

	DstImg.SetImageBuf(m_ImgCheck, true);

	return 1;
}


