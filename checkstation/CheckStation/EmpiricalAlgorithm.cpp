#include "stdafx.h"
#include "EmpiricaAlgorithm.h"


CEmpiricaAlgorithm::CEmpiricaAlgorithm()
{

}

CEmpiricaAlgorithm::~CEmpiricaAlgorithm()
{

}


int CEmpiricaAlgorithm::ZSConvertImageLayer(const unsigned char* srcBuf, int srcPitch, int nImgType,
	unsigned char* dstBuf, int dstPitch, int width, int height, int nChangeType, KxCallStatus& hCall)
{
	IppStatus status = ippStsNoErr;
	hCall.Clear();
	//Nothing to compare!
	if (check_sts(status, "KxConvertImageLayer", hCall))
	{
		return 0;
	}

	if (nImgType == _Type_G8)
	{
		IppiSize roiSize = { width, height };
		ippiCopy_8u_C1R(srcBuf, srcPitch, dstBuf, dstPitch, roiSize);
	}
	if (nImgType == _Type_G24)
	{
		IppiSize Roi = { width, height };

		if (LAB_L <= nChangeType && nChangeType <= LAB_B)
		{
			m_ImgLAB.Init(width, height, 3);
			ippiBGRToLab_8u_C3R(srcBuf, srcPitch, m_ImgLAB.buf, m_ImgLAB.nPitch, Roi);
		}

		if (RGB_R == nChangeType)
		{
			ippiCopy_8u_C3C1R(srcBuf + 2, srcPitch, dstBuf, dstPitch, Roi);

		}
		else if (RGB_G == nChangeType)
		{
			ippiCopy_8u_C3C1R(srcBuf + 1, srcPitch, dstBuf, dstPitch, Roi);
		}
		else if (RGB_B == nChangeType)
		{
			ippiCopy_8u_C3C1R(srcBuf, srcPitch, dstBuf, dstPitch, Roi);

		}
		else if (RGB_GRAY == nChangeType)
		{
			ippiRGBToGray_8u_C3C1R(srcBuf, srcPitch, dstBuf, dstPitch, Roi);
		}

		else if (HSV_H == nChangeType)
		{
			m_ImgHSV.Init(width, height, 3);
			ippiRGBToHSV_8u_C3R(srcBuf, srcPitch, m_ImgHSV.buf, m_ImgHSV.nPitch, Roi);
			ippiCopy_8u_C3C1R(m_ImgHSV.buf, m_ImgHSV.nPitch, dstBuf, dstPitch, Roi);
		}
		else if (HSV_S == nChangeType)
		{
			m_ImgHSV.Init(width, height, 3);
			ippiRGBToHSV_8u_C3R(srcBuf, srcPitch, m_ImgHSV.buf, m_ImgHSV.nPitch, Roi);
			ippiCopy_8u_C3C1R(m_ImgHSV.buf + 1, m_ImgHSV.nPitch, dstBuf, dstPitch, Roi);
		}
		else if (HSV_V == nChangeType)
		{
			m_ImgHSV.Init(width, height, 3);
			ippiRGBToHSV_8u_C3R(srcBuf, srcPitch, m_ImgHSV.buf, m_ImgHSV.nPitch, Roi);
			ippiCopy_8u_C3C1R(m_ImgHSV.buf + 2, m_ImgHSV.nPitch, dstBuf, dstPitch, Roi);
		}
		else if (LAB_L == nChangeType)
		{
			ippiCopy_8u_C3C1R(m_ImgLAB.buf, m_ImgLAB.nPitch, dstBuf, dstPitch, Roi);
		}
		else if (LAB_A == nChangeType)
		{
			ippiCopy_8u_C3C1R(m_ImgLAB.buf + 1, m_ImgLAB.nPitch, dstBuf, dstPitch, Roi);
		}
		else if (LAB_B == nChangeType)
		{
			ippiCopy_8u_C3C1R(m_ImgLAB.buf + 2, m_ImgLAB.nPitch, dstBuf, dstPitch, Roi);
		}
		else if (BGR_GRAY == nChangeType)
		{
			m_ImgBGR.Init(width, height, 3);
			int order[3] = { 2, 1, 0 };
			ippiSwapChannels_8u_C3R(srcBuf, srcPitch, m_ImgBGR.buf, m_ImgBGR.nPitch, Roi, order);
			ippiRGBToGray_8u_C3C1R(m_ImgBGR.buf, m_ImgBGR.nPitch, dstBuf, dstPitch, Roi);
		}


	}
	return 1;
}

