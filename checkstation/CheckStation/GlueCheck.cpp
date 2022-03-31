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


void CGlueCheck::checkyiwu(const kxCImageBuf& SrcImg, Json::Value &checkresult)
{
	//1����������ģ��
	//CreateBaseModel(SrcImg);

	IppiSize imgsize = { SrcImg.nWidth, SrcImg.nHeight };

	kxCImageBuf CheckImg;

	CheckImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	ippiAnd_8u_C3R(SrcImg.buf, SrcImg.nPitch, m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, CheckImg.buf, CheckImg.nPitch, imgsize);

	// 2. ��ȡ������
	unsigned char poffsetlow[3] = { m_param.m_noffsetlow, m_param.m_noffsetlow, m_param.m_noffsetlow };// ��ģ�����RGB

	unsigned char poffsethigh[3] = { m_param.m_noffsethigh, m_param.m_noffsethigh, m_param.m_noffsethigh };// ��ģ�����RGB

	//3���õ���ֵ
	m_ImgCheckLow.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);



	ippiSub_8u_C3RSfs(CheckImg.buf, CheckImg.nPitch, m_ImgBaseTemplate.buf, m_ImgBaseTemplate.nPitch, m_ImgCheckLow.buf, m_ImgCheckLow.nPitch, imgsize, 0);

	ippiSubC_8u_C3IRSfs(poffsetlow, m_ImgCheckLow.buf, m_ImgCheckLow.nPitch, imgsize, 0);

	//4, �õ���ֵ
	m_ImgCheckHigh.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);

	ippiSub_8u_C3RSfs(m_ImgBaseTemplate.buf, m_ImgBaseTemplate.nPitch, CheckImg.buf, CheckImg.nPitch, m_ImgCheckHigh.buf, m_ImgCheckHigh.nPitch, imgsize, 0);

	ippiSubC_8u_C3IRSfs(poffsethigh, m_ImgCheckHigh.buf, m_ImgCheckHigh.nPitch, imgsize, 0);


	// ��ɫͼת��Ϊ�Ҷ�ͼ,ȡ���
	m_ImgCheckHighGray.Init(m_ImgCheckHigh.nWidth, m_ImgCheckHigh.nHeight);

	m_ImgCheckLowGray.Init(m_ImgCheckLow.nWidth, m_ImgCheckLow.nHeight);

	GetMaxGray(m_ImgCheckHigh, m_ImgCheckHighGray);
	
	GetMaxGray(m_ImgCheckLow, m_ImgCheckLowGray);

	m_hFun.KxAddImage(m_ImgCheckLowGray, m_ImgCheckHighGray);

	//kxCImageBuf threshimg;

	//threshimg.Init(m_ImgCheckHighGray.nWidth, m_ImgCheckHighGray.nHeight);

	//m_hFun.KxThreshImage(m_ImgCheckHighGray, threshimg, 1, 255);



	kxCImageBuf checkopenimg;

	checkopenimg.Init(m_ImgCheckHighGray.nWidth, m_ImgCheckHighGray.nHeight, m_ImgCheckHighGray.nChannel);

	m_hFun.KxOpenImage(m_ImgCheckHighGray, checkopenimg, 1, 7);


	CutImg2MulImg(checkopenimg);

	ParallelBlob(checkresult);
}




void CGlueCheck::checkqipao(const kxCImageBuf& SrcImg)
{
	// ������
}


void CGlueCheck::checkEdge(const kxCImageBuf& SrcImg)
{
	//�õ�һ�����ı�Ե

	cv::Mat matMask = cv::Mat(m_ImgGlueMask.nHeight, m_ImgGlueMask.nWidth, CV_8UC1, m_ImgGlueMask.buf);

	cv::Rect outrect = cv::boundingRect(matMask);

	m_ImgR_Mask.Init(matMask.cols, matMask.rows);

	IppiSize imgsize = { m_ImgR_Mask.nWidth, m_ImgR_Mask.nHeight};

	ippiSub_8u_C1RSfs(m_ImgGlueMask.buf, m_ImgGlueMask.nPitch, m_ImgRGB[0].buf, m_ImgRGB[0].nPitch, m_ImgR_Mask.buf, m_ImgR_Mask.nPitch, imgsize, 0);

	const int ncstep = 10;//N������һ�α�

	const int nthresh = 18;


	//  ��������ͳһΪ���ѵ������ģ�����������һ������
	
	//�����


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

	const int noffset = 5;// �����ܱ߻��з��⣬Ӱ���ѱ�

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

	// ���Ҳ�

	// ����

	// ����

	//RANSAC���ֱ�ߣ��������





}


