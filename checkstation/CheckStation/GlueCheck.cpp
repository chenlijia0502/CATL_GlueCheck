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
	CreateBaseModel(SrcImg);

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

	/*
	m_hBlobFun.ToBlobByCV(m_ImgCheckHighGray, CKxBlobAnalyse::_SORT_BYDOTS, 16);

	checkresult["defect num"] = 0;

	if (m_hBlobFun.GetBlobCount() > 0)
	{
		for (int i = 0; i < m_hBlobFun.GetBlobCount(); i++)
		{
			CKxBlobAnalyse::SingleBlobInfo blobinfo;

			blobinfo = m_hBlobFun.GetSortSingleBlob(i);

			if (blobinfo.m_nDots > m_param.m_ndefectdots)
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
	CutImg2MulImg(m_ImgCheckHighGray);

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
					single["pos"].append(blobinfo.m_rc.left);
					single["pos"].append(blobinfo.m_rc.top);
					single["pos"].append(blobinfo.m_rc.Width());
					single["pos"].append(blobinfo.m_rc.Height());
					checkresult["defect feature"].append(single);
					checkresult["defect num"] = checkresult["defect num"].asInt() + 1;
				}
			}
		}
	}



}


void CGlueCheck::CreateBaseModel(const kxCImageBuf& CheckImg)
{
	
	/* ����һ��  �趨һ��������Χ�����ߵ�


	// 1. ���µ�ֵ��������������û�ȡ
	


	const unsigned char Rrange[2] = { 10, 40 };

	const unsigned char Grange[2] = { 20, 60 };

	const unsigned char Brange[2] = { 10, 50 };


	// 2.��ֵ������ͼ

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
		���������趨����ɫ��Ȼ��в�����ȥ������
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

void CGlueCheck::GetGlueMask()
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
	

	/* ������   RGB תHSV��S - V �Ľ����ֵ��ȡ����� */


	//cv::Mat targetimg1 = cv::Mat(m_ImgHSV[1].nHeight, m_ImgHSV[1].nWidth, CV_8UC1, m_ImgHSV[1].buf, m_ImgHSV[1].nPitch);

	//cv::Mat targetimg2 = cv::Mat(m_ImgHSV[0].nHeight, m_ImgHSV[0].nWidth, CV_8UC1, m_ImgHSV[2].buf, m_ImgHSV[2].nPitch);


	//cv::Mat blurresult1, blurresult2, subresult, threshimg;

	//cv::blur(targetimg1, blurresult1, cv::Size(7, 7));

	//cv::blur(targetimg2, blurresult2, cv::Size(7, 7));

	//cv::subtract(blurresult1, blurresult2, subresult);

	//cv::threshold(subresult, threshimg, 50, 255, cv::THRESH_BINARY);

	//m_ImgThresh.SetImageBuf(threshimg.data, threshimg.cols, threshimg.rows, threshimg.step, threshimg.channels(), false);

	m_hAlg.ThreshImg(m_ImgHSV[1], m_ImgThresh, 95, CEmpiricaAlgorithm::_BINARY);
	
	m_hFun.KxOpenImage(m_ImgThresh, m_ImgOpen, 11, 11);

	m_hFun.KxCloseImage(m_ImgOpen, m_ImgClose, 11, 11);

	m_hBlobFun.SelectMaxRegionByDots(m_ImgClose, m_ImgmaxRegion);


	//����ֻ�������ɫ����ȡ�������һ���ⲿ��ȡ���ѿױ���

	m_hAlg.FillHoles(m_ImgmaxRegion, m_ImgGlueMask);

	//�������Ƕ���ɫ���У����Բü��� 2021.12.16 ����Ҫ����

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



	////int maskheight = m_ImgGlueMask.nHeight / 5;

	////int maskstart = m_ImgGlueMask.nHeight - maskheight;

	////IppiSize blobsize = { m_ImgGlueMask.nWidth,maskheight };

	////ippiSet_8u_C1R(0, m_ImgGlueMask.buf + maskstart * m_ImgGlueMask.nPitch, m_ImgGlueMask.nPitch, blobsize);

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

	//ȡsrcimgA��RGB���ֵ
	GetMaxGray(SrcImgA, gray1);

	ippiAnd_8u_C1IR(m_ImgGlueMask.buf, m_ImgGlueMask.nPitch, gray1.buf, gray1.nPitch, imgsize);

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


int CGlueCheck::Check(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg, Json::Value &checkresult)
{

	// ���ڵ���������ȡ��ɫ������ȡ�����ڱ�Ե���ֲ��ø㡣Ҫ����취��˫��ֵ��ȡ

	//static int nsaveindex = 0;

	//char path[64];

	//memset(path, 0, 64);
	//sprintf_s(path, "d:\\%dbig.bmp", nsaveindex);

	//m_hFun.SaveBMPImage_h(path, SrcImgA);
	//nsaveindex++;
	//memset(path, 0, 64);
	//sprintf_s(path, "d:\\%dbig.bmp", nsaveindex);
	//nsaveindex++;

	//m_hFun.SaveBMPImage_h(path, SrcImgB);


	checkresult["defect num"] = 0;

	m_hAlg.SplitRGB(SrcImgA, m_ImgRGB);

	cv::Mat hsv;

	cv::Mat src = cv::Mat(SrcImgA.nHeight, SrcImgA.nWidth, CV_8UC3, SrcImgA.buf);

	cv::cvtColor(src, hsv, cv::COLOR_RGB2HSV);

	kxCImageBuf phsv;

	phsv.SetImageBuf(hsv.data, hsv.cols, hsv.rows, hsv.step, hsv.channels(), true);

	m_hAlg.SplitRGB(phsv, m_ImgHSV);

	GetGlueMask();

	MergeImg(SrcImgA, SrcImgB, m_ImgCheck);

	checkyiwu(m_ImgCheck, checkresult);

	DstImg.SetImageBuf(SrcImgA, true);

	return 1;
}