int CEmpiricaAlgorithm::ZSConvertImageLayer(const cv::Mat& SrcImg, cv::Mat& DstImg, int nChangeType)
{
	//kxCImageBuf kxsrcimg, kxdstimg;
	//kxsrcimg.SetImageBuf(SrcImg.data, SrcImg.cols, SrcImg.rows, SrcImg.step, SrcImg.channels(), false);
	//kxdstimg.SetImageBuf(DstImg.data, DstImg.cols, DstImg.rows, DstImg.step, DstImg.channels(), false);
	KxCallStatus hCall;
	DstImg.create(SrcImg.rows, SrcImg.cols, CV_8UC1);
	return ZSConvertImageLayer(SrcImg.data, SrcImg.step, SrcImg.channels(), DstImg.data, DstImg.step, SrcImg.cols, SrcImg.rows, nChangeType, hCall);
}


void CEmpiricaAlgorithm::CreatProjectImg(const kxCImageBuf& SrcImg, cv::Mat& DstImg, int nDir, int nscalefactor)//投影得到单行或单列图像，并进行归一化
{
	//nDir   _Horizontal_Project_Dir = 1 代表水平，_Vertical_Project_Dir = 0 代表垂直
	cv::Mat projectimg;
	if (nDir == 0)
	{
		projectimg.create(1, SrcImg.nWidth, CV_32F);
	}
	else
	{
		projectimg.create(1, SrcImg.nHeight, CV_32F);
	}

	m_hfun.KxProjectImage(SrcImg, nDir, (Ipp32f*)projectimg.data, nscalefactor);
	//cv::normalize(projectimg, m_normalsrc, 0, 255, cv::NORM_MINMAX);
	//cv::convertScaleAbs(m_normalsrc, DstImg);
	DstImg = projectimg.clone();
}

void CEmpiricaAlgorithm::CreatProjectImg(const cv::Mat& SrcImg, cv::Mat& DstImg, int nDir, int nscalefactor)
{
	kxCImageBuf kximg;
	kximg.SetImageBuf(SrcImg.data, SrcImg.cols, SrcImg.rows, SrcImg.step, SrcImg.channels(), false);
	CreatProjectImg(kximg, DstImg, nDir, nscalefactor);
}

int CEmpiricaAlgorithm::ThreshImg(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nthresh, THRESH_TYPE nthreshtype)
{
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	IppiSize imgsize = { SrcImg.nWidth, SrcImg.nHeight };
	if (nthreshtype == _BINARY)
	{
		ippiCompareC_8u_C1R(SrcImg.buf, SrcImg.nPitch, nthresh, DstImg.buf, DstImg.nPitch, imgsize, ippCmpGreaterEq);
	}
	else if (nthreshtype == _BINARY_INV)
	{
		ippiCompareC_8u_C1R(SrcImg.buf, SrcImg.nPitch, nthresh, DstImg.buf, DstImg.nPitch, imgsize, ippCmpLessEq);
	}
	else
	{
		return 0;
	}
	return 1;
}

int CEmpiricaAlgorithm::Project2Locate(const kxCImageBuf& SrcImg, const kxCImageBuf& TemplateImg, int nDirection, float minConfidence, int& nDx, int& nDy, float& fConfidence)
{
	kxPoint<float>  posx, posy;
	if (nDirection == _Vertical_Project_Dir)
	{
		float fratiox;

		CreatProjectImg(SrcImg, m_MatSrcProjectV, _Vertical_Project_Dir, 255);
		CreatProjectImg(TemplateImg, m_MatTempProjectV, _Vertical_Project_Dir, 255);

		fratiox = Matchtemplate(posx, m_MatSrcProjectV, m_MatTempProjectV, m_imgResultcv, m_imgResizeSrc, 1);

		if (fratiox < minConfidence)
		{
			nDx = 0;
			nDy = 0;
			fConfidence = 0.0f;
			return 0;
		}
		else
		{
			fConfidence = fratiox;
			nDx = posx.x;
			nDy = posy.x;
			return 1;
		}
	}
	else
	{
		CreatProjectImg(SrcImg, m_MatSrcProjectH, _Horizontal_Project_Dir);
		CreatProjectImg(TemplateImg, m_MatTempProjectH, _Horizontal_Project_Dir);
		float fratioy = Matchtemplate(posy, m_MatSrcProjectH, m_MatTempProjectH, m_imgResultcv, m_imgResizeSrc, 1);

		if (fratioy < minConfidence)
		{
			nDx = 0;
			nDy = 0;
			fConfidence = 0.0f;
			return 0;
		}
		else
		{
			fConfidence = fratioy;
			nDx = posx.x;
			nDy = posy.x;
			return 1;
		}

	}

	
}