void CGlueCheck::SearchEdge(const kxCImageBuf& GrayImg, int ndir, int nThreshvalue, int& nedge1, int& nedge2)
{

	const int nContinuityTimes = 10;//TODO: ���ֵ��Ҫ���⣬���Ƿ���Ҫ���ųɽӿڽ������ã���һ�еı߿��޷�����

	cv::Mat projectimg;

	if (ndir == _Vertical_Project_Dir)//ͶӰ������ϵ���Ǽ���Ĵ�С��ԭ����GrayImg��һ��ͳһ�ߴ�Ĵ�ͼ�������ļ������ֻ��m_rcCheckROI�Ĵ�С
	{
		m_hAlg.CreatProjectImg(GrayImg, projectimg, ndir, m_param.m_rcCheckROI.Height());
	}
	else
	{
		m_hAlg.CreatProjectImg(GrayImg, projectimg, ndir, GrayImg.nWidth);//�����ȫ����
	}


	//----------------��ʱ�������-------------------------//

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
		char savepath[64];

		memset(savepath, 0, sizeof(savepath));

		sprintf_s(savepath, "d:\\img\\grayimg2.bmp");

		m_hFun.SaveBMPImage_h(savepath, GrayImg);

		memset(savepath, 0, sizeof(savepath));

		sprintf_s(savepath, "d:\\img\\blurimg2.bmp");

		m_hFun.SaveBMPImage_h(savepath, blurimg);

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

		char savepath[64];

		memset(savepath, 0, sizeof(savepath));

		sprintf_s(savepath, "d:\\img\\grayimg1.bmp");

		m_hFun.SaveBMPImage_h(savepath, GrayImg);

		memset(savepath, 0, sizeof(savepath));

		sprintf_s(savepath, "d:\\img\\blurimg1.bmp");

		m_hFun.SaveBMPImage_h(savepath, blurimg);

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


	/* ����һ: 2G - R - B �����ֵ������ȡ���blob

	//��ȡͿ����Ĥ
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
	


	//1. ��ȡ���Ƚϸߵı߿�, ���ݻҶ�ͶӰ�ó�

	int ntop, nright, nbottom, nleft;

	SearchEdge(RGB[0], _Vertical_Project_Dir, 180, nleft, nright);

	SearchEdge(RGB[0], _Horizontal_Project_Dir, 150, ntop, nbottom);

	
	//2. �õ��ϸߵı�Ե�󣬶�����������о���λ

	//(1) �ü�Ŀ������
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

	// (2) �ѵ�
	const int npointnum = 20;

	kxPoint<int> leftpoint[npointnum];

	kxPoint<int> rightpoint[npointnum];

	GetEdgePoints(edgeimg1, 1, 10, leftpoint, npointnum);

	GetEdgePoints(edgeimg2, 0, 10, rightpoint, npointnum);
	 
	// (3) ����ѵ��ĵ�������ֱ�ߣ�Ȼ��ȡһ��y���м�ֵ�õ�xֵ�Ƕ��٣�Ŀǰ����ֱ��ȡƽ��ֵ��

	int naverageleft = 0;

	int naverageright = 0;
	
	for (int i = 0; i < npointnum; i++)
	{
		naverageleft += leftpoint[i].x;

		naverageright += rightpoint[i].x;
	}

	const int nextend = 10;//��������һ�㣬��ʱ����

	naverageleft = naverageleft / npointnum + nextend;

	naverageright = edgeimg2.nWidth - naverageright / npointnum - nextend;



	int hoffset = 250;

	int bottomoffset = 100;

	nleft = min(nleft + naverageleft, RGB[0].nWidth / 2);

	nright = max(nright - naverageright, RGB[0].nWidth / 2);

	ntop = min(ntop + hoffset, RGB[0].nHeight / 2);

	nbottom = max(nbottom - bottomoffset, RGB[0].nHeight / 2);


	//3. ����ǰһ������ȡ�Ŀ��ڣ�������ɫ�ָ�
	m_recttarget.setup(nleft, ntop, nright, nbottom);

	// �ɷ�����ֱ��һƬmask
	m_ImgGlueMask.Init(RGB[0].nWidth, RGB[0].nHeight, RGB[0].nChannel);

	ippsSet_8u(0, m_ImgGlueMask.buf, m_ImgGlueMask.nPitch * m_ImgGlueMask.nHeight);

	IppiSize copysize = { nright - nleft + 1, nbottom - ntop + 1 };

	ippiSet_8u_C1R(255, m_ImgGlueMask.buf + m_ImgGlueMask.nPitch * ntop + nleft, m_ImgGlueMask.nPitch, copysize);




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



void CGlueCheck::MergeImg(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg)
{
	kxCImageBuf gray1, threshimg, dilateimg;

	gray1.Init(SrcImgA.nWidth, SrcImgA.nHeight);

	dilateimg.Init(SrcImgA.nWidth, SrcImgA.nHeight);

	IppiSize imgsize = { SrcImgA.nWidth, SrcImgA.nHeight };

	//ȡsrcimgA��RGB���ֵ
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

	//B �� mask����
	ippiAnd_8u_C3R(guobaomaskimg.buf, guobaomaskimg.nPitch, SrcImgB.buf, SrcImgB.nPitch, img2.buf, img2.nPitch, imgsize);

	DstImg.Init(threshimg.nWidth, threshimg.nHeight, 3);

	ippiAdd_8u_C3RSfs(img1.buf, img1.nPitch, img2.buf, img2.nPitch, DstImg.buf, DstImg.nPitch, imgsize, 0);

	ippiAnd_8u_C3IR(m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, DstImg.buf, DstImg.nPitch, imgsize);


}

void CGlueCheck::ExtractGreen(const kxCImageBuf* RGB, kxRect<int> roi, kxCImageBuf& DstImg)
{
	
	/*
	// 1. ��ͼ��ת��Ϊ32f
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


	// 3. ��ȡ�����Сֵ��ת��Ϊ 0-255��ע�����ʱ����Сֵ�����Ǹ��ģ�
	double minval = 0;

	double maxval = 0;

	cv::minMaxLoc(submat32f, &minval, &maxval);

	// 4.(val - minval) / (maxval - minval)  * 255 
	if (abs(minval - maxval) < 1e-6)
	{
		// TODO �쳣������ֵdstimg������
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
	*/


	// ����Ƕ�ͼ�����2G - R - B����ȡ��Ȼ���˳���ɫ������Ӱ��
	kxCImageBuf G_R, G_B;
	G_R.Init(RGB[0].nWidth, RGB[0].nHeight);
	G_B.Init(RGB[0].nWidth, RGB[0].nHeight);

	IppiSize imgsize = { G_R.nWidth, G_R.nHeight};
	ippiSub_8u_C1RSfs(RGB[0].buf, RGB[0].nPitch, RGB[1].buf, RGB[1].nPitch, G_R.buf, G_R.nPitch, imgsize, 0);
	ippiSub_8u_C1RSfs(RGB[2].buf, RGB[2].nPitch, RGB[1].buf, RGB[1].nPitch, G_B.buf, G_B.nPitch, imgsize, 0);

	kxCImageBuf addimg;
	addimg.Init(RGB[0].nWidth, RGB[0].nHeight);

	ippiAdd_8u_C1RSfs(G_R.buf, G_R.nPitch, G_B.buf, G_B.nPitch, addimg.buf, addimg.nPitch, imgsize, 0);


	kxCImageBuf mask1, mask2;
	m_hFun.KxThreshImage(addimg, mask1, 7, 255);
	m_hFun.KxThreshImage(G_B, mask2, 7, 255);//������ʵ�Ƕ� G - R����ֵ������ΪRGB�Ƿ���

	kxCImageBuf mask;
	mask.Init(RGB[0].nWidth, RGB[0].nHeight);
	ippiAnd_8u_C1R(mask1.buf, mask1.nPitch, mask2.buf, mask2.nPitch, mask.buf, mask.nPitch, imgsize);

	DstImg.Init(mask.nWidth, mask.nHeight);

	kxCImageBuf openimg, closeimg;
	openimg.Init(mask.nWidth, mask.nHeight);
	closeimg.Init(mask.nWidth, mask.nHeight);



	//

	// ���淽������
	////�ȿ�����ȥ��ɢ��
	m_hFun.KxOpenImage(mask, openimg, 5, 5);

	////������պ�С��
	m_hFun.KxCloseImage(openimg, closeimg, 21,11);

	////��ʴ���α�Ե�����ױ���
	//kxCImageBuf erordeimg;
	//erordeimg.Init(closeimg.nWidth, closeimg.nHeight);
	//m_hFun.KxErodeImage(closeimg, erordeimg, 15, 15);

	//m_hFun.KxErodeImage(erordeimg, DstImg, 15, 15);


	kxCImageBuf fillholes;
	fillholes.Init(mask.nWidth, mask.nHeight);
	m_hFun.KxFillHoles(closeimg, fillholes);
	CKxBlobAnalyse blob;
	kxCImageBuf subimg;
	subimg.Init(fillholes.nWidth, fillholes.nHeight);
	ippiSub_8u_C1RSfs(closeimg.buf, closeimg.nPitch, fillholes.buf, fillholes.nPitch, subimg.buf, subimg.nPitch, imgsize, 0);
	blob.ToBlobByCV(subimg, CKxBlobAnalyse::_SORT_BYDOTS, 100);
	int nblobcount = blob.GetBlobCount();
	
	for (int i = 0; i < blob.GetBlobCount(); i++)
	{
		CKxBlobAnalyse::SingleBlobInfo blobinfo;

		blobinfo = blob.GetSortSingleBlob(i);
		//std::cout << blobinfo.m_rc.left << blobinfo.m_rc.top << blobinfo.m_rc.right << blobinfo.m_rc.bottom << std::endl;

		if (blobinfo.m_rc.Width() > 1000)//���>1000����
		{
			//std::cout << "in" << blobinfo.m_rc.Width() << std::endl;
			kxCImageBuf roiimg;

			blobinfo.m_rc.left = gMax(blobinfo.m_rc.left - 50, 0);

			blobinfo.m_rc.right = gMin(blobinfo.m_rc.right + 50, subimg.nWidth - 1);
			
			blob.GetBlobImage(blobinfo.m_nLabel, blobinfo.m_rc, roiimg);

			kxCImageBuf dilatesmallimg;

			dilatesmallimg.Init(roiimg.nWidth, roiimg.nHeight);

			m_hFun.KxDilateImage(roiimg, dilatesmallimg, 21, 21);

			IppiSize roisize = {roiimg.nWidth, roiimg.nHeight};

			kxCImageBuf subresult;

			subresult.Init(roiimg.nWidth, roiimg.nHeight);

			ippiSub_8u_C1IRSfs(dilatesmallimg.buf, dilatesmallimg.nPitch, fillholes.buf + blobinfo.m_rc.left + blobinfo.m_rc.top * fillholes.nPitch, fillholes.nPitch, roisize, 0);

		}
	}
	
	kxCImageBuf erodeimg;
	m_hFun.KxErodeImage(fillholes, erodeimg, 15, 15);
	DstImg.SetImageBuf(erodeimg, true);
}

void CGlueCheck::SliderMatch(kxCImageBuf& SrcImg, kxCImageBuf& Templateimg)
{

	//1. ͨ�� 2G - R - B�ķ����õ�һ�ŻҶ�ͼ���ٽ��е���ֵ��ֵ��

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

	// 2. ��һ��ͼ��
	int basew = closeimg.nWidth / 4;

	int baseh = closeimg.nHeight / 4;

	kxCImageBuf resizesrc;

	resizesrc.Init(basew, baseh);

	m_hFun.KxResizeImage(closeimg, resizesrc);

	// 3. ���Բ��֣��ȶ�ģ��ͼ����и�ʴ
	kxCImageBuf erodetemplate;

	erodetemplate.Init(Templateimg.nWidth, Templateimg.nHeight);

	m_hAlg.ZSErodeImage(Templateimg, erodetemplate, 9, 9, NULL, ippBorderConst, 0);

	for (int i = 0; i < 1; i++)
	{
		m_hAlg.ZSErodeImage(erodetemplate, erodetemplate, 9, 9, NULL, ippBorderConst, 0);
	}

	//kxCImageBuf subresult;

	//subresult.Init(erodetemplate.nWidth, erodetemplate.nHeight);

	//IppiSize imgsizeresize = { erodetemplate.nWidth, erodetemplate.nHeight };

	//ippiSub_8u_C1RSfs(resizesrc.buf, resizesrc.nPitch, erodetemplate.buf, erodetemplate.nPitch, subresult.buf, subresult.nPitch, imgsizeresize, 0);


	// ��ģ��ͼ�ϡ��±�Ե����ɫ
	kxCImageBuf bigtemplate;

	const int headbotextend = 100;

	bigtemplate.Init(erodetemplate.nWidth, erodetemplate.nHeight + headbotextend * 2);

	ippsSet_8u(0, bigtemplate.buf, bigtemplate.nPitch * bigtemplate.nHeight);

	IppiSize templatesize = { erodetemplate.nWidth, erodetemplate.nHeight};

	ippiCopy_8u_C1R(erodetemplate.buf, erodetemplate.nPitch, bigtemplate.buf + bigtemplate.nPitch * headbotextend, bigtemplate.nPitch, templatesize);
	
	//4. �ص㲿�֣��Լ��ͼ����N�ηָ��Сͼ������ָ,������N��Сͼ��ģ��ͼ�Ͻ��л����Լ�

	const int nsplittimes = 16;

	kxCImageBuf bigCCImg;

	bigCCImg.Init(resizesrc.nWidth, resizesrc.nHeight);

	// ��һ������
	//IppiSize masksize = {200, 200};
	//ippiSet_8u_C1R(0, resizesrc.buf + 290 + 624 * resizesrc.nPitch, resizesrc.nPitch, masksize);


	for (int i = 0; i < nsplittimes; i++)
	{
		// (1) �ü����ͼ
		kxCImageBuf smallimg;

		kxRect<int> cutrect;

		int nstep = resizesrc.nHeight / nsplittimes;

		cutrect.setup(0, nstep * i, resizesrc.nWidth -1, nstep * (i + 1) - 1);

		smallimg.Init(cutrect.Width(), cutrect.Height());

		m_hFun.KxCopyImage(resizesrc, smallimg, cutrect);

		// (2) �ü�ģ��ͼ��ģ��ͼ��������������

		kxCImageBuf smalltemplate;

		const int nextend = 100;

		int ntemplatetop = gMax(0, nstep * i - nextend);

		int ntemplatebottom = gMin(bigtemplate.nHeight - 1, nstep * (i + 1) - 1 + nextend);

		cutrect.setup(0, ntemplatetop, bigtemplate.nWidth - 1, ntemplatebottom);

		smalltemplate.Init(cutrect.Width(), cutrect.Height());

		m_hFun.KxCopyImage(bigtemplate, smalltemplate, cutrect);

		kxCImageBuf CCImg;

		CCImg.Init(smallimg.nWidth, smallimg.nHeight);

		SliderSub(smallimg, smalltemplate, CCImg, 5);

		IppiSize cutsize = { CCImg.nWidth, CCImg.nHeight};

		ippiCopy_8u_C1R(CCImg.buf, CCImg.nPitch, bigCCImg.buf + bigCCImg.nPitch * nstep * i, bigCCImg.nPitch, cutsize);

	}


}