float CEmpiricaAlgorithm::Matchtemplate(kxPoint<float>& pos, const kxCImageBuf& src, const kxCImageBuf& templ,
	cv::Mat& tmpResult, cv::Mat& resizesrc, int resizefactor, int nconverlayer, cv::TemplateMatchModes matchmode)
{
	cv::Mat srcmat = cv::Mat(src.nHeight, src.nWidth, CV_8UC(src.nChannel), src.buf, src.nPitch);
	cv::Mat tempmat = cv::Mat(templ.nHeight, templ.nWidth, CV_8UC(templ.nChannel), templ.buf, templ.nPitch);
	return Matchtemplate(pos, srcmat, tempmat, tmpResult, resizesrc, resizefactor, nconverlayer, matchmode);
}

float CEmpiricaAlgorithm::Matchtemplate(kxPoint<float>& pos, const cv::Mat& src, const cv::Mat& templ,
	cv::Mat& tmpResult, cv::Mat& resizesrc, int resizefactor, int nconverlayer, cv::TemplateMatchModes matchmode)
{
	if (src.cols < templ.cols || src.rows < templ.rows)
	{
		return 0.0;
	}
	cv::Mat Bigsrc;
	cv::Mat templx;
	if (nconverlayer == 0)
	{
		assert(src.type() == templ.type());
		Bigsrc = src;
		templx = templ;
	}
	else
	{
		if (src.channels() != 1)
		{
			//m_hfun.KxConvertImageLayer(src, Bigsrc, nconverlayer - 1);
			ZSConvertImageLayer(src, Bigsrc, nconverlayer - 1);
		}
		else
		{
			Bigsrc = src;
		}
		if (templ.channels() != 1)
		{
			//m_hfun.KxConvertImageLayer(templ, templx, nconverlayer - 1);
			ZSConvertImageLayer(templ, templx, nconverlayer - 1);
		}
		else
		{
			templx = templ;
		}
	}



	double fmax, fmin;
	if (resizefactor == 1)
	{
		cv::matchTemplate(Bigsrc, templx, tmpResult, matchmode);
		cv::Point minLoc, maxLoc;
		cv::minMaxLoc(tmpResult, &fmin, &fmax, &minLoc, &maxLoc);
		if (matchmode == cv::TM_CCOEFF_NORMED)
		{
			pos.x = maxLoc.x;
			pos.y = maxLoc.y;
		}
		else
		{
			pos.x = minLoc.x;
			pos.y = minLoc.y;
			fmax = 1 - fmin;
		}

	}
	else
	{
		cv::resize(Bigsrc, resizesrc, cv::Size(), 1.0 / resizefactor, 1.0 / resizefactor);
		cv::matchTemplate(resizesrc, templx, tmpResult, matchmode);
		cv::Point minLoc, maxLoc;
		cv::minMaxLoc(tmpResult, &fmin, &fmax, &minLoc, &maxLoc);
		if (matchmode == cv::TM_CCOEFF_NORMED)
		{
			pos.x = maxLoc.x;
			pos.y = maxLoc.y;
		}
		else
		{
			pos.x = minLoc.x;
			pos.y = minLoc.y;
			fmax = 1 - fmin;
		}
	}

	return (float)fmax;
}

float CEmpiricaAlgorithm::Matchtemplate(kxPoint<float>& pos, const kxCImageBuf& src, const kxCImageBuf& templ, int nconverlayer, cv::TemplateMatchModes matchmode)
{
	cv::Mat srcmat = cv::Mat(src.nHeight, src.nWidth, CV_8UC(src.nChannel), src.buf, src.nPitch);
	cv::Mat tempmat = cv::Mat(templ.nHeight, templ.nWidth, CV_8UC(templ.nChannel), templ.buf, templ.nPitch);
	return Matchtemplate(pos, srcmat, tempmat, nconverlayer, matchmode);
}

float CEmpiricaAlgorithm::Matchtemplate(kxPoint<float>& pos, const cv::Mat& src, const cv::Mat& templ, int nconverlayer, cv::TemplateMatchModes matchmode)
{

	if (src.cols < templ.cols || src.rows < templ.rows)
	{
		return 0.0;
	}

	cv::Mat tmpResult;
	cv::Mat Bigsrc;
	cv::Mat templx;
	if (nconverlayer == 0)
	{
		assert(src.type() == templ.type());
		Bigsrc = src;
		templx = templ;
	}
	else
	{
		if (src.channels() != 1)
		{
			//m_hfun.KxConvertImageLayer(src, Bigsrc, nconverlayer - 1);
			ZSConvertImageLayer(src, Bigsrc, nconverlayer - 1);
		}
		else
		{
			Bigsrc = src;
		}
		if (templ.channels() != 1)
		{
			ZSConvertImageLayer(templ, templx, nconverlayer - 1);
			//m_hfun.KxConvertImageLayer(templ, templx, nconverlayer - 1);
		}
		else
		{
			templx = templ;
		}
	}

	double fmax, fmin;
	cv::matchTemplate(Bigsrc, templx, tmpResult, matchmode);
	cv::Point minLoc, maxLoc;
	cv::minMaxLoc(tmpResult, &fmin, &fmax, &minLoc, &maxLoc);
	if (matchmode == cv::TM_CCOEFF_NORMED)
	{
		pos.x = maxLoc.x;
		pos.y = maxLoc.y;
	}
	else
	{
		pos.x = minLoc.x;
		pos.y = minLoc.y;
		fmax = 1 - fmin;
	}

	return (float)fmax;
}