void CGlueCheck::CheckLow(kxCImageBuf& SrcImg)
{
	//����ļ�ͣ��Ǽ��Ϳ���ڲ������
	// ��רҵ������˵���Ǽ����ȡ��Ϳ�����ڲ��׶�


	cv::Mat matsrc = cv::Mat(SrcImg.nHeight, SrcImg.nWidth, CV_8UC(SrcImg.nChannel), SrcImg.buf, SrcImg.nPitch);

	cv::Mat hsvimg;

	cv::cvtColor(matsrc, hsvimg, cv::COLOR_RGB2HSV);

	kxCImageBuf kxhsvimg;

	kxhsvimg.SetImageBuf(hsvimg.data, hsvimg.cols, hsvimg.rows, hsvimg.step, hsvimg.channels(), false);



	//1. ��ȡ�̽�
	kxCImageBuf img_dst;
	img_dst.Init(kxhsvimg.nWidth, kxhsvimg.nHeight, kxhsvimg.nChannel);

	int nimg_width = kxhsvimg.nWidth;
	int nimg_height = kxhsvimg.nHeight;

	// HSV��ֵ����
	int nh_min = 29;
	int nh_max = 224;
	int ns_min = 11;
	int ns_max = 255;
	int nv_min = 38;
	int nv_max = 100;

	/* ��������HSV��ͨ����ͼƬ */
	kxCImageBuf img_hsv_to_h;
	img_hsv_to_h.Init(nimg_width, nimg_height, 1);
	kxCImageBuf img_hsv_to_s;
	img_hsv_to_s.Init(nimg_width, nimg_height, 1);

	/* �ֱ�ͼƬת����hsv����������ͨ����ͼƬ */
	CKxBaseFunction basefun_extractgreen;
	// HSV ����ͨ������ֵ��,Ȼ������
	basefun_extractgreen.KxThreshImage(kxhsvimg, img_hsv_to_h, nh_min, nh_max, HSV_H);
	basefun_extractgreen.KxThreshImage(kxhsvimg, img_hsv_to_s, ns_min, ns_max, HSV_S);
	basefun_extractgreen.KxThreshImage(kxhsvimg, img_dst, nv_min, nv_max, HSV_V);
	basefun_extractgreen.KxMinEvery(img_hsv_to_h, img_hsv_to_s);
	basefun_extractgreen.KxMinEvery(img_hsv_to_s, img_dst);            // img_dst Ϊ�������ɵ�ͼ��


	//2. ��ȡ���blob
	//int i = 10;

}

void CGlueCheck::checkwithmodel(const kxCImageBuf& SrcImg, const kxCImageBuf& gluearea, kxCImageBuf& dsthigh, kxCImageBuf& dstlow)
{
	// ���ݱ�׼ģ�壬�Լ��õ�ɫ����ߵͽ������dsthigh, dstlow

	//1����������ģ��
	CreateBaseModel(SrcImg, gluearea, m_ImgTemplateHigh, m_ImgTemplateLow);

	char savepath[64];

	IppiSize imgsize = { SrcImg.nWidth, SrcImg.nHeight };

	kxCImageBuf CheckImg;

	CheckImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	Ipp8u* pbuf[3] = { gluearea.buf, gluearea.buf, gluearea.buf };

	m_ImgColorGlueMask.Init(SrcImg.nWidth, SrcImg.nHeight, 3);

	ippiCopy_8u_P3C3R(pbuf, gluearea.nPitch, m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, imgsize);

	ippiAnd_8u_C3R(SrcImg.buf, SrcImg.nPitch, m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, CheckImg.buf, CheckImg.nPitch, imgsize);



	// 2. ��ȡ������
	unsigned char poffsetlow[3] = { m_param.m_noffsetlow, m_param.m_noffsetlow, m_param.m_noffsetlow };// ��ģ�����RGB

	unsigned char poffsethigh[3] = { m_param.m_noffsethigh, m_param.m_noffsethigh, m_param.m_noffsethigh };// ��ģ�����RGB


	//3, �õ���ֵ
	m_ImgCheckHigh.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);

	ippiSub_8u_C3RSfs(m_ImgTemplateHigh.buf, m_ImgTemplateHigh.nPitch, CheckImg.buf, CheckImg.nPitch, m_ImgCheckHigh.buf, m_ImgCheckHigh.nPitch, imgsize, 0);

	ippiSubC_8u_C3IRSfs(poffsethigh, m_ImgCheckHigh.buf, m_ImgCheckHigh.nPitch, imgsize, 0);

	//4���õ���ֵ����ֵͨ�����Է���Hͨ������Ч���ȽϺ�, ��H -  ��׼ɫ.����Ҫ������ֵ��Ӱ��
	cv::Mat img;

	m_hFun.KxImageBufToMat(CheckImg, img);

	cv::Mat hsv[3], hsvimg;

	cv::cvtColor(img, hsvimg, cv::COLOR_RGB2HSV);

	cv::split(hsvimg, hsv);

	kxCImageBuf himg;

	m_hFun.MatToKxImageBuf(hsv[0], himg, true);

	// Hͼ����Ҫ���ڼ�ⰵȱ�ݣ�����һЩ������Ҳ��������⣬��������Ը����Ľ����˳�
	kxCImageBuf maskhimg;

	GetHmask(CheckImg, maskhimg);

	memset(savepath, 0, sizeof(savepath));

	sprintf_s(savepath, "d:\\img\\CheckImg.bmp");

	m_hFun.SaveBMPImage_h(savepath, CheckImg);

	memset(savepath, 0, sizeof(savepath));

	sprintf_s(savepath, "d:\\img\\himg.bmp");

	m_hFun.SaveBMPImage_h(savepath, himg);

	ippiSub_8u_C1IRSfs(maskhimg.buf, himg.nPitch, himg.buf, himg.nPitch, imgsize, 0);
	memset(savepath, 0, sizeof(savepath));

	sprintf_s(savepath, "d:\\img\\maskhimg.bmp");

	m_hFun.SaveBMPImage_h(savepath, maskhimg);

	m_ImgCheckLow.Init(CheckImg.nWidth, CheckImg.nHeight);

	ippiSub_8u_C1RSfs(m_ImgTemplateLow.buf, m_ImgTemplateLow.nPitch, himg.buf, himg.nPitch, m_ImgCheckLow.buf, m_ImgCheckLow.nPitch, imgsize, 0);

	ippiSubC_8u_C1IRSfs(poffsetlow[0], m_ImgCheckLow.buf, m_ImgCheckLow.nPitch, imgsize, 0);


	//5, ��ɫͼת��Ϊ�Ҷ�ͼ,ȡ���
	dsthigh.Init(m_ImgCheckHigh.nWidth, m_ImgCheckHigh.nHeight);

	dstlow.Init(m_ImgCheckLow.nWidth, m_ImgCheckLow.nHeight);

	kxCImageBuf openimg, closeimg;

	openimg.Init(m_ImgCheckHigh.nWidth, m_ImgCheckHigh.nHeight);

	GetMaxGray(m_ImgCheckHigh, openimg);

	//GetMaxGray(m_ImgCheckLow, dstlow);

	m_hFun.KxCloseImage(m_ImgCheckLow, dstlow, 7, 7);// ����ñ�����

	memset(savepath, 0, sizeof(savepath));

	sprintf_s(savepath, "d:\\img\\dstlow.bmp");

	m_hFun.SaveBMPImage_h(savepath, dstlow);

	//dstlow.SetImageBuf(m_ImgCheckLow, true);

	m_hFun.KxOpenImage(openimg, dsthigh, 1, 7);// ��������������λ��





	//memset(savepath, 0, sizeof(savepath));

	//sprintf_s(savepath, "d:\\img\\dsthigh.bmp");

	//m_hFun.SaveBMPImage_h(savepath, dsthigh);

}





// ---------- 2022.2.28 �ٴ���д����Ϊ���õ� ------------------//

void CGlueCheck::ParallelBlob(Json::Value &checkresult)
{

	// TODO opencv blob�и�bug��̫��ȱ�ݵ�ʱ������
	parallel_for(blocked_range<int>(0, m_nblobimgnum),
		[&](const blocked_range<int>& range)
	{
		for (int index = range.begin(); index != range.end(); index++)
		{
			m_phblob[index].ToBlobByCV(m_pBLOBIMG[index], CKxBlobAnalyse::_SORT_BYDOTS, 100);
		}
	}, auto_partitioner());


	int nsingleimgstep = _SINGLE_BLOBIMG_H - _IMG_OVERLAP;// ����ͼ����ת����ȫ������

	int nblobindex = 0;
	checkresult["defectarea"] = 0;
	checkresult["qipaoarea"] = 0;


	for (int nimgindex = 0; nimgindex < m_nblobimgnum; nimgindex++)
	{
		if (m_phblob[nimgindex].GetBlobCount() > 0)
		{
			for (int i = 0; i < m_phblob[nimgindex].GetBlobCount(); i++)
			{
				CKxBlobAnalyse::SingleBlobInfo blobinfo;

				blobinfo = m_phblob[nimgindex].GetSortSingleBlob(i);

				kxCImageBuf smallblobimg;

				m_phblob[nimgindex].GetBlobImage(blobinfo.m_nLabel, blobinfo.m_rc, smallblobimg);

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


					kxRect<int> defectrect = blobinfo.m_rc;
					defectrect.offset(0, nsingleimgstep * nimgindex);
					//defectrect.top = defectrect.top + nsingleimgstep * nimgindex;

					if (JudgeDefectType(smallblobimg, defectrect))
					{
						single["defecttype"] = 1;//����
						checkresult["qipaoarea"] = checkresult["qipaoarea"].asInt() + blobinfo.m_nDots;

					}
					else
					{
						single["defecttype"] = 0;
						checkresult["defectarea"] = checkresult["defectarea"].asInt() + blobinfo.m_nDots;

					}
					checkresult["defect feature"].append(single);
					checkresult["defect num"] = checkresult["defect num"].asInt() + 1;


				}
			}
		}
	}


	//if (m_param.m_nGrabTimes == 0 && m_param.m_nCurGrabID == 0)
	//{
	//	Json::Value single;
	//	single["Dots"] = 200;
	//	single["Energy"] = 0;
	//	char defectid[32];
	//	sprintf_s(defectid, "%d_%d", m_nID, nblobindex);
	//	std::cout << defectid << std::endl;
	//	nblobindex++;
	//	single["defectid"] = defectid;
	//	single["pos"].append(2720);
	//	single["pos"].append(2990);
	//	single["pos"].append(400);
	//	single["pos"].append(200);
	//	checkresult["defect feature"].append(single);
	//	checkresult["defect num"] = checkresult["defect num"].asInt() + 1;
	//}


	std::cout << " �� " << m_nID << " ��⵽ȱ�������� " << checkresult["defect num"].asInt() << "  " << checkresult["defectarea"].asInt() << std::endl;

}