void CEmpiricaAlgorithm::CalculateHoleNums(const kxCImageBuf& SrcImg, int &nholenums)
{

}

void CEmpiricaAlgorithm::CalculateHoleNums(const cv::Mat& SrcImg, int &nholenums)
{
	//cv::convexHull(SrcImg, )
}

void CEmpiricaAlgorithm::MatchUseSSD(const kxCImageBuf& SrcImg, const kxCImageBuf& Templateimg, int ntype, kxPoint<int>& matchresult)
{
	if (SrcImg.nWidth < Templateimg.nWidth || SrcImg.nHeight < Templateimg.nHeight || Templateimg.nChannel != 1 || SrcImg.nChannel != 1)
	{
		matchresult.setup(0, 0);
		return;
	}
	
	IppiSize size = { Templateimg.nWidth, Templateimg.nHeight };

	int nsliderx = SrcImg.nWidth - Templateimg.nWidth;
	
	int nslidery = SrcImg.nHeight - Templateimg.nHeight;

	m_ImgSubresult.Init(size.width, size.height);

	unsigned char *buf;

	Ipp64f nmin = 99999999999999999;

	for (int i = 0; i <= nsliderx; i++)
	{
		for (int j = 0; j <= nslidery; j++)
		{
			buf = SrcImg.buf + i * SrcImg.nChannel + j * SrcImg.nPitch;
			if (ntype == 0)
			{
				ippiSub_8u_C1RSfs(Templateimg.buf, Templateimg.nPitch, buf, SrcImg.nPitch, m_ImgSubresult.buf, m_ImgSubresult.nPitch, size, 0);
			}
			else
			{
				ippiSub_8u_C1RSfs(buf, SrcImg.nPitch, Templateimg.buf, Templateimg.nPitch, m_ImgSubresult.buf, m_ImgSubresult.nPitch, size, 0);
			}

			Ipp64f sum;
			ippiSum_8u_C1R(m_ImgSubresult.buf, m_ImgSubresult.nWidth, size, &sum);
			if (sum < nmin)
			{
				nmin = sum;
				matchresult.setup(i, j);
			}
		}
	}


}

int CEmpiricaAlgorithm::ZSErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, IppiBorderType bordertype, int nbordervalue)
{
	IppStatus status;

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}
	int npSpecSize, nBufferSize;
	IppiMorphState* pMorphSpec = NULL;
	Ipp8u* pBuffer = NULL;
	if (SrcImg.nChannel == 1)
	{
		IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
		IppiSize maskSize = { nMaskWidth, nMaskHeight };

		status = ippiMorphologyBorderGetSize_8u_C1R(roiSize, maskSize, &npSpecSize, &nBufferSize);
		if (status != ippStsNoErr)
		{
			return 0;
		}

		pMorphSpec = (IppiMorphState*)ippsMalloc_8u(npSpecSize);
		pBuffer = ippsMalloc_8u(nBufferSize);

		status = ippiMorphologyBorderInit_8u_C1R(roiSize, pMask, maskSize, pMorphSpec, pBuffer);
		if (status != ippStsNoErr)
		{
			ippsFree(pBuffer);
			ippsFree(pMorphSpec);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}
			return 0;
		}
		IppiBorderType borderType = bordertype;
		status = ippiErodeBorder_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize,
			borderType, nbordervalue, pMorphSpec, pBuffer);

		ippsFree(pBuffer);
		ippsFree(pMorphSpec);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


		if (status != ippStsNoErr)
		{
			return 0;
		}

	}
	else if (SrcImg.nChannel == 3)
	{
		IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
		IppiSize maskSize = { nMaskWidth, nMaskHeight };

		status = ippiMorphologyBorderGetSize_8u_C3R(roiSize, maskSize, &npSpecSize, &nBufferSize);
		//if (check_sts(status, "KxErodeImage_Type_G24_ippiMorphologyBorderGetSize_8u_C3R", hCall))
		//{
		//	return 0;
		//}
		if (status != ippStsNoErr)
		{
			return 0;
		}

		pMorphSpec = (IppiMorphState*)ippsMalloc_8u(npSpecSize);
		pBuffer = ippsMalloc_8u(nBufferSize);

		status = ippiMorphologyBorderInit_8u_C3R(roiSize, pMask, maskSize, pMorphSpec, pBuffer);
		if (status != ippStsNoErr)
		{
			ippsFree(pBuffer);
			ippsFree(pMorphSpec);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}

			return 0;
		}
		IppiBorderType borderType = bordertype;
		Ipp8u bordervalue[3] = { nbordervalue, nbordervalue, nbordervalue };
		status = ippiErodeBorder_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize,
			borderType, bordervalue, pMorphSpec, pBuffer);

		ippsFree(pBuffer);
		ippsFree(pMorphSpec);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


		if (status != ippStsNoErr)
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	return 1;

}