void CGlueCheck::CutImg2MulImg(const kxCImageBuf& CheckImg)
{
	/*
		��һ�Ŵ�ͼ�ֳ�N�����ص������ͼ���������blob����
	*/

	// -1 �� +1������ȡ��
	m_nblobimgnum = int((CheckImg.nHeight - _IMG_OVERLAP - 1) / (_SINGLE_BLOBIMG_H - _IMG_OVERLAP)) + 1;// ����ȡ��

	if (m_nblobimgnum > _MAX_BLOBIMG)
	{
		printf_s("roi��Ĵ�С�������õĲ�����������");
		// KXPRINTF(); �Ȳ�������
		m_nblobimgnum = _MAX_BLOBIMG;
	}

	int nh2copy = (_SINGLE_BLOBIMG_H - _IMG_OVERLAP) * m_nblobimgnum + _IMG_OVERLAP;


	m_ImgZero2split.Init(CheckImg.nWidth, nh2copy, 1);

	ippsSet_8u(0, m_ImgZero2split.buf, m_ImgZero2split.nPitch * m_ImgZero2split.nHeight);

	IppiSize copysize = { CheckImg.nWidth, CheckImg.nHeight };

	ippiCopy_8u_C1R(CheckImg.buf, CheckImg.nPitch, m_ImgZero2split.buf, m_ImgZero2split.nPitch, copysize);

	for (int i = 0; i < m_nblobimgnum; i++)
	{
		unsigned char *copybuf = m_ImgZero2split.buf + i * (_SINGLE_BLOBIMG_H - _IMG_OVERLAP) * m_ImgZero2split.nPitch;

		m_pBLOBIMG[i].Init(CheckImg.nWidth, _SINGLE_BLOBIMG_H);

		IppiSize smallsize = { CheckImg.nWidth, _SINGLE_BLOBIMG_H };

		ippiCopy_8u_C1R(copybuf, m_ImgZero2split.nPitch, m_pBLOBIMG[i].buf, m_pBLOBIMG[i].nPitch, smallsize);
	}
}


void CGlueCheck::GetTargetROI(const kxCImageBuf& SrcImg, kxRect<int> rcCheck, kxRect<int>& targetrect, float& rotateangle)
{
	//���ݽ�ģROI�ѳ�����Ŀ������


	//1. ��ȡ�����������ͼ��Ŀ����Ϊ�˼��Ч��
	kxRect<int> cutroi = rcCheck;

	const int offset = 200;

	cutroi.left = gMax(cutroi.left - offset, 0);

	cutroi.top = gMax(cutroi.top - offset, 0);

	cutroi.right = gMin(cutroi.right + offset, SrcImg.nWidth - 1);

	cutroi.bottom = gMin(cutroi.bottom + offset, SrcImg.nHeight - 1);

	kxCImageBuf targetimg;

	targetimg.Init(cutroi.Width(), cutroi.Height(), SrcImg.nChannel);

	m_hFun.KxCopyImage(SrcImg, targetimg, cutroi);


	//2. �ü�ͼ���������������ĸ���

	//��1������
	kxCImageBuf RGB[3], edgeimg1, edgeimg2;

	m_hAlg.SplitRGB(targetimg, RGB);

	/*char savepath[64];

	memset(savepath, 0, sizeof(savepath));

	sprintf_s(savepath, "d:\\img\\R.bmp");

	m_hFun.SaveBMPImage_h(savepath, RGB[0]);

	memset(savepath, 0, sizeof(savepath));

	sprintf_s(savepath, "d:\\img\\G.bmp");

	m_hFun.SaveBMPImage_h(savepath, RGB[1]);

	memset(savepath, 0, sizeof(savepath));

	sprintf_s(savepath, "d:\\img\\B.bmp");

	m_hFun.SaveBMPImage_h(savepath, RGB[2]);*/

	// ���Ե��(0��ʼ����)
	int edge1 = gMin(offset * 2, RGB[1].nWidth);

	IppiSize cutsize = { edge1, RGB[1].nHeight };

	edgeimg1.Init(cutsize.width, cutsize.height);

	ippiCopy_8u_C1R(RGB[1].buf, RGB[1].nPitch, edgeimg1.buf, edgeimg1.nPitch, cutsize);

	// �ұ�Ե��(0��ʼ����)
	int edge2 = gMax(0, RGB[1].nWidth - offset * 2);

	int nedge2width = RGB[1].nWidth - edge2;

	cutsize = { nedge2width, RGB[1].nHeight };

	edgeimg2.Init(cutsize.width, cutsize.height);

	ippiCopy_8u_C1R(RGB[1].buf + edge2, RGB[1].nPitch, edgeimg2.buf, edgeimg2.nPitch, cutsize);

	const int npointnum = 20;

	kxPoint<int> leftpoint[npointnum];

	kxPoint<int> rightpoint[npointnum];

	GetEdgePoints(edgeimg1, 1, 10, leftpoint, npointnum);

	GetEdgePoints(edgeimg2, 0, 10, rightpoint, npointnum);

	//int naverageleft = 0;

	//int naverageright = 0;

	//int offsetpoint = 5;

	//for (int i = offsetpoint; i < npointnum - offsetpoint; i++)
	//{
	//	naverageleft += leftpoint[i].x;

	//	naverageright += rightpoint[i].x;
	//}

	// ����Ҫ�������Ϊ90�ȵ�ʱ����ô����?

	float bestangle1 = 0;

	float bestangle2 = 0;

	float a1, b1, a2, b2;

	m_hfitline.FitLineByRansac(leftpoint, npointnum, 3, bestangle1, a1, b1);

	m_hfitline.FitLineByRansac(rightpoint, npointnum, 3, bestangle2, a2, b2);

	rotateangle = bestangle1;
	//cv::Mat src = cv::Mat(edgeimg1.nHeight, edgeimg1.nWidth, CV_8UC(edgeimg1.nChannel), edgeimg1.buf, edgeimg1.nPitch);

	//cv::Mat rotatematri;

	//rotatematri = cv::getRotationMatrix2D(cv::Point2f(0, 0), bestangle - 90, 1);

	//cv::Mat dst;

	//cv::warpAffine(src, dst, rotatematri, cv::Size(src.cols, src.rows));

	//cv::imwrite("d:\\img\\src.bmp", src);

	//cv::imwrite("d:\\img\\rotate.bmp", dst);


	//const int nextend = -20;//��������һ�㣬��ʱ����

	//naverageleft = naverageleft / (npointnum - 2 * offsetpoint) + nextend;

	//naverageright = edgeimg2.nWidth - naverageright / npointnum - nextend;



	// ��2������
	int ntargettop = 0; 
	
	int ntargetbottom = 0;

	SearchEdge(RGB[0], _Horizontal_Project_Dir, 150, ntargettop, ntargetbottom);


	int ntargetleft, ntargetright;
	if (abs(bestangle1 - 90.0) < 1e-6)
	{
		ntargetleft = b1;
	}
	else
	{
		ntargetleft = min((ntargettop - b1) / a1, (ntargetbottom - b1) / a1);

	}

	if (abs(bestangle2 - 90.0) < 1e-6)
	{
		ntargetright = edge2 + b2;
	}
	else
	{
		ntargetright = edge2 + max((ntargettop - b2) / a2, (ntargetbottom - b2) / a2);
	}


	// ��3�������SrcImg��ƫ�ƣ��Ӷ��õ��������
	
	targetrect.setup(ntargetleft, ntargettop, ntargetright, ntargetbottom);

	targetrect.offset(cutroi.left, cutroi.top);


}


void CGlueCheck::MergeImgNew(const kxCImageBuf& SrcImg1, const kxCImageBuf& SrcImg2, kxRect<int> targetrect, kxCImageBuf& DstImg)
{
	// �ü����򣬽��кϲ�����ͼ��������ϲ������ǽ�ͼ1�������򻻳�ͼ2
	kxCImageBuf SrcImgA, SrcImgB;

	SrcImgA.Init(targetrect.Width(), targetrect.Height(), SrcImg1.nChannel);

	SrcImgB.Init(targetrect.Width(), targetrect.Height(), SrcImg1.nChannel);

	m_hFun.KxCopyImage(SrcImg1, SrcImgA, targetrect);

	m_hFun.KxCopyImage(SrcImg2, SrcImgB, targetrect);



	kxCImageBuf gray1, threshimg, dilateimg;

	gray1.Init(SrcImgA.nWidth, SrcImgA.nHeight);

	dilateimg.Init(SrcImgA.nWidth, SrcImgA.nHeight);

	IppiSize imgsize = { SrcImgA.nWidth, SrcImgA.nHeight };

	//ȡsrcimgA��RGB���ֵ
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

	unsigned char* pbuf[3] = { threshimg.buf, threshimg.buf, threshimg.buf };

	ippiCopy_8u_P3C3R(pbuf, threshimg.nPitch, guobaomaskimg.buf, guobaomaskimg.nPitch, imgsize);

	// A - mask 
	ippiSub_8u_C3RSfs(guobaomaskimg.buf, guobaomaskimg.nPitch, SrcImgA.buf, SrcImgA.nPitch, img1.buf, img1.nPitch, imgsize, 0);

	//B �� mask����
	ippiAnd_8u_C3R(guobaomaskimg.buf, guobaomaskimg.nPitch, SrcImgB.buf, SrcImgB.nPitch, img2.buf, img2.nPitch, imgsize);

	kxCImageBuf mergeresult;

	mergeresult.Init(threshimg.nWidth, threshimg.nHeight, 3);
	//DstImg.Init(threshimg.nWidth, threshimg.nHeight, 3);

	ippiAdd_8u_C3RSfs(img1.buf, img1.nPitch, img2.buf, img2.nPitch, mergeresult.buf, mergeresult.nPitch, imgsize, 0);

	//ippiAnd_8u_C3IR(m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, mergeresult.buf, mergeresult.nPitch, imgsize);

	ippiCopy_8u_C3R(mergeresult.buf, mergeresult.nPitch, DstImg.buf, DstImg.nPitch, imgsize);
}