int CEmpiricaAlgorithm::ZSDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, IppiBorderType bordertype, int nbordervalue)
{
	IppStatus status;

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}
	int npSpecSize, nBufferSize;
	IppiMorphState* pMorphSpec = NULL;
	Ipp8u* pBuffer = NULL;
	if (SrcImg.nChannel == _Type_G8)
	{
		IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
		IppiSize maskSize = { nMaskWidth, nMaskHeight };

		status = ippiMorphologyBorderGetSize_8u_C1R(roiSize, maskSize, &npSpecSize, &nBufferSize);
		if (status != ippStsNoErr)
		{
			return 0;
		}

		pMorphSpec = (IppiMorphState*)ippsMalloc_8u(npSpecSize);
		pBuffer = ippsMalloc_8u(nBufferSize);
		status = ippiMorphologyBorderInit_8u_C1R(roiSize, pMask, maskSize, pMorphSpec, pBuffer);
		if (status != ippStsNoErr)
		{
			ippsFree(pBuffer);
			ippsFree(pMorphSpec);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}

			return 0;
		}
		IppiBorderType borderType = bordertype;
		status = ippiDilateBorder_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize,
			borderType, nbordervalue, pMorphSpec, pBuffer);

		ippsFree(pBuffer);
		ippsFree(pMorphSpec);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


		if (status != ippStsNoErr)
		{
			return 0;
		}

	}
	else if (SrcImg.nChannel == _Type_G24)
	{
		IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
		IppiSize maskSize = { nMaskWidth, nMaskHeight };

		status = ippiMorphologyBorderGetSize_8u_C3R(roiSize, maskSize, &npSpecSize, &nBufferSize);
		if (status != ippStsNoErr)
		{
			return 0;
		}

		pMorphSpec = (IppiMorphState*)ippsMalloc_8u(npSpecSize);
		pBuffer = ippsMalloc_8u(nBufferSize);

		status = ippiMorphologyBorderInit_8u_C3R(roiSize, pMask, maskSize, pMorphSpec, pBuffer);
		if (status != ippStsNoErr)
		{
			ippsFree(pBuffer);
			ippsFree(pMorphSpec);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}

			return 0;
		}
		IppiBorderType borderType = bordertype;
		Ipp8u bordervalue[3] = { nbordervalue, nbordervalue, nbordervalue };
		status = ippiDilateBorder_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize,
			borderType, bordervalue, pMorphSpec, pBuffer);

		ippsFree(pBuffer);
		ippsFree(pMorphSpec);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


		if (status != ippStsNoErr)
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	return 1;

}

float CEmpiricaAlgorithm::MatchTemplateInBlock(const kxCImageBuf& SrcImg, const kxCImageBuf& templateimg, kxPoint<int>& pos)
{
	cv::Mat matsrc = cv::Mat(SrcImg.nHeight, SrcImg.nWidth, CV_8UC(SrcImg.nChannel), SrcImg.buf, SrcImg.nPitch);

	cv::Mat mattempl = cv::Mat(templateimg.nHeight, templateimg.nWidth, CV_8UC(templateimg.nChannel), templateimg.buf, templateimg.nPitch);

	cv::Point points;

	float rate = MatchTemplateInBlock(matsrc, mattempl, points);

	pos.x = points.x;
	
	pos.y = points.y;

	return rate;

}

float CEmpiricaAlgorithm::MatchTemplateInBlock(cv::Mat &SrcImg, cv::Mat &templateimg, cv::Point& pos)
{
	if (SrcImg.cols < templateimg.cols || SrcImg.rows < templateimg.rows)
	{
		return 0.0;
	}

	int nsearchrangex = SrcImg.cols - templateimg.cols;
	
	int nsearchrangey = SrcImg.rows - templateimg.rows;

	int nmatchtemplateblockw = templateimg.cols / 2;

	int nmatchtemplateblockh = templateimg.rows / 2;

	int nmatchsrcblockw = nmatchtemplateblockw + nsearchrangex;

	int nmatchsrcblockh = nmatchtemplateblockh + nsearchrangey;

	const int nsplitblocknum = 4;

	cv::Mat matchsrc[nsplitblocknum], matchtemp[nsplitblocknum], matchresult[nsplitblocknum];
	
	kxPoint<float> matchpos[nsplitblocknum];

	matchsrc[0] = SrcImg(cv::Rect(0, 0, nmatchsrcblockw, nmatchsrcblockh));

	matchsrc[1] = SrcImg(cv::Rect(SrcImg.cols - nmatchsrcblockw, 0, nmatchsrcblockw, nmatchsrcblockh));

	matchsrc[2] = SrcImg(cv::Rect(0, SrcImg.rows - nmatchsrcblockh, nmatchsrcblockw, nmatchsrcblockh));
	
	matchsrc[3] = SrcImg(cv::Rect(SrcImg.cols - nmatchsrcblockw, SrcImg.rows - nmatchsrcblockh, nmatchsrcblockw, nmatchsrcblockh));

	matchtemp[0] = templateimg(cv::Rect(0, 0, nmatchtemplateblockw, nmatchtemplateblockh));

	matchtemp[1] = templateimg(cv::Rect(templateimg.cols - nmatchtemplateblockw, 0, nmatchtemplateblockw, nmatchtemplateblockh));

	matchtemp[2] = templateimg(cv::Rect(0, templateimg.rows - nmatchtemplateblockh, nmatchtemplateblockw, nmatchtemplateblockh));

	matchtemp[3] = templateimg(cv::Rect(templateimg.cols - nmatchtemplateblockw, templateimg.rows - nmatchtemplateblockh, nmatchtemplateblockw, nmatchtemplateblockh));

	cv::Mat resizesrc;

	for (int i = 0; i < nsplitblocknum; i++)
	{
		Matchtemplate(matchpos[i], matchsrc[i], matchtemp[i], matchresult[i], resizesrc, 1, 0, cv::TM_SQDIFF);
	}



	
	// 旧方法，四个遍历一遍
	/*
	for (int i = 0; i < nsplitblocknum; i++)
	{
		int x = matchpos[i].x;
		
		int y = matchpos[i].y;

		float mactchadd = 0;

		for (int j = 0; j < nsplitblocknum; j++)
		{
			mactchadd += matchresult[j].at<float>(y, x);
		}

		if (mactchadd > fmatchmax)
		{
			fmatchmax = mactchadd;

			bestpos.x = x;

			bestpos.y = y;
		}
	}

	pos.x = bestpos.x;

	pos.y = bestpos.y;
	*/

	/*
	std::vector<int> vectorx, vectory;

	vectorx.push_back(matchpos[0].x);

	vectory.push_back(matchpos[0].y);

	for (int i = 1; i < nsplitblocknum; i++)
	{
		int curx = matchpos[i].x;
		int cury = matchpos[i].y;

		bool brepeate = false;
		for (int k = 0; k < vectorx.size(); k++)
		{
			if (curx == vectorx[k])
			{
				brepeate = true;
				break;
			}
		}
		if (!brepeate)
		{
			vectorx.push_back(curx);
		}

		brepeate = false;
		for (int k = 0; k < vectory.size(); k++)
		{
			if (cury == vectory[k])
			{
				brepeate = true;
				break;
			}
		}
		if (!brepeate)
		{
			vectory.push_back(cury);
		}
	}

	float fmatchmax = 9999999999999999;


	cv::Point bestpos;
	
	for (int i = 0; i < vectorx.size(); i++)
	{
		for (int j = 0; j < vectory.size(); j++)
		{
			//std::cout << i << "  " << j << std::endl;
			int x = vectorx[i];

			int y = vectory[j];

			float mactchadd = 0;

			for (int k = 0; k < nsplitblocknum; k++)
			{
				mactchadd += matchresult[k].at<float>(y, x);
			}


			if (mactchadd < fmatchmax)
			{
				fmatchmax = mactchadd;

				bestpos.x = x;

				bestpos.y = y;
			}

		}
	}*/

	// 第三个方法
	
	/*
	cv::Mat addresult = cv::Mat(matchresult[0].rows, matchresult[0].cols, matchresult[0].type(), cv::Scalar(0));
	
	for (int i = 0; i < nsplitblocknum; i++)
	{
		cv::Mat result;
		cv::add(addresult, matchresult[i], result);
		addresult = result;
	}


	cv::Point minLoc, maxLoc;
	double fmin;
	double fmax;
	cv::minMaxLoc(addresult, &fmin, &fmax, &minLoc, &maxLoc);

	pos.x = minLoc.x;

	pos.y = minLoc.y;
	*/

	// 第四个方法
	cv::Rect minrect[nsplitblocknum];

	const int nextend = 2;

	for (int i = 0; i < nsplitblocknum; i++)
	{
		int nleft = MAX(0, matchpos[i].x - nextend);
		int ntop = MAX(0, matchpos[i].y - nextend);
		int nright = MIN(matchresult[i].cols - 1, matchpos[i].x + nextend);
		int nbottom = MIN(matchresult[i].cols - 1, matchpos[i].y + nextend);
		minrect[i] = cv::Rect(nleft, ntop, nright - nleft + 1, nbottom - ntop + 1);
	}

	double bestmatch = 9999999999999999999;
	int bestindex = 0;

	for (int i = 0; i < nsplitblocknum; i++)
	{
		double mactchadd = 0;

		for (int j = 0; j < nsplitblocknum; j++)
		{
			//mactchadd += matchresult[k].at<float>(y, x);
			cv::Mat cut = matchresult[j](minrect[i]);
			//cv::Point minLoc, maxLoc;
			//double fmin;
			//double fmax;
			//cv::minMaxLoc(cut, &fmin, &fmax, &minLoc, &maxLoc);
			cv::Scalar val = cv::sum(cut);
			//val[0]
			mactchadd += val[0];
		}
		if (bestmatch > mactchadd)
		{
			bestmatch = mactchadd;
			bestindex = i;
		}
	}
	
	pos.x = matchpos[bestindex].x;

	pos.y = matchpos[bestindex].y;



	//if (matchmode == cv::TM_CCOEFF_NORMED)
	//{
	//	return fmatchmax / nsplitblocknum;
	//}
	//else
	//{
	//	return (nsplitblocknum - fmatchmax) / nsplitblocknum;
	//}
	return 0.8;

}