void CGlueCheck::ExtractGreenNew(const kxCImageBuf* RGB, kxCImageBuf& DstImg)
{
	// ����Ƕ�ͼ�����2G - R - B����ȡ
	kxCImageBuf G_R, G_B;
	G_R.Init(RGB[0].nWidth, RGB[0].nHeight);
	G_B.Init(RGB[0].nWidth, RGB[0].nHeight);

	IppiSize imgsize = { G_R.nWidth, G_R.nHeight };
	ippiSub_8u_C1RSfs(RGB[0].buf, RGB[0].nPitch, RGB[1].buf, RGB[1].nPitch, G_R.buf, G_R.nPitch, imgsize, 0);
	ippiSub_8u_C1RSfs(RGB[2].buf, RGB[2].nPitch, RGB[1].buf, RGB[1].nPitch, G_B.buf, G_B.nPitch, imgsize, 0);

	kxCImageBuf addimg;
	addimg.Init(RGB[0].nWidth, RGB[0].nHeight);

	ippiAdd_8u_C1RSfs(G_R.buf, G_R.nPitch, G_B.buf, G_B.nPitch, addimg.buf, addimg.nPitch, imgsize, 0);

	kxCImageBuf mask;
	mask.Init(RGB[0].nWidth, RGB[0].nHeight);
	m_hFun.KxThreshImage(addimg, mask, 7, 255);

	DstImg.Init(mask.nWidth, mask.nHeight);

	kxCImageBuf openimg, closeimg;
	openimg.Init(mask.nWidth, mask.nHeight);
	closeimg.Init(mask.nWidth, mask.nHeight);

	// TODO : ������Ҫ������ӶԻ�ɫ��Ե������Ĥ��������Ϊ������Ĥ���ڵ��̶�λ��

	//

	// ���淽������
	////�ȿ�����ȥ��ɢ��
	m_hFun.KxOpenImage(mask, openimg, 5, 5);// �˳���Ϳ������ɢ��

	Global_SaveDebugImg("openimg", openimg);


	////������պ�С��
	m_hFun.KxCloseImage(openimg, closeimg, 7, 7);

	Global_SaveDebugImg("closeimg", closeimg);

	ippiCopy_8u_C1R(closeimg.buf, closeimg.nPitch, DstImg.buf, DstImg.nPitch, imgsize);
}


void CGlueCheck::SliderMatchNew(kxCImageBuf& SrcImg, int ntargetw, int ntargeth, kxCImageBuf& Templateimg, kxCImageBuf& blobimg)
{
	//1. 
	kxRect<int> targetrect;

	targetrect.setup(0, 0, ntargetw - 1, ntargeth - 1);

	kxCImageBuf targetimg;

	targetimg.Init(ntargetw, ntargeth);

	m_hFun.KxCopyImage(SrcImg, targetimg, targetrect);


	// 2. ��һ��ͼ��
	int basew = targetimg.nWidth / 4;

	int baseh = targetimg.nHeight / 4;

	kxCImageBuf resizesrc;

	resizesrc.Init(basew, baseh);

	m_hFun.KxResizeImage(targetimg, resizesrc);

	// 3. TODO ��������ģ��������ⲿ�֣��ȶ�ģ��ͼ����и�ʴ������Ҫ��ֲ����ģ��ȥ�����ݲ����������ã�����
	kxCImageBuf erodetemplate;

	erodetemplate.Init(Templateimg.nWidth, Templateimg.nHeight);

	m_hAlg.ZSErodeImage(Templateimg, erodetemplate, 9, 9, NULL, ippBorderConst, 0);

	m_hAlg.ZSErodeImage(erodetemplate, erodetemplate, 9, 9, NULL, ippBorderConst, 0);


	//for (int i = 0; i < 1; i++)
	//{
	//}

	//kxCImageBuf subresult;

	//subresult.Init(erodetemplate.nWidth, erodetemplate.nHeight);

	//IppiSize imgsizeresize = { erodetemplate.nWidth, erodetemplate.nHeight };

	//ippiSub_8u_C1RSfs(resizesrc.buf, resizesrc.nPitch, erodetemplate.buf, erodetemplate.nPitch, subresult.buf, subresult.nPitch, imgsizeresize, 0);


	// ��ģ��ͼ�ϡ��±�Ե����ɫ
	//kxCImageBuf bigtemplate;

	//const int headbotextend = 100;

	//bigtemplate.Init(erodetemplate.nWidth, erodetemplate.nHeight + headbotextend * 2);

	//ippsSet_8u(0, bigtemplate.buf, bigtemplate.nPitch * bigtemplate.nHeight);

	//IppiSize templatesize = { erodetemplate.nWidth, erodetemplate.nHeight };

	//ippiCopy_8u_C1R(erodetemplate.buf, erodetemplate.nPitch, bigtemplate.buf + bigtemplate.nPitch * headbotextend, bigtemplate.nPitch, templatesize);

	
	//4. �ص㲿�֣��Լ��ͼ����N�ηָ��Сͼ������ָ,������N��Сͼ��ģ��ͼ�Ͻ��л����Լ�

	const int nsplittimes = 16;

	kxCImageBuf bigCCImg;

	bigCCImg.Init(resizesrc.nWidth, resizesrc.nHeight);

	// ��һ������
	//IppiSize masksize = {200, 200};
	//ippiSet_8u_C1R(0, resizesrc.buf + 290 + 624 * resizesrc.nPitch, resizesrc.nPitch, masksize);


	for (int i = 0; i < nsplittimes; i++)
	{
		// (1) �ü����ͼ
		kxCImageBuf smallimg;

		kxRect<int> cutrect;

		int nstep = resizesrc.nHeight / nsplittimes;

		cutrect.setup(0, nstep * i, resizesrc.nWidth - 1, nstep * (i + 1) - 1);

		kxRect<int> extendrect = cutrect;

		const int nextend = 50;

		extendrect.top = max(0, extendrect.top - nextend);

		extendrect.bottom = min(resizesrc.nHeight - 1, extendrect.top + nextend);
		
		smallimg.Init(extendrect.Width(), extendrect.Height());

		m_hFun.KxCopyImage(resizesrc, smallimg, extendrect);

		// (2) �ü�ģ��ͼ��ģ��ͼ��������������

		kxCImageBuf smalltemplate;

		smalltemplate.Init(cutrect.Width(), cutrect.Height());

		m_hFun.KxCopyImage(erodetemplate, smalltemplate, cutrect);

		kxCImageBuf CCImg;

		CCImg.Init(smalltemplate.nWidth, smalltemplate.nHeight);

		SliderSub(smallimg, smalltemplate, CCImg, 5);

		IppiSize cutsize = { CCImg.nWidth, CCImg.nHeight };

		ippiCopy_8u_C1R(CCImg.buf, CCImg.nPitch, bigCCImg.buf + bigCCImg.nPitch * nstep * i, bigCCImg.nPitch, cutsize);

	}


}


void CGlueCheck::SliderSub(kxCImageBuf& SrcImg, kxCImageBuf& TemplateImg, kxCImageBuf& dstCCImg, int nscalefactor)
{

	//1 .����ͼ��

	//if (SrcImg.nHeight > TemplateImg.nHeight || SrcImg.nWidth < TemplateImg.nWidth)
	//{
	//	std::cout << "ԭͼ�ߴ粻����Ҫ���޷����л����в�" << std::endl;

	//	return;//�ȱ����İѲ�����Ҫ���ֱ�ӷ���
	//}

	//int nexpand = (SrcImg.nWidth - TemplateImg.nWidth) / 2 + 40;

	const int nexpand = 50;

	kxCImageBuf bigsrc;

	bigsrc.Init(nexpand * 2 + SrcImg.nWidth, SrcImg.nHeight);

	ippsSet_8u(0, bigsrc.buf, bigsrc.nPitch * bigsrc.nHeight);

	IppiSize templatesize = { SrcImg.nWidth, SrcImg.nHeight };

	ippiCopy_8u_C1R(SrcImg.buf, SrcImg.nPitch, bigsrc.buf + nexpand, bigsrc.nPitch, templatesize);



	//1. ѹ��ͼ��
	kxCImageBuf resize1, resize2;

	resize1.Init(bigsrc.nWidth / nscalefactor, bigsrc.nHeight / nscalefactor);

	resize2.Init(TemplateImg.nWidth / nscalefactor, TemplateImg.nHeight / nscalefactor);

	m_hFun.KxResizeImage(bigsrc, resize1);

	m_hFun.KxResizeImage(TemplateImg, resize2);

	int nxtimes = gMax(0, resize2.nWidth - resize1.nWidth);

	int nytimes = gMax(0, resize2.nHeight - resize1.nHeight);

	unsigned char *buf;

	Ipp64f nmin = 99999999999999999;

	kxCImageBuf subresult, minsubresult;

	subresult.Init(resize2.nWidth, resize2.nHeight);

	IppiSize size = { resize2.nWidth, resize2.nHeight };

	IppiPoint minpos = { 0, 0 };

	bool zerostatus = false;

	for (int i = 0; i <= nxtimes; i++)
	{
		if (zerostatus)
		{
			break;
		}
		for (int j = 0; j <= nytimes; j++)
		{
			buf = resize1.buf + i * resize1.nChannel + j * resize1.nPitch;

			ippiSet_8u_C1R(0, subresult.buf, subresult.nPitch, size);

			ippiSub_8u_C1RSfs(buf, resize1.nPitch, resize2.buf, resize2.nPitch, subresult.buf, subresult.nPitch, size, 0);

			Ipp64f sum;

			ippiSum_8u_C1R(subresult.buf, subresult.nWidth, size, &sum);

			if (sum < nmin)
			{
				nmin = sum;

				minpos.x = i;

				minpos.y = j;

				minsubresult.SetImageBuf(subresult, true);

				if (abs(nmin - 0) < 1e-6)
				{
					zerostatus = true;
					break;
				}
			}
		}
	}


	dstCCImg.Init(TemplateImg.nWidth, TemplateImg.nHeight);

	if (zerostatus)// ȫ���� �Խ��ֱ�Ӹ�0
	{
		ippsSet_8u(0, dstCCImg.buf, dstCCImg.nPitch * dstCCImg.nHeight);
	}
	else
	{
		minpos.x *= nscalefactor;

		minpos.y *= nscalefactor;

		buf = bigsrc.buf + minpos.x + minpos.y * bigsrc.nPitch;

		IppiSize orisize = { SrcImg.nWidth, SrcImg.nHeight };

		ippiSub_8u_C1RSfs(SrcImg.buf, SrcImg.nPitch, buf, bigsrc.nPitch, dstCCImg.buf, dstCCImg.nPitch, orisize, 0);

	}

}


void CGlueCheck::ModelFillholes(kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	//���׶�

	//1. ���׶�
	kxCImageBuf fillholes, subimg;

	fillholes.Init(SrcImg.nWidth, SrcImg.nHeight);

	m_hAlg.FillHoles(SrcImg, fillholes);

	/*if (1)// ���λ�ú�����Ҫ��������һ���������Ƿ���Ҫ�˳�����©��
	{
		const int nduanjiaonum = 100;

		const int duanjiaowidth = 400;

		const int nduanjiaoheight = 20;
		

		// 2. ��ȡ��ֵ
		IppiSize imgsize = { SrcImg.nWidth, SrcImg.nHeight};

		CKxBlobAnalyse blob;

		subimg.Init(fillholes.nWidth, fillholes.nHeight);

		ippiSub_8u_C1RSfs(SrcImg.buf, SrcImg.nPitch, fillholes.buf, fillholes.nPitch, subimg.buf, subimg.nPitch, imgsize, 0);

		// 3. blob����
		blob.ToBlobByCV(subimg, CKxBlobAnalyse::_SORT_BYDOTS, 100);

		int nblobcount = blob.GetBlobCount();

		int ntargetblob = 0;

		for (int i = 0; i < blob.GetBlobCount(); i++)
		{
			CKxBlobAnalyse::SingleBlobInfo blobinfo;

			blobinfo = blob.GetSortSingleBlob(i);
			//std::cout << blobinfo.m_rc.left << blobinfo.m_rc.top << blobinfo.m_rc.right << blobinfo.m_rc.bottom << std::endl;

			if (blobinfo.m_rc.Width() > duanjiaowidth && blobinfo.m_rc.Height() > nduanjiaoheight )
			{
				kxCImageBuf roiimg;

				blobinfo.m_rc.left = gMax(blobinfo.m_rc.left - 50, 0);

				blobinfo.m_rc.right = gMin(blobinfo.m_rc.right + 50, subimg.nWidth - 1);

				blob.GetBlobImage(blobinfo.m_nLabel, blobinfo.m_rc, roiimg);

				kxCImageBuf dilatesmallimg;

				dilatesmallimg.Init(roiimg.nWidth, roiimg.nHeight);

				m_hFun.KxDilateImage(roiimg, dilatesmallimg, 21, 21);

				IppiSize roisize = { roiimg.nWidth, roiimg.nHeight };

				kxCImageBuf subresult;

				subresult.Init(roiimg.nWidth, roiimg.nHeight);

				ippiSub_8u_C1IRSfs(dilatesmallimg.buf, dilatesmallimg.nPitch, fillholes.buf + blobinfo.m_rc.left + blobinfo.m_rc.top * fillholes.nPitch, fillholes.nPitch, roisize, 0);

				ntargetblob += 1;

				if (ntargetblob > nduanjiaonum)
				{
					break;
				}
			}
		}

	}
	*/

	//kxCImageBuf erodeimg;
	//m_hFun.KxErodeImage(fillholes, erodeimg, 15, 15);
	DstImg.SetImageBuf(fillholes, true);

}


void CGlueCheck::CreateBaseModel(const kxCImageBuf& CheckImg, const kxCImageBuf& gluemask, kxCImageBuf& DstHigh, kxCImageBuf& DstLow)
{
	/*
		���������趨����ɫ��Ȼ��в�����ȥ������
	*/
	//RGB
	const unsigned char Bbase = 45;

	const unsigned char Gbase = 55;

	const unsigned char Rbase = 30;
	//HSV
	//const unsigned char Rbase = 52;

	//const unsigned char Gbase = 127;

	//const unsigned char Bbase = 83;

	DstHigh.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);
	

	IppiSize imgsize = { CheckImg.nWidth, CheckImg.nHeight };

	kxCImageBuf ImgR, ImgB, ImgG;

	ImgR.Init(CheckImg.nWidth, CheckImg.nHeight);

	ImgG.Init(CheckImg.nWidth, CheckImg.nHeight);

	ImgB.Init(CheckImg.nWidth, CheckImg.nHeight);

	ippsSet_8u(Bbase, ImgR.buf, ImgR.nPitch * ImgR.nHeight);

	ippsSet_8u(Gbase, ImgG.buf, ImgG.nPitch * ImgG.nHeight);

	ippsSet_8u(Rbase, ImgB.buf, ImgB.nPitch * ImgB.nHeight);

	ippiAnd_8u_C1IR(gluemask.buf, gluemask.nPitch, ImgR.buf, ImgR.nPitch, imgsize);

	ippiAnd_8u_C1IR(gluemask.buf, gluemask.nPitch, ImgG.buf, ImgG.nPitch, imgsize);

	ippiAnd_8u_C1IR(gluemask.buf, gluemask.nPitch, ImgB.buf, ImgB.nPitch, imgsize);

	unsigned char * pbuf1[3] = { ImgR.buf,  ImgG.buf,  ImgB.buf };

	ippiCopy_8u_P3C3R(pbuf1, ImgR.nPitch, DstHigh.buf, DstHigh.nPitch, imgsize);


	kxCImageBuf Erodemask;

	Erodemask.Init(gluemask.nWidth, gluemask.nHeight);

	m_hFun.KxErodeImage(gluemask, Erodemask, 9, 9);


	ippiAnd_8u_C1IR(Erodemask.buf, Erodemask.nPitch, ImgR.buf, ImgR.nPitch, imgsize);

	ippiAnd_8u_C1IR(Erodemask.buf, Erodemask.nPitch, ImgG.buf, ImgG.nPitch, imgsize);

	ippiAnd_8u_C1IR(Erodemask.buf, Erodemask.nPitch, ImgB.buf, ImgB.nPitch, imgsize);

	unsigned char * pbuf2[3] = { ImgR.buf,  ImgG.buf,  ImgB.buf };

	DstLow.Init(CheckImg.nWidth, CheckImg.nHeight, 3);

	ippiCopy_8u_P3C3R(pbuf2, ImgR.nPitch, DstLow.buf, DstLow.nPitch, imgsize);


	
}


void CGlueCheck::GetMaxGray(const kxCImageBuf& ColorImg, kxCImageBuf& DstGrayImg)
{
	kxCImageBuf gray[3];

	m_hAlg.SplitRGB(ColorImg, gray);

	m_hFun.KxMaxEvery(gray[0], gray[1]);

	m_hFun.KxMaxEvery(gray[1], gray[2]);

	DstGrayImg.SetImageBuf(gray[2], true);
}


int CGlueCheck::JudgeDefectType(kxCImageBuf& blobimg,  kxRect<int>rect)
{
	//kxCImageBuf ;
	kxCImageBuf smallsrcimg;

	smallsrcimg.Init(rect.Width(), rect.Height(), 3);

	m_hFun.KxCopyImage(m_ImgCheck, smallsrcimg, rect);

	kxCImageBuf maskimg;

	maskimg.Init(blobimg.nWidth, blobimg.nHeight);

	m_hFun.KxThreshImage(blobimg, maskimg, 1, 255);

	kxCImageBuf rgb[3];

	m_hAlg.SplitRGB(smallsrcimg, rgb);

	int average[3];

	for (int i = 0; i < 3; i++)
	{
		kxCImageBuf targetimg;

		targetimg.Init(maskimg.nWidth, maskimg.nHeight);

		IppiSize smallsize = { maskimg.nWidth, maskimg.nHeight };

		ippiAnd_8u_C1R(maskimg.buf, maskimg.nPitch, rgb[i].buf, rgb[i].nPitch, targetimg.buf, targetimg.nPitch, smallsize);

		Ipp64f sum, alldots;

		ippiSum_8u_C1R(targetimg.buf, targetimg.nPitch, smallsize, &sum);

		ippiSum_8u_C1R(maskimg.buf, maskimg.nPitch, smallsize, &alldots);

		alldots = alldots / 255;

		average[i] = sum / alldots;

	}

	static int nsavenum = 0;

	std::cout << nsavenum << "    ƽ���Ҷ�ֵ�� " << average[0] << "   " << average[1] << "   " << average[2] << std::endl;

	nsavenum++;

	
	if (average[0] > 180)
	{
		return 1;// ����
	}
	else
	{
		return 0;
	}


}