void CEmpiricaAlgorithm::CalEachDirectionPointNum(const kxCImageBuf& SrcImg, kxPoint<int>& centerpoint, int *presult, int nDirectionNum)
{
	cv::Mat srcmat = cv::Mat(SrcImg.nHeight, SrcImg.nWidth, CV_8UC(SrcImg.nChannel), SrcImg.buf, SrcImg.nPitch);
	cv::Point points = cv::Point(centerpoint.x, centerpoint.y);
	CalEachDirectionPointNum(srcmat, points, presult, nDirectionNum);
}

void CEmpiricaAlgorithm::CalEachDirectionPointNum(const cv::Mat& SrcImg, cv::Point& centerpoint, int *presult, int nDirectionNum)
{

}

/*
	输入一张二值化图像，centerpoint为中心，得到angle角度方向上非零数据的点数之和
*/

void CEmpiricaAlgorithm::CalSingleDirectionPointNum(const kxCImageBuf& SrcImg, kxPoint<int>& centerpoint, int angle, int &nnum)
{
	cv::Mat srcmat = cv::Mat(SrcImg.nHeight, SrcImg.nWidth, CV_8UC(SrcImg.nChannel), SrcImg.buf, SrcImg.nPitch);
	cv::Point points = cv::Point(centerpoint.x, centerpoint.y);
	CalSingleDirectionPointNum(srcmat, points, angle, nnum);
}

void CEmpiricaAlgorithm::CalSingleDirectionPointNum(const cv::Mat& SrcImg, cv::Point& centerpoint, int angle, int &nnum)
{
	float k = std::tan(angle);
}

int CEmpiricaAlgorithm::ZSFilterSpeckles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize, int nconnectivity)
{
	assert(nconnectivity != ippiNormL1 || nconnectivity != ippiNormInf);
	if (SrcImg.nChannel != _Type_G8)
	{
		return 0;
	}
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppStatus status;
	int nBufferSize = 0;
	status = ippiMarkSpecklesGetBufferSize(roiSize, ipp8u, SrcImg.nChannel, &nBufferSize);


	m_MarkImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	ippiThreshold_GTVal_8u_C1R(SrcImg.buf, SrcImg.nPitch, m_MarkImg.buf, m_MarkImg.nPitch, roiSize, 0, 20);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	//tick_count tbb_start, tbb_end;
	//tbb_start = tick_count::now();
	IppiNorm norm;
	if (nconnectivity == 4)
	{
		norm = ippiNormL1;
	}
	else
	{
		norm = ippiNormInf;
	}

	status = ippiMarkSpeckles_8u_C1IR(m_MarkImg.buf, m_MarkImg.nPitch, roiSize, 255, nMaxSpeckleSize, 10, norm, pBuffer);
	//tbb_end = tick_count::now();
	//printf("KxFilterSpeckles filter cost: %f ms\n", (tbb_end - tbb_start).seconds());

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	ThreshImg(m_MarkImg, DstImg, 1, _BINARY);
	ippiAnd_8u_C1IR(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize);

	ippsFree(pBuffer);

	return 1;
}


void CEmpiricaAlgorithm::ZSFilterSpecklesWithBlob(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize, int nconnectivity)
{
	cv::Mat srcmat = cv::Mat(SrcImg.nHeight, SrcImg.nWidth, CV_8UC(SrcImg.nChannel), SrcImg.buf, SrcImg.nPitch);

	cv::Mat dstmat;

	ZSFilterSpecklesWithBlob(srcmat, dstmat, nMaxSpeckleSize, nconnectivity);

	DstImg.SetImageBuf(dstmat.data, dstmat.cols, dstmat.rows, dstmat.step, dstmat.channels(), true);

}

void CEmpiricaAlgorithm::ZSFilterSpecklesWithBlob(const cv::Mat& SrcImg, cv::Mat& DstImg, int nMaxSpeckleSize, int nconnectivity)
{

	cv::connectedComponentsWithStats(SrcImg, m_MatLabel, m_MatState, m_MatCentroids, nconnectivity, CV_16U);

	DstImg = SrcImg.clone();

	int nblobnums = m_MatState.rows;

	cv::Mat dotscol = m_MatState.col(4);

	while (nblobnums--)
	{
		double minval, maxval;

		cv::Point minpoint, maxpoint;

		cv::minMaxLoc(dotscol, &minval, &maxval, &minpoint, &maxpoint);

		if (minpoint.y == 0)
		{
			dotscol.at<int>(minpoint.y, 0) = 999999999;

			continue;
		}

		if (minval > nMaxSpeckleSize)
		{
			break;
		}
		else
		{
			int x = m_MatState.at<int>(minpoint.y, 0);

			int y = m_MatState.at<int>(minpoint.y, 1);

			int width = m_MatState.at<int>(minpoint.y, 2);

			cv::Point seed = {x, y};

			for (int ncurx = x; ncurx < x + width; ncurx++)
			{
				if (m_MatLabel.at<ushort>(y, ncurx) == minpoint.y)
				{
					seed = { ncurx, y};

					break;
				}
			}

			cv::floodFill(DstImg, seed, 0);

			dotscol.at<int>(minpoint.y, 0) = 999999999;
		}
	}




}

void CEmpiricaAlgorithm::SplitRGB(const kxCImageBuf& SrcImg, kxCImageBuf* dstimgarray)
{
	if (SrcImg.nChannel != 3)
	{
		return;
	}

	Ipp8u* pbufarray[3];

	for (int i = 0; i < 3; i++)
	{
		dstimgarray[i].Init(SrcImg.nWidth, SrcImg.nHeight);

		pbufarray[i] = dstimgarray[i].buf;
	}

	IppiSize imgsize = { SrcImg.nWidth, SrcImg.nHeight};

	ippiCopy_8u_C3P3R(SrcImg.buf, SrcImg.nPitch, pbufarray, dstimgarray[0].nPitch, imgsize);


}

void CEmpiricaAlgorithm::FillHoles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	if (SrcImg.nChannel != 1)
		return;

	m_Matblack = cv::Mat(SrcImg.nHeight + 2, SrcImg.nWidth + 2, CV_8UC1, cv::Scalar(0));

	IppiSize imgsize = { SrcImg.nWidth, SrcImg.nHeight};

	ippiCopy_8u_C1R(SrcImg.buf, SrcImg.nPitch, m_Matblack.data + 1 + m_Matblack.step, m_Matblack.step, imgsize);

	cv::Point seed = {0, 0};

	cv::Mat blackclone = m_Matblack.clone();

	cv::floodFill(m_Matblack, seed, 255);

	IppiSize bigimgsize = { m_Matblack.cols, m_Matblack.rows};

	ippiSub_8u_C1IRSfs(blackclone.data, blackclone.step, m_Matblack.data, m_Matblack.step, bigimgsize, 0);

	cv::Mat whiteimg = cv::Mat(m_Matblack.size(), m_Matblack.type(), cv::Scalar(255));

	ippiSub_8u_C1IRSfs(m_Matblack.data, m_Matblack.step, whiteimg.data, whiteimg.step, bigimgsize, 0);

	DstImg.SetImageBuf(whiteimg.data + 1 + whiteimg.step, SrcImg.nWidth, SrcImg.nHeight, whiteimg.step, whiteimg.channels(), true);
}