void CGlueCheck::GetHmask(const kxCImageBuf& SrcImg, kxCImageBuf& DstMask)
{
	kxCImageBuf maximg;

	GetMaxGray(SrcImg, maximg);

	DstMask.Init(SrcImg.nWidth, SrcImg.nHeight);

	m_hFun.KxThreshImage(maximg, DstMask, 80, 255);



}


void CGlueCheck::checkwithmodelNew(const kxCImageBuf& SrcImg, const kxCImageBuf& gluearea, kxRect<int> ROI, kxCImageBuf& dsthigh, kxCImageBuf& dstlow, kxCImageBuf& dstH)
{
	/* ���ݱ�׼ģ�壬�Լ��õ�ɫ����ߵͽ������dsthigh, dstlow
	SrcImg ���ͼ
	gluearea �������
	ROI  ����ǰλ��
	dsthigh ��߲в�
	dstlow  ��Ͳв�
	dstH    ��ɫ��
	*/

	//1����������ģ��
	CreateBaseModel(SrcImg, gluearea, m_ImgTemplateHigh, m_ImgTemplateLow);

	char savepath[64];

	IppiSize imgsize = { SrcImg.nWidth, SrcImg.nHeight };

	kxCImageBuf CheckImg;

	CheckImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	Ipp8u* pbuf[3] = { gluearea.buf, gluearea.buf, gluearea.buf };

	m_ImgColorGlueMask.Init(SrcImg.nWidth, SrcImg.nHeight, 3);

	ippiCopy_8u_P3C3R(pbuf, gluearea.nPitch, m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, imgsize);

	ippiAnd_8u_C3R(SrcImg.buf, SrcImg.nPitch, m_ImgColorGlueMask.buf, m_ImgColorGlueMask.nPitch, CheckImg.buf, CheckImg.nPitch, imgsize);



	// 2. ��ȡ������
	unsigned char poffsetlow[3] = { m_param.m_noffsetlow, m_param.m_noffsetlow, m_param.m_noffsetlow };// ��ģ�����RGB

	unsigned char poffsethigh[3] = { m_param.m_noffsethigh, m_param.m_noffsethigh, m_param.m_noffsethigh };// ��ģ�����RGB


	//3, �õ���ֵ
	m_ImgCheckHigh.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);

	ippiSub_8u_C3RSfs(m_ImgTemplateHigh.buf, m_ImgTemplateHigh.nPitch, CheckImg.buf, CheckImg.nPitch, m_ImgCheckHigh.buf, m_ImgCheckHigh.nPitch, imgsize, 0);

	ippiSubC_8u_C3IRSfs(poffsethigh, m_ImgCheckHigh.buf, m_ImgCheckHigh.nPitch, imgsize, 0);


	//4. �õ���ֵ
	m_ImgCheckLow.Init(CheckImg.nWidth, CheckImg.nHeight, CheckImg.nChannel);

	ippiSub_8u_C3RSfs(CheckImg.buf, CheckImg.nPitch, m_ImgTemplateLow.buf, m_ImgTemplateLow.nPitch, m_ImgCheckLow.buf, m_ImgCheckLow.nPitch, imgsize, 0);

	ippiSubC_8u_C3IRSfs(poffsetlow, m_ImgCheckLow.buf, m_ImgCheckLow.nPitch, imgsize, 0);


	Global_SaveDebugImg("m_ImgCheckHigh", m_ImgCheckHigh);
	
	Global_SaveDebugImg("m_ImgCheckLow", m_ImgCheckLow);


	//5. ��H ���׼ɫ�ĶԱȣ��õ���ͬɫ����ȱ�ݣ�һ�������ڼ�ⰵȱ�ݣ�HSV�� V �Ƚϰ��Ķ�ɫ��H��ͬ������RGB������©����
	// TODO �ĳɶ�ֵ����ʽ

	cv::Mat img;

	m_hFun.KxImageBufToMat(CheckImg, img);

	cv::Mat hsv[3], hsvimg;

	cv::cvtColor(img, hsvimg, cv::COLOR_BGR2HSV);

	cv::split(hsvimg, hsv);

	kxCImageBuf himg;

	m_hFun.MatToKxImageBuf(hsv[0], himg, true);

	kxCImageBuf h1, h2;// h1 Ϊ�Ҷȵ�ģ�棬 h2Ϊ�Ҷȸ�ģ��

	h1.Init(CheckImg.nWidth, CheckImg.nHeight);

	h2.Init(CheckImg.nWidth, CheckImg.nHeight);

	ippsSet_8u(gMax(0, m_param.m_nstandardH - m_param.m_noffsetcolor), h1.buf, h1.nPitch * h1.nHeight);

	ippsSet_8u(gMin(255, m_param.m_nstandardH + m_param.m_noffsetcolor), h2.buf, h2.nPitch * h2.nHeight);

	ippiAnd_8u_C1IR(gluearea.buf, gluearea.nPitch, h1.buf, h1.nPitch, imgsize);

	ippiAnd_8u_C1IR(gluearea.buf, gluearea.nPitch, h2.buf, h2.nPitch, imgsize);

	kxCImageBuf hdiff1, hdiff2;

	hdiff1.Init(CheckImg.nWidth, CheckImg.nHeight);

	hdiff2.Init(CheckImg.nWidth, CheckImg.nHeight);

	ippiSub_8u_C1RSfs(himg.buf, himg.nPitch, h1.buf, h1.nPitch, hdiff1.buf, hdiff1.nPitch, imgsize, 0);

	ippiSub_8u_C1RSfs(h2.buf, h2.nPitch, himg.buf, himg.nPitch, hdiff2.buf, hdiff2.nPitch, imgsize, 0);


	Global_SaveDebugImg("himg", himg);

	Global_SaveDebugImg("h1", h1);

	Global_SaveDebugImg("h2", h2);

	Global_SaveDebugImg("hdiff1", hdiff1);

	Global_SaveDebugImg("hdiff2", hdiff2);


	//5, ��ɫͼת��Ϊ�Ҷ�ͼ,ȡ���
	dsthigh.Init(m_ImgCheckHigh.nWidth, m_ImgCheckHigh.nHeight);

	dstlow.Init(m_ImgCheckLow.nWidth, m_ImgCheckLow.nHeight);

	kxCImageBuf maxlowimg, maxhighimg;

	maxlowimg.Init(m_ImgCheckHigh.nWidth, m_ImgCheckHigh.nHeight);

	maxhighimg.Init(m_ImgCheckHigh.nWidth, m_ImgCheckHigh.nHeight);

	GetMaxGray(m_ImgCheckHigh, maxhighimg);

	GetMaxGray(m_ImgCheckLow, maxlowimg);

	kxCImageBuf maskhigh, masklow;

	maskhigh.Init(m_ImgCheckHigh.nWidth, m_ImgCheckHigh.nHeight);

	masklow.Init(m_ImgCheckHigh.nWidth, m_ImgCheckHigh.nHeight);

	GetMaskHL(maskhigh, masklow);


	Global_SaveDebugImg("maskhigh", maskhigh);

	Global_SaveDebugImg("masklow", masklow);

	ippiSub_8u_C1IRSfs(masklow.buf, masklow.nPitch, maxlowimg.buf, maxlowimg.nPitch, imgsize, 0);

	ippiSub_8u_C1IRSfs(maskhigh.buf, maskhigh.nPitch, maxhighimg.buf, maxhighimg.nPitch, imgsize, 0);



	//6. �������ܱ��Ϸ���

	GetEdgeCorrectionMask(SrcImg, ROI, m_ImgEdgeGrayMask, m_ImgEdgeHMask);

	Global_SaveDebugImg("m_ImgEdgeGrayMask", m_ImgEdgeGrayMask);

	Global_SaveDebugImg("m_ImgEdgeHMask", m_ImgEdgeHMask);


	dstH.Init(CheckImg.nWidth, CheckImg.nHeight);

	ippiSub_8u_C1IRSfs(m_ImgEdgeHMask.buf, m_ImgEdgeHMask.nPitch, hdiff1.buf, hdiff1.nPitch, imgsize, 0);

	ippiSub_8u_C1IRSfs(m_ImgEdgeGrayMask.buf, m_ImgEdgeGrayMask.nPitch, maxhighimg.buf, maxhighimg.nPitch, imgsize, 0);


	ippiAdd_8u_C1RSfs(hdiff1.buf, hdiff1.nPitch, hdiff2.buf, hdiff2.nPitch, dstH.buf, dstH.nPitch, imgsize, 0);

	ippiSub_8u_C1IRSfs(masklow.buf, masklow.nPitch, dstH.buf, dstH.nPitch, imgsize, 0);


	Global_SaveDebugImg("dstH", dstH);

	Global_SaveDebugImg("maxlowimg", maxlowimg);

	Global_SaveDebugImg("maxhighimg", maxhighimg);


	//7. ��Щ��������
	m_hFun.KxCloseImage(maxlowimg, dstlow, 7, 7);// ����ñ����㣬��ȱ����������

	m_hFun.KxOpenImage(maxhighimg, dsthigh, 1, 7);// ���������������ཻ����ķ���



	//memset(savepath, 0, sizeof(savepath));

	//sprintf_s(savepath, "d:\\img\\dsthigh.bmp");

	//m_hFun.SaveBMPImage_h(savepath, dsthigh);

}


void CGlueCheck::GetMaskHL(kxCImageBuf& maskhigh, kxCImageBuf& masklow)
{
	// ������Ĥλ�ã�����λ�ý��и�ֵ255����
	ippsSet_8u(0, maskhigh.buf, maskhigh.nPitch * maskhigh.nHeight);

	ippsSet_8u(0, masklow.buf, masklow.nPitch * masklow.nHeight);

	for (int i = 0; i < m_param.m_vecrcHighmaskROI.size(); i++)
	{
		kxRect<int> maskrect = m_param.m_vecrcHighmaskROI[i];

		if (maskrect.left > 0 && maskrect.right < maskhigh.nWidth - 1 && maskrect.top > 0 && maskrect.bottom < maskhigh.nHeight - 1)
		{
			IppiSize masksize = { maskrect.Width(), maskrect.Height()};

			ippiSet_8u_C1R(255, maskhigh.buf + maskrect.left + maskrect.top * maskhigh.nPitch, maskhigh.nPitch, masksize);

		}
	}

	for (int i = 0; i < m_param.m_vecrcLowmaskROI.size(); i++)
	{
		kxRect<int> maskrect = m_param.m_vecrcLowmaskROI[i];

		if (maskrect.left > 0 && maskrect.right < masklow.nWidth - 1 && maskrect.top > 0 && maskrect.bottom < masklow.nHeight - 1)
		{
			IppiSize masksize = { maskrect.Width(), maskrect.Height() };

			ippiSet_8u_C1R(255, masklow.buf + maskrect.left + maskrect.top * masklow.nPitch, masklow.nPitch, masksize);

		}
	}
}


void CGlueCheck::MaskEdge(kxCImageBuf& SrcDstImg, kxRect<int> roi, int nedge)
{
	// ��Ĥ��Ե
	int ntop1 = roi.top;
	int ntop2 = gMax(0, roi.bottom - nedge);

	IppiSize masksize1 = { SrcDstImg.nWidth, nedge };
	Ipp8u value[3] = {0, 0, 0};
	ippiSet_8u_C3R(value, SrcDstImg.buf + ntop1 * SrcDstImg.nPitch, SrcDstImg.nPitch, masksize1);
	ippiSet_8u_C3R(value, SrcDstImg.buf + ntop2 * SrcDstImg.nPitch, SrcDstImg.nPitch, masksize1);

	int nleft1 = roi.left;
	int nleft2 = gMax(0, roi.right - nedge);
	IppiSize masksize2 = {nedge, SrcDstImg.nHeight};
	ippiSet_8u_C3R(value, SrcDstImg.buf + nleft1 * SrcDstImg.nChannel, SrcDstImg.nPitch, masksize2);
	ippiSet_8u_C3R(value, SrcDstImg.buf + nleft2 * SrcDstImg.nChannel, SrcDstImg.nPitch, masksize2);

}


void CGlueCheck::GetEdgeCorrectionMask(const kxCImageBuf& SrcImg, kxRect<int> roi, kxCImageBuf& dstmaskgray, kxCImageBuf& dstmaskH)
{
	dstmaskgray.Init(SrcImg.nWidth, SrcImg.nHeight);
	dstmaskH.Init(SrcImg.nWidth, SrcImg.nHeight);
	ippsSet_8u(0, dstmaskgray.buf, dstmaskgray.nPitch * dstmaskgray.nHeight);
	ippsSet_8u(0, dstmaskH.buf, dstmaskH.nPitch * dstmaskH.nHeight);
	if (roi.right > SrcImg.nWidth || roi.bottom > SrcImg.nHeight)
	{
		// ��¼������־
		return;
	}
	else
	{
		// ����ƫ�Ƹ�ֵ
		int ntop1 = roi.top;
		int nH1 = gMin(roi.top + m_param.m_nveroffsetpos, SrcImg.nHeight - 1) - ntop1;

		int ntop2 = gMax(0, roi.bottom - m_param.m_nveroffsetpos);
		int nH2 = roi.bottom - ntop2;

		IppiSize masksize1 = { roi.Width(),  nH1 };
		ippiSet_8u_C1R(m_param.m_ngrayoffset, dstmaskgray.buf + ntop1 * dstmaskgray.nPitch, dstmaskgray.nPitch, masksize1);
		ippiSet_8u_C1R(m_param.m_nHoffset, dstmaskH.buf + ntop1 * dstmaskH.nPitch, dstmaskH.nPitch, masksize1);

		masksize1 = { roi.Width(),  nH2 };
		ippiSet_8u_C1R(m_param.m_ngrayoffset, dstmaskgray.buf + ntop2 * dstmaskgray.nPitch, dstmaskgray.nPitch, masksize1);
		ippiSet_8u_C1R(m_param.m_nHoffset, dstmaskH.buf + ntop2 * dstmaskH.nPitch, dstmaskH.nPitch, masksize1);


		// ����ƫ�Ƹ�ֵ
		int nleft1 = roi.left;
		int nW1 = gMin(roi.left + m_param.m_nhoroffsetpos, SrcImg.nWidth - 1) - nleft1;

		int nleft2 = gMax(0, roi.right - m_param.m_nhoroffsetpos);
		int nW2 = roi.right - nleft2;

		IppiSize masksize2 = {nW1, roi.Height()};
		ippiSet_8u_C1R(m_param.m_ngrayoffset, dstmaskgray.buf + nleft1, dstmaskgray.nPitch, masksize2);
		ippiSet_8u_C1R(m_param.m_nHoffset, dstmaskH.buf + nleft1, dstmaskH.nPitch, masksize2);

		masksize2 = { nW2, roi.Height() };
		ippiSet_8u_C1R(m_param.m_ngrayoffset, dstmaskgray.buf + nleft2, dstmaskgray.nPitch, masksize2);
		ippiSet_8u_C1R(m_param.m_nHoffset, dstmaskH.buf + nleft2, dstmaskH.nPitch, masksize2);

	}


}


int CGlueCheck::Check(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg, Json::Value &checkresult)
{

	
	// 1. �ҵ����λ��

	kxRect <int> checkarea;

	float frotateangle = 0;

	GetTargetROI(SrcImgA, m_param.m_rcCheckROI, checkarea, frotateangle);

	if (checkarea.Width() > DstImg.nWidth || checkarea.Height() > DstImg.nHeight)
	{
		std::cout << "�ѱ߽����ʾ������� > �������ô�С���޷����м��" << std::endl;
		return 0;// ���ڼ���־���Լ��쳣����
	}

	
	// 2.Ŀ������ͼ��ϲ������˹���ͼ������

	m_ImgMerge.Init(DstImg.nWidth, DstImg.nHeight, SrcImgA.nChannel);

	ippsSet_8u(0, m_ImgMerge.buf, m_ImgMerge.nHeight * m_ImgMerge.nPitch);

	MergeImgNew(SrcImgA, SrcImgB, checkarea, m_ImgMerge);

	Global_SaveDebugImg("m_ImgMerge", m_ImgMerge);

	// 3. pack���ܴ�����б��������ת
	cv::Mat src = cv::Mat(m_ImgMerge.nHeight, m_ImgMerge.nWidth, CV_8UC(m_ImgMerge.nChannel), m_ImgMerge.buf, m_ImgMerge.nPitch);

	cv::Mat rotatematri;

	rotatematri = cv::getRotationMatrix2D(cv::Point2f(0, 0), frotateangle - 90, 1);

	cv::Mat dst;

	cv::warpAffine(src, dst, rotatematri, cv::Size(src.cols, src.rows));

	m_hFun.MatToKxImageBuf(dst, m_ImgCheck, true);

	Global_SaveDebugImg("m_ImgCheck", m_ImgCheck);


	// 4. ���������Ҹ�����Ĥ10�����أ�Ŀ�����˳�pack��Ե��ȡ��Ӱ�죨����߼��Ƚϴֲڣ������Ƿ�ĵ���
	kxRect<int> currect = checkarea;
	
	currect.offset(-checkarea.left, -checkarea.top);//��������ڴ�ͼ�е�λ��

	MaskEdge(m_ImgCheck, currect, 10);

	Global_SaveDebugImg("m_ImgCheckAfterMask", m_ImgCheck);

	// 5.��ȡͿ������
	m_hAlg.SplitRGB(m_ImgCheck, m_ImgRGB);

	m_ImgGlueMask.Init(m_ImgRGB[0].nWidth, m_ImgRGB[0].nHeight, m_ImgRGB[0].nChannel);

	ExtractGreenNew(m_ImgRGB, m_ImgGlueMask);



	// 6.������ضϽ�,���õ����Ĳв�Լ�����ΪͿ���ǹ���,��Ե����Ĵ���Ƚ�����(һ�ַ�����ģ��������,��ϸ��; ��һ����ģ��ͼ�����ֽ�)

	m_Blobimg1.Init(m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight);

	ippsSet_8u(0, m_Blobimg1.buf, m_Blobimg1.nPitch * m_Blobimg1.nHeight);
	//SliderMatchNew(m_ImgGlueMask, checkarea.Width(), checkarea.Height(), m_param.m_ImgTemplate, blobimg1);


	// 7. ��ģ��������׶�����, ������Ϊ������Щȱ�ݲ��ᱻ����Ϳ����ȡ����,������Ҫ�պϳ�һ��ģ��
	m_ImgGlueMaskFillholes.Init(m_ImgGlueMask.nWidth, m_ImgGlueMask.nHeight);

	ModelFillholes(m_ImgGlueMask, m_ImgGlueMaskFillholes);
	//m_ImgGlueMaskFillholes.SetImageBuf(m_ImgGlueMask, true);// 2022.3.20 ֱ����


	// 8. �����ɫ��ͬ������
	checkwithmodelNew(m_ImgCheck, m_ImgGlueMaskFillholes, currect, m_Blobimg2, m_Blobimg3, m_Blobimg4);



	// 7. ������ͨ���Ĳв�ͼ�ŵ�һ�� blobimg1 �Ǵ����Ϳ��ȱ��ȱ����Ҫ����blob��blobimg2��blobimg3�Ǽ�߼������ͼ�񣬿��Ժϲ���blob
	checkresult["defect num"] = 0;

	m_hFun.KxAddImage(m_Blobimg1, m_Blobimg2);

	m_hFun.KxAddImage(m_Blobimg2, m_Blobimg3);

	m_hFun.KxAddImage(m_Blobimg3, m_Blobimg4);


	CutImg2MulImg(m_Blobimg4);
	
	ParallelBlob(checkresult);

	checkresult["blockarea"] = checkarea.Width() * checkarea.Height();

	cv::Mat notzeromat;

	m_hFun.KxImageBufToMat(m_ImgGlueMask, notzeromat);

	int nresult = cv::countNonZero(notzeromat);

	checkresult["area"] = nresult;



	DstImg.SetImageBuf(m_ImgCheck, true);
	
	return 1;
}


