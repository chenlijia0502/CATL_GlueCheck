#include "KxPlateEdgeCheck.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include "global.h"


CKxPlateEdgeCheck::CKxPlateEdgeCheck()
{

}

CKxPlateEdgeCheck::~CKxPlateEdgeCheck()
{

}

bool CKxPlateEdgeCheck::ReadParaXml(const char* filePath)
{
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "全局", "极片类型", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus = KxXmlFun::FromStringToInt(szResult, m_hPara.m_nPolepiceType);
	if (!nStatus)
	{
		return false;
	}
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "极片亮度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hPara.m_nSearchEdgeThresh);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "毛刺检查亮度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hPara.m_nWhiteLineThresh);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "铝丝检查亮度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hPara.m_nOtherAreaThresh);
	if (!nStatus)
	{
		return false;
	}

	//-----------------------20190313 新添加 -----------------------//
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "断线检查亮度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hPara.m_nBrokenThresh);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "拱检查亮度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hPara.m_nArchThresh);
	if (!nStatus)
	{
		return false;
	}

	////检断是最小的阈值，因为集流体是通过检断的阈值定出在哪的
	//if (m_hPara.m_nBrokenThresh > m_hPara.m_nArchThresh)
	//{
	//	m_hPara.m_nArchThresh = m_hPara.m_nBrokenThresh;
	//}
	//if (m_hPara.m_nBrokenThresh > m_hPara.m_nWhiteLineThresh)
	//{
	//	m_hPara.m_nWhiteLineThresh = m_hPara.m_nBrokenThresh;
	//}


	return true;
}

//y = ax + b
bool CKxPlateEdgeCheck::FiltLine(Ipp32f* pX, Ipp32f* pY, int n, float& fAngle, float& a, float& b)
{
	Ipp32f pSumX, pSumY, pSumXY, pSumXX;
	ippsSum_32f(pX, n, &pSumX, ippAlgHintFast);
	ippsSum_32f(pY, n, &pSumY, ippAlgHintFast);
	ippsDotProd_32f(pX, pY, n, &pSumXY);
	ippsDotProd_32f(pX, pX, n, &pSumXX);
	if (abs(pSumX*pSumX - n*pSumXX) < 1e-8)
	{
		fAngle = 90;
		return false;
	}
	else
	{
		b = (pSumXY * pSumX - pSumY * pSumXX) / (pSumX*pSumX - n*pSumXX);
		a = (pSumY * pSumX - n*pSumXY) / (pSumX*pSumX - n*pSumXX);
		fAngle = float(atan(a) * 180 / PI);
		return true;
	}

}
//y = ax + b
bool CKxPlateEdgeCheck::FiltLine2D(kxPoint<float>* pts, int n, float& fAngle, float& a, float& b)
{
	Ipp32f* pX = new Ipp32f[n];
	Ipp32f* pY = new Ipp32f[n];

	for (int i = 0; i < n; i++)
	{
		pX[i] = pts[i].x;
		pY[i] = pts[i].y;
	}
	bool bStatus = FiltLine(pX, pY, n, fAngle, a, b);

	delete[]pX;
	delete[]pY;

	return bStatus;
}

void CKxPlateEdgeCheck::combine(int* arr, int start, int* result, int count, int num, int& nTotal, int** Result)
{
	for (int i = start; i >= count; i--)
	{
		result[count - 1] = i - 1;
		if (count > 1)
		{
			combine(arr, i - 1, result, count - 1, num, nTotal, Result);
		}
		else
		{
			int n = 0;
			for (int j = num - 1; j >= 0; j--)
			{
				Result[nTotal][n++] = arr[result[j]];
			}
			nTotal++;
		}
	}
}

//nFitLineDots采样集 10;  nFitLineDots拟合点数 3
float CKxPlateEdgeCheck::FitLineByRansac(float* pArray, int nArrayStart, int nLen, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb)
{
	//数据平均采样
	int nDots = nSampleCount;
	if (nLen < nSampleCount)
	{
		nDots = nLen;
	}

	int nStep = nLen / nDots;

	kxPoint<float>* ptFit = new kxPoint<float>[nDots];
	for (int i = 0; i < nDots; i++)
	{
		ptFit[i].x = (float)i*nStep + 1 + nArrayStart;
		ptFit[i].y = pArray[i*nStep + 1 + nArrayStart];
	}

	kxPoint<float>* pFitArray = new kxPoint<float>[nFitLineDots];

	int nMaxVoteCount = INT_MIN;
	float fInliersPercentage = 0.0;


	int N = nDots;
	int m = nFitLineDots;
	int* arr = new int[N];
	for (int i = 0; i < N; i++)
	{
		arr[i] = i;
	}
	int nFenmu = 1;
	int nFenZi = 1;
	for (int i = 0; i < m; i++)
	{
		nFenmu *= (N - i);
		nFenZi *= (i + 1);
	}
	int nTotal = nFenmu / nFenZi;

	int** result = new int*[nTotal];
	for (int i = 0; i < nTotal; i++)
	{
		result[i] = new int[m];
	}
	int* pTmp = new int[m];
	int nTotalCount = 0;
	//排列组合C10取3
	combine(arr, N, pTmp, m, m, nTotalCount, result);
	int nIterCount = 0;
	while ((nIterCount < nTotal) && (fInliersPercentage < 0.95))
	{
		//随机采样
		//GetRandomDotFromArray(ptFit, nDots, pFitArray, nFitLineDots);
		for (int i = 0; i < nFitLineDots; i++)
		{
			int nIndex = result[nIterCount][i];
			pFitArray[i] = ptFit[nIndex];
		}
		float fAngle, a, b;
		int nVoteCount = 0;
		if (FiltLine2D(pFitArray, nFitLineDots, fAngle, a, b))
		{
			for (int k = 0; k < nDots; k++)
			{
				if (abs(ptFit[k].y - a*ptFit[k].x - b) < 2)//容许误差2pixel
				{
					nVoteCount++;
				}
			}
		}
		else
		{
			for (int k = 0; k < nDots; k++)
			{
				if (abs(ptFit[k].x - pFitArray[0].x) < 2)
				{
					nVoteCount++;
				}
			}

		}

		if (nVoteCount > nMaxVoteCount)
		{
			nMaxVoteCount = nVoteCount;
			fInliersPercentage = (float)nVoteCount / nDots;
			fBesta = a;
			fBestAngle = fAngle;
			fBestb = b;
		}

		nIterCount++;
	}

	delete[]ptFit;
	delete[]pFitArray;
	delete[]arr;
	delete[]pTmp;
	for (int i = 0; i < nTotal; i++)
	{
		delete[]result[i];
		result[i] = NULL;
	}
	delete[] result;

	return fInliersPercentage;
}

int CKxPlateEdgeCheck::CheckOtherArea(const kxCImageBuf& SrcImg, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	m_hBaseFun.KxThreshImage(SrcImg, m_OtherAreaImg, m_hPara.m_nOtherAreaThresh, 255);
	//mask白线区域
	IppiSize roiSize = { m_OtherAreaImg.nWidth, m_nWhiteLineEnd - m_nWhiteLineStart + 1 };
	ippiSet_8u_C1R(0, m_OtherAreaImg.buf + m_nWhiteLineStart * m_OtherAreaImg.nPitch, m_OtherAreaImg.nPitch, roiSize);
	
	m_hBlobAnalyseResult.ToBlobParallel(m_OtherAreaImg, CKxBlobAnalyse::_SORT_BYDOTS, _MAX_DEFECT_COUNT, 1, 0, hCallInfo);
	if (check_sts(hCallInfo, "CheckOtherArea_", hCall))
	{
		return 0;
	}

	for (int i = 0; i < m_hBlobAnalyseResult.GetBlobCount(); i++)
	{
		int nIndx = (std::min)(m_hResult.nDefectCount++, (int)_MAX_DEFECT_COUNT - 1);
		if (nIndx <= _MAX_DEFECT_COUNT-1)
		{
			m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nDots*1.0f);
			m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nEnergy*1.0f); 
			m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f,m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.left* 1.0f - 30));
			m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f,m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.top* 1.0f - 50));
			m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (std::min)(m_OtherAreaImg.nWidth - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Width() + 60)* 1.0f);
			m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (std::min)(m_OtherAreaImg.nHeight - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Height() + 100)* 1.0f);
			m_hResult.mapFeaturelists[nIndx][6] = std::make_pair("位置", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.top >= m_nWhiteLineStart ? _UPER *1.0f : _DOWN*1.0f);
			m_hResult.nCheckStatus = _BLOBERR;
			//m_hResult.szErrInfo = "have some blobs";
		}

	}

	return 1;
}

int CKxPlateEdgeCheck::Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	IppStatus status;
	m_hResult.Clear();

	tick_count tbb_start, tbb_end, total_s, total_e;
	
	//1, 预处理
	kxRect<int> rcEdge;
	int funresult = PreSolve(SrcImg, DstImg, rcEdge, hCall);
	if (funresult < 0)
	{
		return funresult;//不再进行处理，因为图像可能是搜边异常
	}

	//2，拟合极片边缘，旋转校正图像
	double disT = 0;
	double disB = 0;
	funresult = FitlineAndRotateImg(SrcImg, m_CropImg, rcEdge, disT, disB, hCall);
	if (funresult == 0)
	{
		return 0;
	}
	
	////3，检测特征
	m_hResult.nEdgeStart = (int)((_EDGE_EXTEND + _EDGE_HEIGHT / 2) - disT);
	m_hResult.nEdgeEnd = (int)((_EDGE_EXTEND + _EDGE_HEIGHT / 2) + disB);
	int middle = 0;
	
	if (m_hPara.m_nPolepiceType == ZHENG)
	{
		MoveFrontFix2(m_CropImg, middle, hCallInfo);
	}
	else
	{
		MoveFrontCut(m_CropImg, middle, hCallInfo);
	}

	//4，拷贝结果，返回数据
	IppiSize roiSize = { m_CropImg.nWidth, m_CropImg.nHeight };
	DstImg.Init(m_CropImg.nWidth, m_CropImg.nHeight);
	ippiCopy_8u_C1R(m_CropImg.buf, m_CropImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize);
	IppiSize blackmask = { DstImg.nWidth, 1 };
	ippiSet_8u_C1R(0, DstImg.buf + middle*DstImg.nPitch, DstImg.nPitch, blackmask);
	m_hResult.nDefectCount = MIN(m_hResult.nDefectCount, _MAX_DEFECT_COUNT);
	
	return 1;

}

int CKxPlateEdgeCheck::PreSolve(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, kxRect<int>& rcEdge, KxCallStatus& hCall)
{

	KxCallStatus hCallInfo;
	m_CropImg.Init(SrcImg.nWidth, _EDGE_EXTEND * 2 + _EDGE_HEIGHT);
	ippsSet_8u(0, m_CropImg.buf, m_CropImg.nPitch * m_CropImg.nHeight);



	m_hBaseFun.KxConvertImageLayer(SrcImg, m_GrayImg, RGB_GRAY);

	////--------------TODO : 这里为了一个实验稍微改动下，记得改回来
	////ori
	//IppiSize blacksize1 = {m_GrayImg.nWidth, 300};
	//IppiSize blacksize2 = { m_GrayImg.nWidth, SrcImg.nHeight - 800};
	//ippiSet_8u_C1R(0, m_GrayImg.buf, m_GrayImg.nPitch, blacksize1);
	//ippiSet_8u_C1R(0, m_GrayImg.buf + 800 * m_GrayImg.nPitch, m_GrayImg.nPitch, blacksize2);
	////--------------------------------//
	//
	m_hBaseFun.KxThreshImage(m_GrayImg, m_BinaryImg, m_hPara.m_nSearchEdgeThresh, 255);

	//Blob分析确定边的大概位置
	IppiSize imgsize = { m_GrayImg.nWidth, m_GrayImg.nHeight };
	m_hBlobAnalyse.ToBlobParallel(m_BinaryImg, CKxBlobAnalyse::_SORT_BYSIZE, 4, 1, 0);
	int blobcount = m_hBlobAnalyse.GetBlobCount();
	int blobwidth = 0;
	bool bstatus = false;
	if (blobcount != 0)
	{
		blobwidth = m_hBlobAnalyse.GetSortSingleBlob(0).m_rc.Width();
		if (blobwidth != SrcImg.nWidth)
		{
			bstatus = true;
		}
	}
	// blob为空 虚焦  二值化后的图有明显的问题
	if (m_hBlobAnalyse.GetBlobCount() == 0 || bstatus)
	{
		int nIndx = (std::min)(m_hResult.nDefectCount++, (int)_MAX_DEFECT_COUNT - 1);
		m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", 100000.0f);
		m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", 10000000.0f);
		m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", 0.0f);
		m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", 0.0f);
		m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (m_CropImg.nWidth - 1)*1.0f);
		m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (m_CropImg.nHeight - 1)*1.0f);
		m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::ABNORMAL);

		m_hResult.nCheckStatus = _BLOBERR;
		m_hResult.szErrInfo = "Not find a edge area";
		kxRect<int> rc;
		rc.setup(0, m_GrayImg.nHeight / 2, SrcImg.nWidth - 1, m_GrayImg.nHeight / 2 + m_CropImg.nHeight - 1);
		m_hBaseFun.KxCopyImage(m_GrayImg, m_CropImg, rc);
		IppiSize roiSize = { m_CropImg.nWidth, m_CropImg.nHeight };
		DstImg.Init(m_CropImg.nWidth, m_CropImg.nHeight);
		ippiCopy_8u_C1R(m_CropImg.buf, m_CropImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize);


		hCallInfo.nCallStatus = 10000;
		if (check_sts(hCallInfo, "Warning: Not find a edge area !", hCall))
		{
			return -1;
		}

	}

	rcEdge = m_hBlobAnalyse.GetSortSingleBlob(0).m_rc;
	return 1;
}

int CKxPlateEdgeCheck::FitlineAndRotateImg(const kxCImageBuf& SrcImg, kxCImageBuf& CropImg, kxRect<int> rcEdge, double &disT, double &disB, KxCallStatus& hCall)
{
	
	IppStatus status;

	const int nExtend = 50;
	int nLen = CropImg.nWidth;
	int nVerDirStart = (std::max)(rcEdge.top - nExtend, 0);
	int nVerDirEnd = (std::min)(nVerDirStart + _CROP_HEIGHT - 1, SrcImg.nHeight - 1);

	m_EdgeRegionImg.Init(nLen, _CROP_HEIGHT);
	ippsSet_8u(0, m_EdgeRegionImg.buf, m_EdgeRegionImg.nPitch * m_EdgeRegionImg.nHeight);
	kxRect<int> rcCopy;
	rcCopy.setup(0, nVerDirStart, nLen - 1, nVerDirEnd);
	m_hBaseFun.KxCopyImage(m_BinaryImg, m_EdgeRegionImg, rcCopy);


	Ipp32f* nTopEdge = ippsMalloc_32f(nLen);
	ippsSet_32f(0, nTopEdge, nLen);
	Ipp32f* nBottomEdge = ippsMalloc_32f(nLen);
	ippsSet_32f(0, nBottomEdge, nLen);

	m_MarkImg.Init(m_EdgeRegionImg.nWidth, m_EdgeRegionImg.nHeight);
	m_hBaseFun.KxCopyImage(m_GrayImg, m_MarkImg, rcCopy);


	m_EdgeAreaImg.Init(m_EdgeRegionImg.nWidth, m_EdgeRegionImg.nHeight);
	m_hBaseFun.KxCopyImage(m_GrayImg, m_EdgeAreaImg, rcCopy);

	for (int i = 1; i < nLen - 1; i++)
	{
		bool bNotFindTop = true;
		bool bNotFindBottom = true;
		for (int j = 1; j < m_EdgeRegionImg.nHeight - 1; j++)
		{
			if (m_EdgeRegionImg.buf[j*m_EdgeRegionImg.nPitch + i] > 0 && bNotFindTop)
			{
				nTopEdge[i] = (std::min)(float(j + 1), (float)m_EdgeRegionImg.nHeight);
				bNotFindTop = false;
				m_MarkImg.buf[(int)(nTopEdge[i])*m_MarkImg.nPitch + i] = 255;
			}

			if (m_EdgeRegionImg.buf[(m_EdgeRegionImg.nHeight - 1 - j)*m_EdgeRegionImg.nPitch + i] > 0 && bNotFindBottom)
			{
				nBottomEdge[i] = (std::max)(float((m_EdgeRegionImg.nHeight - 1 - j)), (float)0.0f);
				bNotFindBottom = false;
				m_MarkImg.buf[(int)(nBottomEdge[i])*m_MarkImg.nPitch + i] = 255;
			}

			if (!bNotFindTop && !bNotFindBottom)
			{
				break;
			}
		}
		if (bNotFindTop)
		{
			nTopEdge[i] = float(m_EdgeRegionImg.nHeight - 1);
		}
		if (bNotFindBottom)
		{
			nBottomEdge[i] = float(0.0f);
		}

	}
	nTopEdge[0] = nTopEdge[1];
	nTopEdge[nLen - 1] = nTopEdge[nLen - 2];
	nBottomEdge[0] = nBottomEdge[1];
	nBottomEdge[nLen - 1] = nBottomEdge[nLen - 2];


	int nBufferSize;
	ippsFilterMedianGetBufferSize(7, ipp32f, &nBufferSize);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	ippsFilterMedian_32f_I(nTopEdge, nLen, 7, NULL, NULL, pBuffer);
	ippsFilterMedian_32f_I(nBottomEdge, nLen, 7, NULL, NULL, pBuffer);
	ippsFree(pBuffer);

	//直线拟合
	float fAngleT, faT, fbT, fAngleB, faB, fbB;
	FitLineByRansac(nTopEdge, 0, nLen, 15, 3, fAngleT, faT, fbT);
	FitLineByRansac(nBottomEdge, 0, nLen, 15, 3, fAngleB, faB, fbB);

	//m_TmpImg2.Init(m_EdgeRegionImg.nWidth, m_EdgeRegionImg.nHeight);
	//m_hBaseFun.KxCopyImage(m_GrayImg, m_TmpImg2, rcCopy);


	//Ipp32f* pFitTopEdge = ippsMalloc_32f(nLen);
	//ippsSet_32f(0, pFitTopEdge, nLen);
	//Ipp32f* pFitBottomEdge = ippsMalloc_32f(nLen);
	//ippsSet_32f(0, pFitBottomEdge, nLen);

	//for (int i = 0; i < nLen; i++)
	//{
	//	pFitTopEdge[i] = faT*i + fbT + 0.5f;
	//	if (pFitTopEdge[i] > m_TmpImg2.nHeight || pFitBottomEdge[i] > m_TmpImg2.nHeight)
	//	{
	//		std::cout << "----------------------就是这里 -----------------------------------------------" << std::endl;
	//		std::cout << pFitTopEdge[i] << "   " << pFitBottomEdge[i] << std::endl;
	//	}
	//	m_TmpImg2.buf[(int)(pFitTopEdge[i])*m_TmpImg2.nPitch + i] = 255;
	//	pFitBottomEdge[i] = faB*i + fbB + 0.5f;
	//	m_TmpImg2.buf[(int)(pFitBottomEdge[i])*m_TmpImg2.nPitch + i] = 255;
	//}
	//std::cout << std::endl;

	float fDis = abs(fbB - fbT) / sqrt((faT + faB) / 2 * (faT + faB) / 2 + 1);
	//厚度
	{
		int nIndx = (std::min)(m_hResult.nDefectCount++, (int)_MAX_DEFECT_COUNT - 1);
		m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::THICKNESS);
		m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", 0.0f);
		m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", 0.0f);
		m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (m_CropImg.nWidth - 1)*1.0f);
		m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (m_CropImg.nHeight - 1)*1.0f);
		m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", fDis*1.95);
		m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", 1);

		std::cout << fDis * 1.95 << std::endl;
	}


	//整体旋转 确定旋转中心
	double nCenterX = (m_EdgeRegionImg.nWidth / 2);
	double nCenterY = (faT + faB) / 2 * nCenterX + (fbB + fbT) / 2;
	//点到两直线的距离
	disT = abs(faT * nCenterX - nCenterY + fbT) / sqrt(faT*faT + 1);
	disB = abs(faB * nCenterX - nCenterY + fbB) / sqrt(faB*faB + 1);


	double angle = atan((faT + faB) / 2) * 180 / PI;


	//-- TODO: 下面这一部分要改成非投影变换，因为投影变换会造成测量数据失真

	double xShift, yShift, coeffs[2][3];

	status = ippiGetRotateShift(nCenterX, nCenterY, angle, &xShift, &yShift);
	if (check_sts(status, "ippiGetRotateShift_", hCall))
	{
		return 0;
	}
	status = ippiGetRotateTransform(angle, xShift, yShift, coeffs);
	if (check_sts(status, "ippiGetRotateTransform_", hCall))
	{
		return 0;
	}
	IppiSize srcSize = { m_EdgeRegionImg.nWidth, m_EdgeRegionImg.nHeight };
	IppiSize dstSize = { m_EdgeRegionImg.nWidth, m_EdgeRegionImg.nHeight };
	int pSpecSize, pInitBufSize;
	status = ippiWarpAffineGetSize(srcSize, dstSize, ipp8u, coeffs, ippLinear, ippWarpForward, ippBorderConst, &pSpecSize, &pInitBufSize);
	if (check_sts(status, "ippiWarpAffineGetSize_", hCall))
	{
		return 0;
	}
	Ipp64f pBorderValue = 0;
	IppiWarpSpec* pSpec = (IppiWarpSpec*)ippsMalloc_8u(pSpecSize);
	Ipp8u* pBuf = ippsMalloc_8u(pInitBufSize);
	status = ippiWarpAffineLinearInit(srcSize, dstSize, ipp8u, coeffs, ippWarpForward, 1, ippBorderConst, &pBorderValue, 0, pSpec);
	if (check_sts(status, "ippiWarpAffineLinearInit_", hCall))
	{
		return 0;
	}
	m_RotateImg.Init(m_EdgeRegionImg.nWidth, m_EdgeRegionImg.nHeight);
	IppiPoint dstRoiOffset = { 0, 0 };
	status = ippiWarpAffineLinear_8u_C1R(m_EdgeAreaImg.buf, m_EdgeAreaImg.nPitch, m_RotateImg.buf, m_RotateImg.nPitch, dstRoiOffset, dstSize, pSpec, pBuf);
	if (check_sts(status, "ippiWarpAffineLinear_8u_C1R_", hCall))
	{
		return 0;
	}
	ippsFree(pSpec);
	ippsFree(pBuf);




	IppiSize roiSize = { m_EdgeRegionImg.nWidth, _EDGE_EXTEND * 2 + _EDGE_HEIGHT };
	int nOffsetY = (std::max)(0, (int)(nCenterY - (_EDGE_EXTEND + _EDGE_HEIGHT / 2)));
	ippiCopy_8u_C1R(m_RotateImg.buf + nOffsetY*m_RotateImg.nPitch, m_RotateImg.nPitch,
		CropImg.buf, CropImg.nPitch, roiSize);



	ippsFree(nTopEdge);
	ippsFree(nBottomEdge);
	return 1;
}

/*!
	date:			2018/12/26
	author:			HYH
	description:	以上部分在旋转之后的检测并不能满足分类的要求，而且检测的效果也并不稳定，新版需满足并分类一下几项功能
					（1）分切端毛刺  ---1-----						
					（2）分切端面粘料(集流体断续) —— - —— 
					（3）分切端面集流体弯曲	__-~-__
					（4）切端面粘有铝丝、铝粉   ----=====----
					（5）分切端面掉粉
*/
int CKxPlateEdgeCheck::ConfirmArch(const kxCImageBuf& SrcImg, KxCallStatus& hCall)
{//确定图形之中是否存在拱
	enum{
		MAXDIFF = 3,
	};
	//1， 必要的二值化
	m_ArchthreshImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	IppiSize roisize = {SrcImg.nWidth, SrcImg.nHeight};
	Ipp8u thresholdvalue;
	ippiComputeThreshold_Otsu_8u_C1R(SrcImg.buf, SrcImg.nPitch, roisize, &thresholdvalue);
	m_hBaseFun.KxThreshImage(SrcImg, m_ArchthreshImg, thresholdvalue, 255);

	m_ArcherodeImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	m_hBaseFun.KxOpenImage(m_ArchthreshImg, m_ArcherodeImg, 1, 3);
	int *pixelnum = new int[m_ArcherodeImg.nWidth];
	int *ysum = new int[m_ArcherodeImg.nWidth];
	int *result = new int[m_ArcherodeImg.nWidth];
	int *leftsum = new int[m_ArcherodeImg.nWidth];
	int *rightsum = new int[m_ArcherodeImg.nWidth];

	memset(pixelnum, 0, sizeof(int)*m_ArcherodeImg.nWidth);
	memset(ysum, 0, sizeof(int)*m_ArcherodeImg.nWidth);
	memset(result, 0, sizeof(int)*m_ArcherodeImg.nWidth);
	memset(leftsum, 0, sizeof(int)*m_ArcherodeImg.nWidth);
	memset(rightsum, 0, sizeof(int)*m_ArcherodeImg.nWidth);

	//2.统计竖直方向不为零的点数及其y左标，记录保存
	for (int i = 0; i < m_ArcherodeImg.nWidth; i++)
	{
		for (int j = 0; j < m_ArcherodeImg.nHeight; j++)
		{
			if (m_ArcherodeImg.buf[j*m_ArcherodeImg.nPitch + i] > 0)
			{
				pixelnum[i] += 1;
				ysum[i] += j; 
			}
		}
	}

	//3.对每个点的y坐标取均值，然后算梯度
	for (int i = 0; i < m_ArcherodeImg.nWidth; i++)
	{
		ysum[i] = ysum[i] / pixelnum[i];
	}
	for (int i = 0; i < m_ArcherodeImg.nWidth - 1; i++)
	{
		if (ysum[i] != 0 && ysum[i + 1] != 0)
		{
			result[i] = ysum[i + 1] - ysum[i];
		}
		else{
			// 考虑i+1那位是零，则认为是断了,让之继承未断前的值
			if (ysum[i + 1] == 0)
			{
				ysum[i + 1] = ysum[i];
			}
		}
	}

	//4.将正、负值统计，两个数组表示左右两侧梯度之和，如果是拱形，则这两个数组中的最大值（绝对值）
	//对应的那个数相加应该是接近0，并且位置接近（都是峰顶）
	int nsuml = 0, nsumr = 0;
	for (int i = 0; i < m_ArcherodeImg.nWidth - 1; i++)
	{
		leftsum[i] = result[i] + nsuml;
		nsuml += result[i];
	}

	for (int j = m_ArcherodeImg.nWidth - 2; j >= 0; j--)
	{
		rightsum[j] = result[j] + nsumr;
		nsumr += result[j];
	}
	int nleftmaxidx = 0; 
	int nrightmaxidx = 0;
	for (int i = 0; i < m_ArcherodeImg.nWidth - 2; i++)
	{
		if (abs(leftsum[i + 1]) > abs(leftsum[i]))
			nleftmaxidx = i + 1;

		if (abs(rightsum[i + 1]) > abs(rightsum[i]))
			nrightmaxidx = i + 1;
	}

	delete[]pixelnum ;
	delete[]ysum;
	delete[]result;
	delete[]leftsum;
	delete[]rightsum;

	std::cout << "统计的均衡值：" << abs(leftsum[nleftmaxidx] - rightsum[nrightmaxidx]) << std::endl;
	if (abs(leftsum[nleftmaxidx] + rightsum[nrightmaxidx]) <= MAXDIFF && abs(nleftmaxidx - nrightmaxidx) <= MAXDIFF)
	{
		return 1;
	}
	else{
		return 0;
	}
	
}

int CKxPlateEdgeCheck::MoveFrontFix(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall)
{//第一版


	//1,预处理+断
	Ipp32f* midrecords = ippsMalloc_32f(SrcImg.nWidth);
	int middleindex = 0;//极片细线中间
	BrokenSolve(SrcImg, m_Whitelinearea, m_PlateThreshImgClose, middleindex, midrecords);
	//if (1)
	//	return 1;//只检测断
	kxRect<int> imgrect(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
	//m_WhitelineareaClone.Init(SrcImg.nWidth, SrcImg.nHeight);
	//kxCImageBuf img;
	//MatToKxImageBuf(m_MatPlateWhiteline, img);
	//m_hBaseFun.KxCopyImage(m_Whitelinearea, m_WhitelineareaClone, imgrect);


	//2.将毛刺以及拱给挑出来，记录两个特征值
	int averageup = 0;
	int averagedown = 0;
	int result = WhiteLineSolve(m_Whitelinearea, middleindex, midrecords, averageup, averagedown);
	ippsFree(midrecords);
	if (result == 0)
	{
		return 0;
	}


	//3, 极片上的其它区域：先膨胀将白线造成的拖影包含进来，再对白线所在水平位置全部掩模，最后用原图减掉这部分，剩下的便是要检测的区域
	m_PlateClone.Init(SrcImg.nWidth, SrcImg.nHeight);
	m_hBaseFun.KxCopyImage(SrcImg, m_PlateClone, imgrect);	
	AnotherSolve(m_PlateClone, m_Whitelinearea, averageup, averagedown);

	middle = (averageup + averagedown) / 2;

	float fDis = PlateHeightSolve(SrcImg, middle);


	return 1;
}

int CKxPlateEdgeCheck::BrokenSolve(const kxCImageBuf& SrcImg, kxCImageBuf& WhiteLineArea, kxCImageBuf& DstImgClose, int &middleindex, Ipp32f* midrecords)
{

	m_PlateThreshImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	//1,必要的二值化（输入图严格保证只有极片区域且已经旋转校正过）
	kxRect<int> imgrect(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
	IppiSize roisize = { SrcImg.nWidth, SrcImg.nHeight };
	m_hBaseFun.KxThreshImage(SrcImg, m_PlateThreshImg, m_hPara.m_nWhiteLineThresh, 255);
	m_maskImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	ippiSet_8u_C1R(0, m_maskImg.buf, m_maskImg.nPitch, roisize);


	Ipp32f * pProject = ippsMalloc_32f(m_PlateThreshImg.nHeight);
	m_hBaseFun.KxProjectImage(m_PlateThreshImg, 1, pProject, 25);
	Ipp32f maxvalue = 0;
	ippsMaxIndx_32f(pProject, m_PlateThreshImg.nHeight, &maxvalue, &middleindex);//这里应该改成取五个极大值的中间值索引

	//这个闭是为了拱那部分区域如果断了能连起来，避免造成误认为多拱；如果拱太过平整，则有可能被充实，成为毛刺，但那种拱是极其尖锐的，概率低
	DstImgClose.Init(m_PlateThreshImg.nWidth, m_PlateThreshImg.nHeight);
	m_hBaseFun.KxCloseImage(m_PlateThreshImg, DstImgClose, 5, 1);


	//2.以一个N*1的卷积核沿细线中心往前推，遇到断了的地方便记录（该记录是为了后面能减掉在这里加上的部分）
	memset(midrecords, 0, m_PlateThreshImg.nWidth*sizeof(Ipp32f));
	Ipp32f *records = ippsMalloc_32f(m_PlateThreshImg.nWidth);
	memset(records, 0, m_PlateThreshImg.nWidth*sizeof(Ipp32f));
	int whitetop = std::max(0, middleindex - 5);
	int whitebot = std::min(m_PlateThreshImg.nHeight - 1, middleindex + 5);
	for (int i = 0; i < m_PlateThreshImg.nWidth; i++)
	{
		// 方案一
		if (DstImgClose.buf[middleindex * DstImgClose.nPitch + i] != 0)
		{
			midrecords[i] = 1;//这个是为了后面用
		}
		//这一步是为了尽量让在中心区域的那部分细线，包括断掉的部分尽可能的被包含进来
		for (int j = whitetop; j <= whitebot; j++)
		{
			if (m_PlateThreshImg.buf[j * m_PlateThreshImg.nPitch + i] == 0)
			{
				m_PlateThreshImg.buf[j * m_PlateThreshImg.nPitch + i] = 255;
				m_maskImg.buf[j* m_PlateThreshImg.nPitch + i] = 255;
				records[i] += 1;//这个是为了后面计算特征用
			}
		}
	}
	cv::Mat mat_PlateThreshImg, stats, centroids, targetimg, labelimg;
	KxImageBufToMat(m_PlateThreshImg, m_MatPlateThreshImg);
	cv::connectedComponentsWithStats(m_MatPlateThreshImg, labelimg, stats, centroids);
	cv::Mat listwidth = stats.colRange(cv::CC_STAT_WIDTH, cv::CC_STAT_WIDTH + 1).clone();
	listwidth.at<int>(0, 0) = 0;//第一位是整幅图的宽
	double *minval = 0, *maxval = 0;
	int minidx[2];
	int maxidx[2];
	cv::minMaxIdx(listwidth, minval, maxval, minidx, maxidx);
	cv::convertScaleAbs(labelimg, m_MatWhitelineArea);
	cv::threshold(m_MatWhitelineArea, m_MatWhitelineArea, maxidx[0], 255, cv::THRESH_TOZERO_INV);
	cv::threshold(m_MatWhitelineArea, m_MatWhitelineArea, maxidx[0] - 1, 255, cv::THRESH_BINARY);
	MatToKxImageBuf(m_MatWhitelineArea, WhiteLineArea);
	// 1，取断
	Ipp32f * vProject = ippsMalloc_32f(SrcImg.nWidth);
	m_hBaseFun.KxProjectImage(WhiteLineArea, 0, vProject, 255);
	ippiSub_8u_C1IRSfs(m_maskImg.buf, m_maskImg.nPitch, WhiteLineArea.buf, WhiteLineArea.nPitch, roisize, 0);

	//把原先是强制连接上的部分复原
	ippsSub_32f_I(records, vProject, SrcImg.nWidth);

	bool broken = false;
	int brokenidx = 0;
	std::vector<Broken> brokenrecord;
	int nsum;
	for (int i = 0; i < SrcImg.nWidth; i++)
	{
		nsum = vProject[i];
		if (nsum < 1e-8 && !broken)//首断
		{
			broken = true;
			brokenidx = i;
		}
		else if (nsum > 0 && broken)//断后初次连上
		{
			broken = false;
			Broken partone(brokenidx, i - 1);
			brokenrecord.push_back(partone);
		}
	}
	if (broken && nsum == 0)//这是补齐最后一小段残缺的问题
	{
		Broken partone(brokenidx, SrcImg.nWidth - 1);
		brokenrecord.push_back(partone);
	}
	int nbrokennum = std::min(int(_BROKEN_MAX), int(brokenrecord.size()));
	if (nbrokennum != 0)
	{
		int *idx = new int[nbrokennum];
		sortbroken(brokenrecord, idx, nbrokennum);
		for (int i = 0; i < nbrokennum; i++)
		{
			int nIndx = m_hResult.nDefectCount++;
			m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::BROKE);
			m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", brokenrecord[idx[i]].nstart* 1.0f);
			m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", 0);
			m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", brokenrecord[idx[i]].nlen* 1.0f);
			m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (SrcImg.nHeight - 1)* 1.0f);
			m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", brokenrecord[idx[i]].nlen* 1.95);
			m_hResult.nCheckStatus = _BLOBERR;
		}
		delete[]idx;
	}
	ippsFree(pProject);
	ippsFree(vProject);
	std::vector<Broken>().swap(brokenrecord);
	ippsFree(records);

	return 1;

}

int CKxPlateEdgeCheck::WhiteLineSolve(kxCImageBuf& WhiteLineArea, int middleindex, Ipp32f* midrecords, int& averageup, int& averagedown)
{
	int nstart = std::max(0, middleindex - _WHITELINE_HEIGHT);
	int nend = std::max(WhiteLineArea.nHeight - 1, middleindex + _WHITELINE_HEIGHT);
	double sumup = 0;
	double sumdown = 0;
	int nuptimes = 0;
	int ndowtimes = 0;
	int scale = 3;//缩进，节省时间
	for (int i = 0; i < WhiteLineArea.nWidth; i += scale)
	{
		for (int j = middleindex; j >= 0; j--)
		{
			if (WhiteLineArea.buf[j*WhiteLineArea.nPitch + i] == 0)
			{
				//edgeupdiff[i] = abs(j - middleindex - _WHITELINE_HEIGHT / 2);
				sumup += std::min(j + 1, WhiteLineArea.nHeight - 1);
				nuptimes++;
				break;
			}
		}
		for (int k = middleindex; k <= WhiteLineArea.nHeight - 1; k++)
		{
			if (WhiteLineArea.buf[k*WhiteLineArea.nPitch + i] == 0)
			{
				//edgedowndiff[i] = abs(k - middleindex);
				sumdown += std::max(k - 1, 0);
				ndowtimes++;
				break;
			}
		}
	}

	if (nuptimes == 0 || ndowtimes == 0)
	{
		int nIndx = (std::min)(m_hResult.nDefectCount++, (int)_MAX_DEFECT_COUNT - 1);
		m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::THICKNESS);
		m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", 0.0f);
		m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", 0.0);
		m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (WhiteLineArea.nWidth - 1)*1.0f);
		m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (WhiteLineArea.nHeight - 1)*1.0f);
		m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", (-1)*1.95);
		return 0;
	}

	averageup = std::max(int(sumup / nuptimes - _MARK_DILATE), 0);
	averagedown = std::min(int(sumdown / ndowtimes + _MARK_DILATE), WhiteLineArea.nHeight - 1);
	int averagemid = (averagedown + averageup) / 2;
	{
		float f = std::max(averagemid - m_hResult.nEdgeStart, m_hResult.nEdgeEnd - averagemid) * 1.0f /
			(std::min(averagemid - m_hResult.nEdgeStart, m_hResult.nEdgeEnd - averagemid)*1.0f);
		int nIndx = (std::min)(m_hResult.nDefectCount++, (int)_MAX_DEFECT_COUNT - 1);
		m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::SCALE);
		m_hResult.mapFeaturelists[nIndx][6] = std::make_pair("中心占比率", f);
		m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", 0.0f);
		m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", 0.0f);
		m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (WhiteLineArea.nWidth - 1)*1.0f);
		m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (WhiteLineArea.nHeight - 1)*1.0f);
	}
	
	//方案二：按点数排序一部分，侧重拱；按高度排序一部分，侧重毛刺 ;将这两者拼在一起，重复的去掉
	cv::Mat closeImg, labelsbad, statsbad, centroidsbad;

	m_WhitelineareaClose.Init(WhiteLineArea.nWidth, WhiteLineArea.nHeight);
	m_hBaseFun.KxCloseImage(WhiteLineArea, m_WhitelineareaClose, 5, 1);
	IppiSize masksize = { WhiteLineArea.nWidth, averagedown - averageup + 1 };

	//m_WhitelineareaClone.Init(labels8u.cols, labels8u.rows);
	ippiSet_8u_C1R(0, m_WhitelineareaClose.buf + averageup * m_WhitelineareaClose.nPitch, m_WhitelineareaClose.nPitch, masksize);
	KxImageBufToMat(m_WhitelineareaClose, closeImg);
	cv::connectedComponentsWithStats(closeImg, labelsbad, statsbad, centroidsbad);
	if (statsbad.rows > 1)
	{
		//(1)按高排序，侧重毛刺
		cv::Mat listheight = statsbad.colRange(cv::CC_STAT_HEIGHT, cv::CC_STAT_HEIGHT + 1).clone();
		listheight.at<int>(0, 0) = 0;
		cv::Mat listheightsortidx;
		cv::sortIdx(listheight, listheightsortidx, cv::SORT_EVERY_COLUMN + cv::SORT_DESCENDING);

		//(2)按点排序，侧重拱
		cv::Mat listarea = statsbad.colRange(cv::CC_STAT_AREA, cv::CC_STAT_AREA + 1).clone();
		listarea.at<int>(0, 0) = 0;
		cv::Mat listareasortidx;
		cv::sortIdx(listarea, listareasortidx, cv::SORT_EVERY_COLUMN + cv::SORT_DESCENDING);
		int ninterest_glitch = std::min(statsbad.rows - 1, int(_MAOCI_MAX));
		int ninterest_arch = std::min(statsbad.rows - 1, int(_ARCH));
		int *allidx = new int[ninterest_glitch + ninterest_arch];
		int finalnum = 0;

		mergeidx(listheightsortidx, ninterest_glitch, listareasortidx, ninterest_arch, allidx, finalnum);
		for (int i = 0; i < finalnum; i++)
		{
			int nIndx = m_hResult.nDefectCount++;
			if (nIndx < _MAX_DEFECT_COUNT - 1)
			{
				int x = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_LEFT);
				int y = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_TOP);
				int width = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_WIDTH);
				int height = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_HEIGHT);
				int area = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_AREA);
				//方案一
				//float rate = getrate(vProject, labels8u.cols, x, x + width - 1, area);
				//方案二
				float rate = getrate(midrecords, WhiteLineArea.nWidth, x, x + width - 1, width);
				//局部离心度，作为参考值，可用可不用
				//float localdis = calculatedis(m_PlateThreshImgClose, x, y, width, height, averagemid, averagedown - averageup - _MARK_DILATE * 2 + 1);
				float averagedis = height + (averagedown - averageup + 1) / 2;//偏离中心
				m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::LINE);
				m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", area*1.0f);
				m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量",averagemid);
				//m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, (x - 30)* 1.0f));
				//m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, (y - 30)* 1.0f));
				//m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (std::min)(WhiteLineArea.nWidth - x + 30, width + 60)* 1.0f);
				//m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (std::min)(WhiteLineArea.nHeight - y + 30, height + 60)* 1.0f);//这里的扩充是为了显示好看，所以这些数据不能用于表达式的计算
				m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, (x)* 1.0f));
				m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, (y )* 1.0f));
				m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", width* 1.0f);
				m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", height* 1.0f);//这里的扩充是为了显示好看，所以这些数据不能用于表达式的计算
				m_hResult.mapFeaturelists[nIndx][6] = std::make_pair("中心占比率", rate);
				m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", averagedis * 1.95);//1.95是像素物理尺寸
				m_hResult.nCheckStatus = _BLOBERR;
			}
		}
		delete[]allidx;
	}
	return 1;

}

int CKxPlateEdgeCheck::AnotherSolve(kxCImageBuf&SrcImg, kxCImageBuf& WhitelineImg, int averageup, int averagedown)
{
	IppiSize roisize = { WhitelineImg.nWidth, WhitelineImg.nHeight};
	m_hBaseFun.KxDilateImage(WhitelineImg, m_WhitelineareaCloneDilate, 1, 7);
	int up = averageup;
	int maskh = averagedown - averageup + 1;
	IppiSize whitemasksize = { m_Whitelinearea.nWidth, maskh };

	ippiSet_8u_C1R(255, m_WhitelineareaCloneDilate.buf + up * m_WhitelineareaCloneDilate.nPitch, m_WhitelineareaCloneDilate.nPitch, whitemasksize);
	ippiSub_8u_C1IRSfs(m_WhitelineareaCloneDilate.buf, m_WhitelineareaCloneDilate.nPitch,
		SrcImg.buf, SrcImg.nPitch, roisize, 0);
	m_hBaseFun.KxThreshImage(SrcImg, m_PlateOtherArea, m_hPara.m_nOtherAreaThresh, 255);
	m_hBlobAnalyseResult.ToBlobParallel(m_PlateOtherArea, CKxBlobAnalyse::_SORT_BYDOTS, _MAX_DEFECT_COUNT - m_hResult.nDefectCount, 1, 0);

	for (int i = 0; i < std::min(m_hBlobAnalyseResult.GetBlobCount(), int(_DIRTY)); i++)
	{
		int nIndx = m_hResult.nDefectCount++;
		if (nIndx <= _MAX_DEFECT_COUNT - 1)
		{
			m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::OTHERAREA);
			m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nDots*1.0f);
			m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nEnergy*1.0f);
			m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.left* 1.0f));
			m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.top* 1.0f));
			m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (std::min)(m_PlateClone.nWidth - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Width())* 1.0f);
			m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (std::min)(m_PlateClone.nHeight - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Height())* 1.0f);
			m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Width() * 1.95);

			m_hResult.nCheckStatus = _BLOBERR;
			//m_hResult.szErrInfo = "have some blobs";
		}

	}
	return 1;
}

int CKxPlateEdgeCheck::PlateHeightSolve(const kxCImageBuf& CropImg, int middle)
{//求极片厚度, 厚度的变化太大，不适合一直显示出来，所以这里这是为了测量极片是否有大的异常

	// 方案一，按照旧版的方法求的边缘
	//IppStatus status = ippsSub_32f_I(TopEdge, BotEdge, len);
	//Ipp32f result = 0;
	//status = ippsSum_32f(BotEdge, len, &result, ippAlgHintAccurate);
	//Ipp32f averagedis = result / len;
	//float fdis = abs(cos(angle)) * (averagedis);

	// 方案二，把图旋转完成之后再进行搜边测厚度
	m_CropThreshImg.Init(CropImg.nWidth, CropImg.nHeight);
	m_hBaseFun.KxThreshImage(CropImg, m_CropThreshImg, m_hPara.m_nSearchEdgeThresh, 255);
	
	Ipp32f topsum = 0, bottomsum = 0;
	int ntopnum = 0, nbotnum = 0;
	//for (int i = 0; i < m_CropThreshImg.nWidth; i += 3)
	//{
	//	//搜顶部
	//	for (int top = 0; top < middle; top += 5)
	//	{
	//		if (m_CropThreshImg.buf[top * m_CropThreshImg.nPitch + i] > 0)
	//		{
	//			ntopnum += 1;
	//			if (top != 0)
	//			{
	//				for (int k = top - 1; k > top - 5; k--)
	//				{
	//					if (m_CropThreshImg.buf[k * m_CropThreshImg.nPitch + i] == 0)
	//					{
	//						topsum += k;
	//						break;
	//					}
	//				}
	//			}
	//			else
	//			{
	//				topsum += top;
	//			}
	//			break;
	//		}
	//	}
	//	//搜底部
	//	for (int bot = CropImg.nHeight - 1; bot > middle; bot -= 5)
	//	{
	//		if (m_CropThreshImg.buf[bot * m_CropThreshImg.nPitch + i] > 0)
	//		{
	//			nbotnum += 1;
	//			if (bot != CropImg.nHeight - 1)
	//			{
	//				for (int k = bot + 1; k < bot + 5; k++)
	//				{
	//					if (m_CropThreshImg.buf[k * m_CropThreshImg.nPitch + i] == 0)
	//					{
	//						bottomsum += k;
	//						break;
	//					}
	//				}
	//			}
	//			else
	//			{
	//				bottomsum += bot;
	//			}
	//			break;
	//		}
	//	}
	//}

	int scale = 3;
	for (int i = 0; i < m_CropThreshImg.nWidth; i += scale)
	{
		for (int j = middle; j >= 0; j--)
		{
			if (m_CropThreshImg.buf[j*m_CropThreshImg.nPitch + i] == 0)
			{
				//edgeupdiff[i] = abs(j - middleindex - _WHITELINE_HEIGHT / 2);
				topsum += std::min(j + 1, m_CropThreshImg.nHeight - 1);
				ntopnum++;
				break;
			}
		}
		for (int k = middle; k <= m_CropThreshImg.nHeight - 1; k++)
		{
			if (m_CropThreshImg.buf[k*m_CropThreshImg.nPitch + i] == 0)
			{
				//edgedowndiff[i] = abs(k - middleindex);
				bottomsum+= std::max(k - 1, 0);
				nbotnum++;
				break;
			}
		}
	}




	float fDis, plateup, platedown;
	if (ntopnum != 0 || nbotnum != 0)
	{
		plateup = topsum / ntopnum;
		platedown = bottomsum / nbotnum;
		fDis = platedown - plateup;
	}
	else
	{
		plateup = 0;
		platedown = CropImg.nHeight - 1;
		fDis = -1;//搜边错误，设置的时候可以直接设置该值 > 0
	} 

	//厚度
	{
		int nIndx = (std::min)(m_hResult.nDefectCount++, (int)_MAX_DEFECT_COUNT - 1);
		m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::SCALE);
		m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", 0.0f);
		m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", plateup);
		m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (CropImg.nWidth - 1)*1.0f);
		m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (platedown - plateup + 1)*1.0f);
		m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", fDis*1.95);
	}
	std::cout << fDis*1.95 << std::endl;
	return 1;
}

void CKxPlateEdgeCheck::sortbroken(std::vector<Broken> brokenrecord, int* resultidx, int neednum)
{//排序最大的neednum个
	for (int i = 0; i < neednum; i++)
	{
		int maxlen = 0, idx = 0;
		for (int j = 0; j < brokenrecord.size(); j++)
		{
			if (maxlen < brokenrecord[j].nlen)
			{
				maxlen = brokenrecord[j].nlen;
				idx = j;
			}
		}
		resultidx[i] = idx;
		brokenrecord[idx].nlen = 0;
	}

}

float CKxPlateEdgeCheck::getrate(Ipp32f* whitelineproject, int len, int nstart, int nend, int blobarea)
{//凸出部分垂直投影占整条白线垂直投影的比例，这可以分辨是毛刺还是拱
	if (nend > len)
	{
		return -1;
	}
	Ipp32f result = 0;
	//for (int i = nstart; i < nend + 1; i++)
	//{
	//	std::cout << whitelineproject[i] << "   ";
	//}
	ippsSum_32f(whitelineproject + nstart, nend - nstart + 1, &result, ippAlgHintAccurate);

	//Ipp32f resultss[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	//ippsSum_32f(resultss +  2, 3, &result, ippAlgHintAccurate);

	if (result < 1e-8)
	{
		return 100;
	}
	else
	{
		return float(blobarea) / result;
	}
}

void CKxPlateEdgeCheck::mergeidx(cv::Mat listidx1, int glitchnum, cv::Mat listidx2, int archnum, int* result, int &finallen)
{//将两个矩阵合并到一个数组，重复出现的部分删掉
	for (int i = 0; i < glitchnum; i++)
	{
		result[i] = listidx1.at<int>(i, 0);
		finallen++;
	}
	for (int i = 0; i < archnum; i++)
	{
		bool bnotin = true;
		int idx = listidx2.at<int>(i, 0);
		for (int j = 0; j < glitchnum; j++)
		{
			if (idx == listidx1.at<int>(j, 0))
			{
				bnotin = false;
				break;
			}
		}
		if (bnotin)
		{
			result[finallen++] = idx;
		}
	}

}

float CKxPlateEdgeCheck::calculatedis(kxCImageBuf singleblobimg, int x, int y, int width, int height, int blobcentery, int blobheight)
{//为了能算出毛刺的局部偏差

	int scale = 3;//缩进，节省时间
	float localdis;
	int ntimes = 0;
	if (y < blobcentery)//凸出物在中心上侧
	{
		int sumdown = 0;
		for (int i = x; i < x+width; i += scale)
		{
			for (int k = blobcentery; k <= singleblobimg.nHeight - 1; k++)
			{
				if (singleblobimg.buf[k*singleblobimg.nPitch + i] == 0)
				{
					//edgedowndiff[i] = abs(k - middleindex);
					sumdown += k;
					ntimes += 1;
					break;
				}
			}
		}
		//localdis = ((sumdown / ntimes) - (y + height)) / 2 + height;
		localdis = (sumdown / ntimes) - blobheight / 2 - y;
	}
	else
	{
		int sumup = 0;
		for (int i = x; i < x+width; i += scale)
		{
			for (int j = blobcentery; j >= 0; j--)
			{
				if (singleblobimg.buf[j*singleblobimg.nPitch + i] == 0)
				{
					//edgeupdiff[i] = abs(j - middleindex - _WHITELINE_HEIGHT / 2);
					sumup += j;
					ntimes += 1;
					break;
				}
			}

		}
		//localdis = (y - sumup / ntimes )/ 2 + height;
		localdis = (y + height) - ((sumup / ntimes) + blobheight / 2);
	}
	return localdis;
}

void CKxPlateEdgeCheck::KxImageBufToMat(const kxCImageBuf& SrcImg, cv::Mat& DstImg)
{
	DstImg = cv::Mat(cv::Size(SrcImg.nWidth, SrcImg.nHeight), CV_8UC(SrcImg.nChannel), SrcImg.buf);
}

void CKxPlateEdgeCheck::MatToKxImageBuf(cv::Mat& SrcImg, kxCImageBuf&DstImg)
{
	DstImg.SetImageBuf(SrcImg.data, SrcImg.cols, SrcImg.rows, SrcImg.step, SrcImg.channels(), false);
}

/*
	date:			2019/03/18
	author:			HYH
	description:	在进厂很长一段时间之后发现，极片的断、毛刺、拱的概念是不一样的，也就是说不能用同一个阈值，
					当前版本对这个进行改动，不考虑算法时间问题
*/
int CKxPlateEdgeCheck::MoveFrontFix2(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall)
{//第二版
	tick_count  total_s, total_e;
	total_s = tick_count::now();


	KxCallStatus hCallstatus;
	int status = 0;
	//------- 1,处理断 ---------//
	middle = 0;
	status = BrokenSolve2(SrcImg, m_BrokenAreaImg, middle, hCallstatus);
	if (status <= 0)
	{
		check_sts(hCallstatus, "solve broken error_", hCall);
		return status;
	}


	//std::cout << "first: " << (tick_count::now() - total_s).seconds() * 1000 << std::endl;
	total_s = tick_count::now();


	//------ 2,处理白线区域，也就是拱或者毛刺 ----------//
	std::vector <RectBlob> vec_blob;
	m_WhiteLineMask.Init(SrcImg.nWidth, SrcImg.nHeight);
	status = WhiteLineSolve2(SrcImg, m_WhiteLineMask, vec_blob, middle, hCallstatus);
	if (status <= 0)
	{
		check_sts(hCallstatus, "solve whiteline error_", hCall);
		return status;
	}

	//std::cout << "second: " << (tick_count::now() - total_s).seconds() * 1000 << std::endl;
	total_s = tick_count::now();

	//----- 3，最后处理极片除了集流体外的区域，mask掉断的白线 ---------------//
	status = AnotherSolve2(SrcImg, m_WhiteLineMask, vec_blob, middle, hCallstatus);
	if (status <= 0)
	{
		check_sts(hCallstatus, "solve whiteline error_", hCall);
		return status;
	}
	//std::cout << "end: " << (tick_count::now() - total_s).seconds() * 1000 << std::endl;

	//middle = (averageup + averagedown) / 2;

	/*
	//1,预处理+断
	Ipp32f* midrecords = ippsMalloc_32f(SrcImg.nWidth);
	int middleindex = 0;//极片细线中间
	BrokenSolve(SrcImg, m_Whitelinearea, m_PlateThreshImgClose, middleindex, midrecords);
	kxRect<int> imgrect(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);


	//2.将毛刺以及拱给挑出来，记录两个特征值
	int averageup = 0;
	int averagedown = 0;
	int result = WhiteLineSolve(m_Whitelinearea, middleindex, midrecords, averageup, averagedown);
	ippsFree(midrecords);
	if (result == 0)
	{
		return 0;
	}


	//3, 极片上的其它区域：先膨胀将白线造成的拖影包含进来，再对白线所在水平位置全部掩模，最后用原图减掉这部分，剩下的便是要检测的区域
	m_PlateClone.Init(SrcImg.nWidth, SrcImg.nHeight);
	m_hBaseFun.KxCopyImage(SrcImg, m_PlateClone, imgrect);
	AnotherSolve(m_PlateClone, m_Whitelinearea, averageup, averagedown);

	middle = (averageup + averagedown) / 2;

	float fDis = PlateHeightSolve(SrcImg, middle);

	*/
	return 1;
}

int CKxPlateEdgeCheck::GetMid(Ipp32f* pProject, int len, int selectnum)
{//在一个数组里挑出selectum个最大值，且在这些最大值形成的“索引队列”中挑选索引值为中间的值返回
	if (selectnum > len)
	{
		return -1;
	}
	int* indexarray = new int[selectnum];
	Ipp32f maxvalue = 0;
	int middleindex = 0;
	for (int i = 0; i < selectnum; ++i)
	{
		ippsMaxIndx_32f(pProject, len, &maxvalue, &middleindex);
		pProject[middleindex] = 0;
		indexarray[i] = middleindex;
	}

	int maxv = 0, nselectindex = 0;
	for (int i = 0; i < selectnum; i++)
	{
		ippsMaxIndx_32s(indexarray, selectnum, &maxv, &nselectindex);
		if ( i == selectnum / 2)
		{
			int result = indexarray[nselectindex];
			delete[] indexarray;
			return result;
		}
		else if (i > selectnum / 2)//这一段是多余的
		{
			delete[] indexarray;
			return -1;
		}
		indexarray[nselectindex] = 0;//归零
	}

	delete[] indexarray;
	return -1;

}

int CKxPlateEdgeCheck::GetCenterROI(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int threshvalue, int &middleindex, KxCallStatus& hCall, Ipp32f *midrecord, int offset)
{//找出中心相连最大的blob
	tick_count  total_s, total_e;
	total_s = tick_count::now();

	KxCallStatus hCallInfo;
	m_PlateThreshImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	//1,必要的二值化（输入图严格保证只有极片区域且已经旋转校正过）
	kxRect<int> imgrect(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
	IppiSize roisize = { SrcImg.nWidth, SrcImg.nHeight };
	m_hBaseFun.KxThreshImage(SrcImg, m_PlateThreshImg, threshvalue, 255);

	Ipp32f * pProject = ippsMalloc_32f(m_PlateThreshImg.nHeight);
	m_hBaseFun.KxProjectImage(m_PlateThreshImg, 1, pProject, 255);
	//Ipp32f maxvalue = 0;
	middleindex = GetMid(pProject, m_PlateThreshImg.nHeight, 5);
	if (middleindex < 0 || middleindex >= SrcImg.nHeight)
	{
		hCallInfo.nCallStatus = 10000;
		ippsFree(pProject);
		check_sts(hCallInfo, "Error: blob area GetMid error_", hCall);
		return 0; //上面的预处理之后找到的中心宽度肯定是一个图像宽，否则就是有问题
	}

	//ippsMaxIndx_32f(pProject, m_PlateThreshImg.nHeight, &maxvalue, &middleindex);//这里应该改成取五个极大值的中间值索引
	//std::cout << "（1）: " << (tick_count::now() - total_s).seconds() * 1000 << std::endl;
	total_s = tick_count::now();

	int whitetop = std::max(0, middleindex - offset);
	int whitebot = std::min(m_PlateThreshImg.nHeight - 1, middleindex + offset);

	if (midrecord != NULL)//拷贝中心点数据
	{
		memset(midrecord, 0, SrcImg.nWidth*sizeof(Ipp32f));
		m_SingleMid.Init(SrcImg.nWidth, 1);
		kxRect<int>rc(0, middleindex, SrcImg.nWidth - 1, middleindex);
		m_hBaseFun.KxCopyImage(m_PlateThreshImg, m_SingleMid, rc);
		m_hBaseFun.KxProjectImage(m_SingleMid, 0, midrecord, 255);
	}

	m_PlateCenterArea.Init(m_PlateThreshImg.nWidth, whitebot - whitetop + 1);
	kxRect<int> rc(0, whitetop, m_PlateThreshImg.nWidth - 1, whitebot);
	IppiSize whitemask = { m_PlateThreshImg.nWidth, rc.Height() };
	m_hBaseFun.KxCopyImage(m_PlateThreshImg, m_PlateCenterArea, rc);
	ippiSet_8u_C1R(255, m_PlateThreshImg.buf + whitetop * m_PlateThreshImg.nPitch, m_PlateThreshImg.nPitch, whitemask);

	total_s = tick_count::now();

	//TODO: 20190425 下面程序会出现一种bug的可能，铝粉离集流体就差了一两个像素，但在上面的掩模过程中不小心掩模到它了，之后就把这块铝粉算进来了

	IppiPoint seed = { 0, middleindex};
	FloodfilltoGetROI(m_PlateThreshImg, DstImg, seed);

	//ippiSub_8u_C1IRSfs(m_maskImg.buf, m_maskImg.nPitch, DstImg.buf, DstImg.nPitch, roisize, 0);
	//ippiSub_8u_C1IRSfs(m_PlateCenterArea.buf, m_PlateCenterArea.nPitch, DstImg.buf + whitetop * DstImg.nPitch, DstImg.nPitch, whitemask, 0);
	//IppiSize masksize = { m_PlateCenterArea.nWidth, m_PlateCenterArea.nHeight};
	ippiCopy_8u_C1R(m_PlateCenterArea.buf, m_PlateCenterArea.nPitch, DstImg.buf + whitetop * DstImg.nPitch, DstImg.nPitch, whitemask);


	ippsFree(pProject);
	return 1;
}

int CKxPlateEdgeCheck::BrokenSolve2(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg,  int&middleindex, KxCallStatus& hCall)
{
	tick_count  total_s, total_e;
	total_s = tick_count::now();
	KxCallStatus hCallInfo; 
	int status = 0;
	status = GetCenterROI(SrcImg, DstImg, m_hPara.m_nBrokenThresh, middleindex, hCallInfo);
	if (status <= 0)
	{
		check_sts(hCallInfo, "check broken error_", hCall);
		return status;
	}
	std::cout << "broken first: " << (tick_count::now() - total_s).seconds() * 1000 << std::endl;
	//total_s = tick_count::now();

	Ipp32f * vProject = ippsMalloc_32f(DstImg.nWidth);

	m_hBaseFun.KxProjectImage(DstImg, 0, vProject, 255);

	bool broken = false;
	int brokenidx = 0;
	std::vector<Broken> brokenrecord;
	int nsum;
	for (int i = 0; i < DstImg.nWidth; i++)
	{
		nsum = vProject[i];
		if (nsum < 1e-8 && !broken)//首断
		{
			broken = true;
			brokenidx = i;
		}
		else if (nsum > 0 && broken)//断后初次连上
		{
			broken = false;
			Broken partone(brokenidx, i - 1);
			brokenrecord.push_back(partone);
		}
	}
	if (broken && nsum == 0)//这是补齐最后一小段残缺的问题
	{
		Broken partone(brokenidx, DstImg.nWidth - 1);
		brokenrecord.push_back(partone);
	}
	//std::cout << "broken second: " << (tick_count::now() - total_s).seconds() * 1000 << std::endl;
	//total_s = tick_count::now();

	int nbrokennum = std::min(int(_BROKEN_MAX), int(brokenrecord.size()));
	if (nbrokennum != 0)
	{
		int *idx = new int[nbrokennum];
		sortbroken(brokenrecord, idx, nbrokennum);
		for (int i = 0; i < nbrokennum; i++)
		{
			int nIndx = m_hResult.nDefectCount++;
			m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::BROKE);
			m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", brokenrecord[idx[i]].nstart* 1.0f);
			m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", 0);
			m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", brokenrecord[idx[i]].nlen* 1.0f);
			m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (SrcImg.nHeight - 1)* 1.0f);
			m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", brokenrecord[idx[i]].nlen* 1.95);
			m_hResult.nCheckStatus = _BLOBERR;
		}
		delete[]idx;
	}

	ippsFree(vProject);
	//std::cout << "broken end: " << (tick_count::now() - total_s).seconds() * 1000 << std::endl;

	return 1;

}

int CKxPlateEdgeCheck::WhiteLineSolve2(const kxCImageBuf& SrcImg, kxCImageBuf& whitelinearea, std::vector <RectBlob>& vec_blob, int middleidx, KxCallStatus& hCall)
{
	//核心，根据拱跟毛刺的阈值分别找到区域
	KxCallStatus hCallInfo;
	int status = 0;

	//--- 1，根据毛刺的阈值得到最细的集流体，同时得到那个中心区域的record，毛刺的record是最靠谱的，因为它把中心瘦化 ----//
	int middle = 0;//注意，这里的middle没用，统一用断产生的middleidx
	Ipp32f *midrecord = ippsMalloc_32f(SrcImg.nWidth);
	status = GetCenterROI(SrcImg, m_GlitchAreaImg, m_hPara.m_nWhiteLineThresh, middle, hCallInfo, midrecord);
	whitelinearea.SetImageBuf(m_GlitchAreaImg.buf, m_GlitchAreaImg.nWidth, m_GlitchAreaImg.nHeight, m_GlitchAreaImg.nPitch, m_GlitchAreaImg.nChannel, true);
	if (status <= 0)
	{
		ippsFree(midrecord);
		check_sts(hCallInfo, "check glitch error_", hCall);
		return status;
	}
	status = GetCenterROI(SrcImg, m_ArchAreaImg, m_hPara.m_nArchThresh, middle, hCallInfo);
	if (status <= 0)
	{
		ippsFree(midrecord);
		check_sts(hCallInfo, "check Arch error_", hCall);
		return status;
	}

	//--- 2，对拱的二值化图进行处理，去掉中心集流体，多出来的部分做个闭运算尽量让断开的小拱闭合起来；在满足拱的必要条件时，
	//	  将毛刺对应图像的那部分区域掩模掉，目的是为了不在后期的判别中重复，避免既是拱又是毛刺的情况  ----//
	int offset = 3;//太小会让一个略有弧度的拱被识别为毛刺，因为太宽，导致误判 2019.04.15
	int  up = std::max(0, middleidx - _WHITELINE_HEIGHT / 2 - offset);
	int down = std::min(SrcImg.nHeight - 1, middleidx + _WHITELINE_HEIGHT / 2 + offset);
	IppiSize masksize = { SrcImg.nWidth, down - up + 1 };
	ippiSet_8u_C1R(0, m_ArchAreaImg.buf + up * m_ArchAreaImg.nPitch, m_ArchAreaImg.nPitch, masksize);
	m_ArchCloseImg.Init(m_ArchAreaImg.nWidth, m_ArchAreaImg.nHeight);
	m_hBaseFun.KxCloseImage(m_ArchAreaImg, m_ArchCloseImg, 11, 1);

	cv::Mat closeArchImg, labelsbad, statsbad, centroidsbad;
	KxImageBufToMat(m_ArchCloseImg, closeArchImg);
	cv::connectedComponentsWithStats(closeArchImg, labelsbad, statsbad, centroidsbad);
	if (statsbad.rows > 1)
	{
		cv::Mat listheight = statsbad.colRange(cv::CC_STAT_HEIGHT, cv::CC_STAT_HEIGHT + 1).clone();
		listheight.at<int>(0, 0) = 0;
		cv::Mat listheightsortidx;
		cv::sortIdx(listheight, listheightsortidx, cv::SORT_EVERY_COLUMN + cv::SORT_DESCENDING);
		int maxdefectnum = std::min(m_hResult.nDefectCount + _ARCH, int(_MAX_DEFECT_COUNT));
		for (int i = 0; i < listheightsortidx.rows - 1; i++)
		{
			int x = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_LEFT);
			int y = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_TOP);
			int width = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_WIDTH);
			int height = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_HEIGHT);
			int area = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_AREA);
			float rate = 0;
			if (height < _GLITCH_ARCH_SMALLEST || width  > _GLITCH_LARGETEST)//根据情况使用哪个，因为getrate2耗时
			{
				rate = getrate(midrecord, SrcImg.nWidth, x, x + width - 1, width);
			}
			else
			{
				rate = getrate2(whitelinearea, middleidx, x, x + width - 1);
				if (rate < 0)
				{
					hCallInfo.nCallStatus = 10000;
					check_sts(hCallInfo, "get arch rate2 error_", hCall);
					ippsFree(midrecord);
					return -1;
				}
			}			
			if (rate >= _RATE_GLITCH_OR_ARCH)
			{
				int nIndx = m_hResult.nDefectCount;
				RectBlob one(x, y, width, height);
				vec_blob.push_back(one);
				if (nIndx < maxdefectnum)
				{
					//将毛刺图像的那部分掩模掉
					IppiSize blacksize = { width, height };
					ippiSet_8u_C1R(0, m_GlitchAreaImg.buf + y * m_GlitchAreaImg.nPitch + x, m_GlitchAreaImg.nPitch, blacksize);
					//方案一
					//float rate = getrate(vProject, labels8u.cols, x, x + width - 1, area);
					//方案二
					//局部离心度，作为参考值，可用可不用
					//float localdis = calculatedis(m_PlateThreshImgClose, x, y, width, height, averagemid, averagedown - averageup - _MARK_DILATE * 2 + 1);
					//float averagedis = abs(y - middleidx);//偏离中心
					float averagedis = y > middleidx ? (y - middleidx + height) : (middleidx - y);//偏离中心
					m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::LINE);
					m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", area*1.0f);
					//m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", averagemid);
					//m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, (x - 30)* 1.0f));
					//m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, (y - 30)* 1.0f));
					//m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (std::min)(WhiteLineArea.nWidth - x + 30, width + 60)* 1.0f);
					//m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (std::min)(WhiteLineArea.nHeight - y + 30, height + 60)* 1.0f);//这里的扩充是为了显示好看，所以这些数据不能用于表达式的计算
					m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, (x)* 1.0f));
					m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, (y)* 1.0f));
					m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", width* 1.0f);
					m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", height* 1.0f);//这里的扩充是为了显示好看，所以这些数据不能用于表达式的计算
					m_hResult.mapFeaturelists[nIndx][6] = std::make_pair("中心占比率", rate);
					m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", averagedis * 1.95);//1.95是像素物理尺寸
					m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", width * 1.95);//1.95是像素物理尺寸
					m_hResult.nCheckStatus = _BLOBERR;
					m_hResult.nDefectCount++;
				}
			}
			else
			{
				break;
			}
		}
	}

	//------ 3,对毛刺的二值化图进行处理 ---------//
	ippiSet_8u_C1R(0, m_GlitchAreaImg.buf + up * m_GlitchAreaImg.nPitch, m_GlitchAreaImg.nPitch, masksize);
	cv::Mat mat_glitchimg;
	KxImageBufToMat(m_GlitchAreaImg, mat_glitchimg);
	cv::connectedComponentsWithStats(mat_glitchimg, labelsbad, statsbad, centroidsbad);
	if (statsbad.rows > 1)
	{
		cv::Mat listheight = statsbad.colRange(cv::CC_STAT_HEIGHT, cv::CC_STAT_HEIGHT + 1).clone();
		listheight.at<int>(0, 0) = 0;
		cv::Mat listheightsortidx;
		cv::sortIdx(listheight, listheightsortidx, cv::SORT_EVERY_COLUMN + cv::SORT_DESCENDING);
		int maxdefectnum = std::min(m_hResult.nDefectCount + _MAOCI_MAX, int(_MAX_DEFECT_COUNT));
		for (int i = 0; i < listheightsortidx.rows - 1; i++)
		{
			int x = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_LEFT);
			int y = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_TOP);
			int width = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_WIDTH);
			int height = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_HEIGHT);
			int area = statsbad.at<int>(listheightsortidx.at<int>(i, 0), cv::CC_STAT_AREA);
			float rate = 0;
			if (height < _GLITCH_ARCH_SMALLEST || width  > _GLITCH_LARGETEST)//根据情况使用哪个，因为getrate2耗时
			{
				rate = getrate(midrecord, SrcImg.nWidth, x, x + width - 1, width);
			}
			else
			{
				rate = getrate2(whitelinearea, middleidx, x, x + width - 1);
				if (rate < 0)
				{
					hCallInfo.nCallStatus = 10000;
					check_sts(hCallInfo, "get glitch rate2 error_", hCall);
					ippsFree(midrecord);
					return -1;
				}
			}
			if (rate < _RATE_GLITCH_OR_ARCH)
			{
				int nIndx = m_hResult.nDefectCount;
				RectBlob one(x, y, width, height);
				vec_blob.push_back(one);
				if (nIndx < maxdefectnum)
				{
					//方案一
					//float rate = getrate(vProject, labels8u.cols, x, x + width - 1, area);
					//方案二
					//局部离心度，作为参考值，可用可不用
					//float localdis = calculatedis(m_PlateThreshImgClose, x, y, width, height, averagemid, averagedown - averageup - _MARK_DILATE * 2 + 1);
					//float averagedis = abs(y - middleidx);//偏离中心
					//float averagedis = y > middleidx ? (y - middleidx + height) : (middleidx - y);//偏离中心
					float averagedis = 0;
					if (y > middleidx)
					{
						averagedis = y - middleidx + height;
						y = middleidx;
					}
					else
					{
						averagedis = middleidx - y;
					}
					m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::LINE);
					m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", area*1.0f);
					//m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", averagemid);
					//m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, (x - 30)* 1.0f));
					//m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, (y - 30)* 1.0f));
					//m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", (std::min)(WhiteLineArea.nWidth - x + 30, width + 60)* 1.0f);
					//m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", (std::min)(WhiteLineArea.nHeight - y + 30, height + 60)* 1.0f);//这里的扩充是为了显示好看，所以这些数据不能用于表达式的计算
					m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, (x)* 1.0f));
					m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, (y)* 1.0f));
					m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", width* 1.0f);
					m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", averagedis* 1.0f);//这里的扩充是为了显示好看，所以这些数据不能用于表达式的计算
					m_hResult.mapFeaturelists[nIndx][6] = std::make_pair("中心占比率", rate);
					m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", averagedis * 1.95);//1.95是像素物理尺寸
					m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", width * 1.95);//1.95是像素物理尺寸
					m_hResult.nCheckStatus = _BLOBERR;
					m_hResult.nDefectCount++;
				}
				else
				{
					break;
				}
			}
		}
	}
	ippsFree(midrecord);
	return 1;

}

int CKxPlateEdgeCheck::AnotherSolve2(const kxCImageBuf& SrcImg, const kxCImageBuf& MaskImg, std::vector <RectBlob>& vec_blob, int middleidx, KxCallStatus& hCall)
{
	// 其他区域检查较为复杂，要顾虑贴近集流体的铝丝铝粉的感受
	// 故，其他区域的检测虽然有自己的阈值，但其与检查白线也即毛刺的阈值直接挂钩
	KxCallStatus hCallInfo;
	IppiSize roisize = { SrcImg.nWidth, SrcImg.nHeight };

	
	//-----------方案一----------//
	//int middleindex = 0;
	//int status = GetCenterROI(SrcImg, m_AnotherAreaWhiteImg, m_hPara.m_nOtherAreaThresh, middleindex, hCallInfo);
	//if (status <= 0)
	//{
	//	check_sts(hCallInfo, "check another area error_", hCall);
	//	return status;
	//}

	//m_AnotherThreshImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	//m_hBaseFun.KxThreshImage(SrcImg, m_AnotherThreshImg, m_hPara.m_nOtherAreaThresh, 255);

	//m_AnotherAreaImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	//ippiSub_8u_C1IRSfs(m_AnotherAreaWhiteImg.buf, m_AnotherAreaWhiteImg.nPitch, m_AnotherThreshImg.buf, m_AnotherThreshImg.nPitch, roisize, 0);
	//
	//if (m_hPara.m_nOtherAreaThresh < m_hPara.m_nWhiteLineThresh)
	//{
	//	//这一步很重要，想象一下靠近集流体的铝丝铝粉，在m_nWhiteLineThresh二值化中与集流体分开，但是m_nOtherAreaThresh却
	//	//将铝丝铝粉跟集流体和到一起，那么在上一步相减的过程中这一块铝丝铝粉就被删掉了，而检毛刺那里也跳过去，就相当于屏蔽

	//	//-------------------TODO -----------------------、、
	//	//这部分逻辑还是有问题的，那块差点被屏蔽掉的铝丝就是用毛刺阈值二值化的了，那就不准了
	//	//考虑下粗暴点直接减掉MaskWhitelineImg（处理一下这张图，上下dilate一下， 再根据中心跟白线高mask一下）

	//	
	//	m_GlitchThreshImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	//	m_hBaseFun.KxThreshImage(SrcImg, m_GlitchThreshImg, m_hPara.m_nWhiteLineThresh, 255);
	//	ippiSub_8u_C1IRSfs(MaskWhitelineImg.buf, MaskWhitelineImg.nPitch, m_GlitchThreshImg.buf, m_GlitchThreshImg.nPitch, roisize, 0);
	//	ippiAdd_8u_C1IRSfs(m_GlitchThreshImg.buf, m_GlitchThreshImg.nPitch, m_AnotherThreshImg.buf, m_AnotherThreshImg.nPitch, roisize, 0);
	//}
	//else if (m_hPara.m_nOtherAreaThresh > m_hPara.m_nWhiteLineThresh)
	//{
	//	//这是因为贴近集流体的铝丝也有可能被可能当做毛刺，那么既然被当做毛刺，在这里就不要再被分类为铝丝
	//	ippiSub_8u_C1IRSfs(MaskWhitelineImg.buf, MaskWhitelineImg.nPitch, m_AnotherThreshImg.buf, m_AnotherThreshImg.nPitch, roisize, 0);

	//}


	//方案二
	//m_AnotherThreshImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	//m_hBaseFun.KxThreshImage(SrcImg, m_AnotherThreshImg, m_hPara.m_nOtherAreaThresh, 255);
	//
	//int offset = 3;
	//int  up = std::max(0, middleidx -_WHITELINE_HEIGHT / 2 - offset);
	//int down = std::min(SrcImg.nHeight - 1, middleidx + _WHITELINE_HEIGHT / 2 + offset);
	//IppiSize masksize = {SrcImg.nWidth, down - up + 1};
	//ippiSet_8u_C1R(0, m_AnotherThreshImg.buf + up * m_AnotherThreshImg.nPitch, m_AnotherThreshImg.nPitch, masksize);
	//
	//m_MaskDilateImg.Init(MaskWhitelineImg.nWidth, m_MaskDilateImg.nHeight);
	//m_hBaseFun.KxDilateImage(MaskWhitelineImg, m_MaskDilateImg, 1, 5);
	//
	//ippiSub_8u_C1IRSfs(m_MaskDilateImg.buf, m_MaskDilateImg.nPitch, m_AnotherThreshImg.buf, m_AnotherThreshImg.nPitch, roisize, 0);


	//m_hBlobAnalyseResult.ToBlobParallel(m_AnotherThreshImg, CKxBlobAnalyse::_SORT_BYDOTS, _MAX_DEFECT_COUNT - m_hResult.nDefectCount, 1, 0);
	//for (int i = 0; i < std::min(m_hBlobAnalyseResult.GetBlobCount(), int(_DIRTY)); i++)
	//{
	//	int nIndx = m_hResult.nDefectCount++;
	//	if (nIndx <= _MAX_DEFECT_COUNT - 1)
	//	{
	//		int w = (std::min)(SrcImg.nWidth - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Width());
	//		int h = (std::min)(SrcImg.nHeight - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Height());
	//		m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", 2);//暂时拿第八位作为标志位(0是特指断的区域，1是线上的，2是其它区域)
	//		m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nDots*1.0f);
	//		m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nEnergy*1.0f);
	//		m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.left* 1.0f));
	//		m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.top* 1.0f));
	//		m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", w * 1.0f);
	//		m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", h * 1.0f);
	//		m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", h * 1.95);
	//		m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", w * 1.95);

	//		m_hResult.nCheckStatus = _BLOBERR;
	//		//m_hResult.szErrInfo = "have some blobs";
	//	}

	//}

	//方案三：
	m_AnotherThreshImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	m_hBaseFun.KxThreshImage(SrcImg, m_AnotherThreshImg, m_hPara.m_nOtherAreaThresh, 255);
	
	//int offset = 3;
	//int  up = std::max(0, middleidx -_WHITELINE_HEIGHT / 2 - offset);
	//int down = std::min(SrcImg.nHeight - 1, middleidx + _WHITELINE_HEIGHT / 2 + offset);
	//IppiSize masksize = {SrcImg.nWidth, down - up + 1};
	//ippiSet_8u_C1R(0, m_AnotherThreshImg.buf + up * m_AnotherThreshImg.nPitch, m_AnotherThreshImg.nPitch, masksize);

	if (m_hPara.m_nOtherAreaThresh >= m_hPara.m_nWhiteLineThresh)
	{
		ippiSub_8u_C1IRSfs(MaskImg.buf, MaskImg.nPitch, m_AnotherThreshImg.buf, m_AnotherThreshImg.nPitch, roisize, 0);
	}
	else
	{
		// 在这一步中可能会对铝丝的高度检查造成误差
		m_MaskDilateImg.Init(MaskImg.nWidth, m_MaskDilateImg.nHeight);
		m_hBaseFun.KxDilateImage(MaskImg, m_MaskDilateImg, 1, 5);
		ippiSub_8u_C1IRSfs(m_MaskDilateImg.buf, m_MaskDilateImg.nPitch, m_AnotherThreshImg.buf, m_AnotherThreshImg.nPitch, roisize, 0);
	}

	//这是为了掩模掉中间断的区域容易造成的缺陷
	int offset = 3;
	int  up = std::max(0, middleidx - _WHITELINE_HEIGHT / 2 - offset);
	int down = std::min(SrcImg.nHeight - 1, middleidx + _WHITELINE_HEIGHT / 2 + offset);
	IppiSize masksize = { SrcImg.nWidth, down - up + 1 };
	ippiSet_8u_C1R(0, m_AnotherThreshImg.buf + up * m_AnotherThreshImg.nPitch, m_AnotherThreshImg.nPitch, masksize);

	/*！
		这里算法的目的是为了将毛刺、拱所包含的区域完全掩模掉，因为极片的检测阈值与毛刺、拱的检测阈值
		都不相同，那么就会造成普通的掩模无法覆盖性的消除掉影响，就会造成同样的一片区域报出了两个很接
		近的缺陷
	*/
	if (vec_blob.size() != 0)
	{
		NestingVersus(m_AnotherThreshImg, vec_blob);
		vec_blob.clear();
		// TODO: 记得加上清除vec的内存
	}

	//开操作是为了将掩模掉之后产生的集流体边缘碎屑部分清除掉，但是考虑到有些缺陷本来就很小，故横向再做一次闭运算复原，
	//顺便也把一些挨得近的缺陷连接
	m_AnotherThreshOpenImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	m_AnotherThreshCloseImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	m_hBaseFun.KxOpenImage(m_AnotherThreshImg, m_AnotherThreshOpenImg, 1, 3);
	m_hBaseFun.KxCloseImage(m_AnotherThreshOpenImg, m_AnotherThreshCloseImg, 7, 7);


	m_hBlobAnalyseResult.ToBlobParallel(m_AnotherThreshCloseImg, CKxBlobAnalyse::_SORT_BYDOTS, _MAX_DEFECT_COUNT - m_hResult.nDefectCount, 1, 0);
	for (int i = 0; i < std::min(m_hBlobAnalyseResult.GetBlobCount(), int(_DIRTY)); i++)
	{
		int nIndx = m_hResult.nDefectCount;
		if (nIndx <= _MAX_DEFECT_COUNT - 1)
		{
			int w = (std::min)(SrcImg.nWidth - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Width());
			int h = (std::min)(SrcImg.nHeight - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Height());
			m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::OTHERAREA);
			m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nDots*1.0f);
			m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nEnergy*1.0f);
			m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.left* 1.0f));
			m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.top* 1.0f));
			m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", w * 1.0f);
			m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", h * 1.0f);
			m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", h * 1.95);
			m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", w * 1.95);
			m_hResult.nCheckStatus = _BLOBERR;
			m_hResult.nDefectCount++;
			//m_hResult.szErrInfo = "have some blobs";
		}
		else
		{
			break;
		}

	}

	return 1;
}

int CKxPlateEdgeCheck::NestingVersus(kxCImageBuf& SrcDstImg, std::vector <RectBlob>& vec_blob)
{
	cv::Mat solveimg;
	KxImageBufToMat(SrcDstImg, solveimg);
	cv::Rect comp;
	kxCImageBuf smallimg;
	for (int i = 0; i < vec_blob.size(); i++)
	{

		//cv::Mat roiimg = solveimg(cv::Rect(vec_blob[i].x, vec_blob[i].y, vec_blob[i].width, vec_blob[i].height));
		//smallimg.SetImageBuf(SrcDstImg.buf + )
		//m_hBlobAnalyseResult.ToBlobParallel(m_AnotherThreshCloseImg, CKxBlobAnalyse::_SORT_BYDOTS, _MAX_DEFECT_COUNT - m_hResult.nDefectCount, 1, 0);
		//cv::Point seed = GetSeedPoint(SrcDstImg, vec_blob[i]);
		//if (seed != cv::Point(-1, -1))
		//{
		//	cv::floodFill(solveimg, seed, cv::Scalar(100), &comp, cv::Scalar(0), cv::Scalar(200), 8);
		//}

		// 临时方案
		IppiSize roisize = {vec_blob[i].width, vec_blob[i].height};
		ippiSet_8u_C1R(255, solveimg.data + vec_blob[i].x + vec_blob[i].y*solveimg.step, solveimg.step, roisize);
		cv::Point seed(vec_blob[i].x, vec_blob[i].y);
		cv::floodFill(solveimg, seed, cv::Scalar(100), &comp, cv::Scalar(0), cv::Scalar(200), 8);

	}
	m_hBaseFun.KxThreshImage(SrcDstImg, SrcDstImg, 101, 255);//结果已经存入SrcDstImg,正常返回即可
	return 1;
}

cv::Point CKxPlateEdgeCheck::GetSeedPoint(kxCImageBuf& SrcDstImg, RectBlob& vec_blob)
{
	for (int y = vec_blob.y; y < vec_blob.y + vec_blob.height; y++)
	{
		for (int x = vec_blob.x; x < vec_blob.x + vec_blob.width; x++)
		{
			if (SrcDstImg.buf[y*SrcDstImg.nPitch + x] != 0)
			{
				return cv::Point(x, y);
			}
		}
	}
	return cv::Point(-1, -1);
}

float CKxPlateEdgeCheck::getrate2(const kxCImageBuf& WhitelineArea, int heightmid, int nstart, int nend)
{//为更精准的区分毛刺，因为毛刺有可能略微拱起，导致原先的getrate更倾向于将物体识别为拱；缺点是遇到宽的拱容易定位为毛刺

	// todo:在中线范围找
	int offset = 50;
	int x = std::max(0, nstart - offset);
	int width = std::min(WhitelineArea.nWidth, nend - nstart + 1 + offset * 2);
	int y = std::max(0, heightmid - _WHITELINE_HEIGHT);
	int height = std::min(WhitelineArea.nHeight, heightmid + _WHITELINE_HEIGHT) - y;
	IppiSize smallimgsize = { width, height };
	kxCImageBuf smallimg;
	smallimg.Init(smallimgsize.width, smallimgsize.height);
	ippiCopy_8u_C1R(WhitelineArea.buf + x + y*WhitelineArea.nPitch, WhitelineArea.nPitch, smallimg.buf, smallimg.nPitch, smallimgsize);
	
	Ipp32f *vproject = ippsMalloc_32f(smallimgsize.height);
	m_hBaseFun.KxProjectImage(smallimg, 1, vproject, 255);

	int middle = GetMid(vproject, smallimgsize.height, 1); 
	if (middle < 0) 
	{
		ippsFree(vproject);
		return -1;
	}
	middle = middle + y;//变为整幅图像的偏移
	int recordlen = nend - nstart + 1;

	Ipp32f result = 0;

	for (int i = nstart; i <= nend; i++)
	{
		if (WhitelineArea.buf[i + middle * WhitelineArea.nPitch] != 0)
		{
			result += 1;
		}
	}
	ippsFree(vproject);
	if (result < 1e-8)
	{
		return 100;
	}
	else
	{
		return float(recordlen) / result;
	}
	
}

int CKxPlateEdgeCheck::FloodfilltoGetROI(kxCImageBuf& SrcImg, kxCImageBuf& DstImg, IppiPoint& seed)
{
	IppiSize roisize = { SrcImg.nWidth, SrcImg.nHeight };
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	DstImg.SetImageBuf(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, SrcImg.nChannel, true);
	int bufferlen = 0;
	ippiFloodFillGetSize(roisize, &bufferlen);
	Ipp8u* buffer = ippsMalloc_8u(bufferlen);
	IppiConnectedComp regionmsg;
	IppStatus status = ippiFloodFill_8Con_8u_C1IR(SrcImg.buf, SrcImg.nPitch, roisize, seed, 0, &regionmsg, buffer);
	ippiSub_8u_C1IRSfs(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roisize, 0);
	//ippiThreshold_Val_8u_C1IR(SrcDstImg.buf, SrcDstImg.nPitch, roisize, 100, 0, ippCmpGreater);
	//ippiThreshold_Val_8u_C1IR(SrcDstImg.buf, SrcDstImg.nPitch, roisize, 99, 255, ippCmpGreater);

	//ippiThreshold_LTValGTVal_8u_C1IR(SrcDstImg.buf, SrcDstImg.nPitch, roisize, 101, 255, 254, 0);
	ippsFree(buffer);
	if (status != ippStsNoErr)
		return 0;
	else
		return 1;
}

/*!
	data:			2019.04.25
	author:			HYH
	description:	负极，负极的成像与正极不太一致，总体呈现集流体偏细，掉粉问题更严重但不需要报出来
*/
int CKxPlateEdgeCheck::MoveFrontCut(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall)
{
	KxCallStatus hcallstatus;
	int singlewidth = SrcImg.nWidth / _CUT_NUM;
	int midx = 0;
	m_BigBlobImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	m_BigBlobNoMaskImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	IppiSize bigsize = {SrcImg.nWidth, SrcImg.nHeight};
	ippiSet_8u_C1R(0, m_BigBlobImg.buf, m_BigBlobImg.nPitch, bigsize);
	ippiSet_8u_C1R(0, m_BigBlobNoMaskImg.buf, m_BigBlobNoMaskImg.nPitch, bigsize);

	int data[_CUT_NUM];

	////// ------------------方案一-------------------//
	//for (int i = 0; i < _CUT_NUM; i++)
	//{
	//	IppiSize roisize = { singlewidth, SrcImg.nHeight };
	//	m_SmallCutImg.Init(singlewidth, SrcImg.nHeight);
	//	kxRect<int> rc(i * singlewidth, 0, (i+1)*singlewidth - 1, SrcImg.nHeight - 1);
	//	m_hBaseFun.KxCopyImage(SrcImg, m_SmallCutImg, rc);
	//	m_SmallCenterImg.Init(singlewidth, SrcImg.nHeight);
	//	GetCenterROI(m_SmallCutImg, m_SmallCenterImg, m_hPara.m_nWhiteLineThresh, midx, hcallstatus);
	//	middle += midx;
	//	std::cout << midx << "    ";
	//	int top = std::max(0, midx - 4);
	//	int bot = std::min(SrcImg.nHeight - 1, midx + 4);
	//	IppiSize masksize = { singlewidth, bot - top + 1 };
	//	ippiSet_8u_C1R(0, m_SmallCenterImg.buf + top * m_SmallCenterImg.nPitch, m_SmallCenterImg.nPitch, masksize);
	//	
	//	ippiCopy_8u_C1R(m_SmallCenterImg.buf, m_SmallCenterImg.nPitch, m_BigBlobImg.buf + i*singlewidth, m_BigBlobImg.nPitch, roisize);
	//}

	Ipp32f * pProject = ippsMalloc_32f(SrcImg.nHeight);

	for (int i = 0; i < _CUT_NUM; i++)
	{
		memset(pProject, 0, sizeof(Ipp32f)*SrcImg.nHeight);
		IppiSize roisize = { singlewidth, SrcImg.nHeight };
		m_SmallCutImg.Init(singlewidth, SrcImg.nHeight);
		kxRect<int> rc(i * singlewidth, 0, (i+1)*singlewidth - 1, SrcImg.nHeight - 1);
		m_hBaseFun.KxCopyImage(SrcImg, m_SmallCutImg, rc);
		m_SmallThreshImg.Init(singlewidth, SrcImg.nHeight);
		m_hBaseFun.KxThreshImage(m_SmallCutImg, m_SmallThreshImg, m_hPara.m_nWhiteLineThresh, 255);

		m_hBaseFun.KxProjectImage(m_SmallThreshImg, 1, pProject, 255);
		midx = GetMid(pProject, m_SmallThreshImg.nHeight, 3);

		m_SmallImgArray[i] = m_SmallThreshImg;
		data[i] = midx;
	}

	ippsFree(pProject);
	int bestcenter = CalcuMin_Variance(data, _CUT_NUM);
	const int offset = 0;
	const int maskoffset = 2;

	int curcenter = 0;

	for (int i = 0; i < _CUT_NUM; i++)
	{
		if (abs(data[i] - bestcenter) > 2)//根据正极的经验来看，差别超过两个像素便会造成影响
		{
			curcenter = bestcenter;
		}
		else
		{
			curcenter = data[i];
		}

		int whitetop = std::max(0, curcenter - offset);
		int whitebot = std::min(m_SmallImgArray[i].nHeight - 1, curcenter + offset);
		m_SmallCenterRc.Init(m_SmallImgArray[i].nWidth, whitebot - whitetop + 1);
		kxRect<int> rc(0, whitetop, m_SmallImgArray[i].nWidth - 1, whitebot);
		IppiSize whitemask = { m_SmallImgArray[i].nWidth, rc.Height() };
		m_hBaseFun.KxCopyImage(m_SmallImgArray[i], m_SmallCenterRc, rc);
		ippiSet_8u_C1R(255, m_SmallImgArray[i].buf + whitetop * m_SmallImgArray[i].nPitch, m_SmallImgArray[i].nPitch, whitemask);

		//TODO: 20190425 下面程序会出现一种bug的可能，铝粉离集流体就差了一两个像素，但在上面的掩模过程中不小心掩模到它了，之后就把这块铝粉算进来了
		m_SmallCenterImg.Init(m_SmallImgArray[i].nWidth, m_SmallImgArray[i].nHeight);
		IppiPoint seed = { 0, curcenter };
		FloodfilltoGetROI(m_SmallImgArray[i], m_SmallCenterImg, seed);

		IppiSize roisize = { singlewidth, SrcImg.nHeight };
		//拷贝中心区到原图上
		ippiCopy_8u_C1R(m_SmallCenterRc.buf, m_SmallCenterRc.nPitch, m_SmallCenterImg.buf + whitetop * m_SmallCenterImg.nPitch, m_SmallCenterImg.nPitch, whitemask);
		//保存这张未被掩膜的图
		ippiCopy_8u_C1R(m_SmallCenterImg.buf, m_SmallCenterImg.nPitch, m_BigBlobNoMaskImg.buf + i*m_SmallCenterImg.nWidth, m_BigBlobNoMaskImg.nPitch, roisize);

		//掩模中心区，得到
		int blacktop = std::max(0, curcenter - maskoffset);
		int blackbot = std::min(SrcImg.nHeight - 1, curcenter + maskoffset);
		IppiSize masksize = { singlewidth, blackbot - blacktop + 1 };
		ippiSet_8u_C1R(0, m_SmallCenterImg.buf + blacktop * m_SmallCenterImg.nPitch, m_SmallCenterImg.nPitch, masksize);
		ippiCopy_8u_C1R(m_SmallCenterImg.buf, m_SmallCenterImg.nPitch, m_BigBlobImg.buf + i*m_SmallCenterImg.nWidth, m_BigBlobImg.nPitch, roisize);
	}



	// -----------------  方案一：老式blob --------------------//

	// 1. 检测毛刺
	m_hBlobAnalyseResult.ToBlobParallel(m_BigBlobImg, CKxBlobAnalyse::_SORT_BYDOTS, 16, 1, 0);
	for (int i = 0; i < std::min(m_hBlobAnalyseResult.GetBlobCount(), 16); i++)
	{
		int nIndx = m_hResult.nDefectCount;
		if (nIndx <= _MAX_DEFECT_COUNT - 1)
		{
			int w = (std::min)(SrcImg.nWidth - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Width());
			int h = (std::min)(SrcImg.nHeight - 1, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.Height());
			m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::LINE);//暂时拿第八位作为标志位(0是特指断的区域，1是线上的，2是其它区域)
			m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nDots*1.0f);
			m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", m_hBlobAnalyseResult.GetSortSingleBlob(i).m_nEnergy*1.0f);
			m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.left* 1.0f));
			m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, m_hBlobAnalyseResult.GetSortSingleBlob(i).m_rc.top* 1.0f));
			m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", w * 1.0f);
			m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", h * 1.0f);
			m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", h * 1.95);
			m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", w * 1.95);
			m_hResult.nCheckStatus = _BLOBERR;
			m_hResult.nDefectCount++;
		}
		else
		{
			break;
		}
	}

	//2.检测掉粉情况，也即正极的断
	Ipp32f * vProject = ippsMalloc_32f(SrcImg.nWidth);
	float brokelen = 0;
	m_hBaseFun.KxProjectImage(m_BigBlobNoMaskImg, _Vertical_Project_Dir, vProject);
	for (int i = 0; i < SrcImg.nWidth; i++)
	{
		if (vProject[i] < 1e-8)
		{
			brokelen += 1;
		}
	}

	float rate = brokelen / SrcImg.nWidth;
	int nIndx = m_hResult.nDefectCount;
	if (nIndx <= _MAX_DEFECT_COUNT - 1)
	{
		m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::DIAOFENRATE);
		m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", 0);
		m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", 0);
		m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", 0);
		m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", 0);
		m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", SrcImg.nWidth * 1.0f);
		m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", SrcImg.nHeight * 1.0f);
		m_hResult.mapFeaturelists[nIndx][6] = std::make_pair("中心占比率", rate);
		m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", 0);
		m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", 0);
		m_hResult.nCheckStatus = _BLOBERR;
		m_hResult.nDefectCount++;
	}





	// -----------------  方案二：----------------------//
	//int aresult = AnalyseSmallBlob(SrcImg, hcallstatus);
	middle = bestcenter;

	/*!
		目前来看两套算法各有各的问题，方案一是无法应对杂质太多造成的中心点搜索错误
		方案二是针对一做了改进，但是极片本身可能没拉的足够的正直，左右存在偏差，这样便会有其他问题
		待进厂拷贝一批最新数据之后再看有没有
	*/

	return 1;

}

int CKxPlateEdgeCheck::CalcuMin_Variance(int data[], int len)
{// n个值中，计算以选中的当前值为中心，其他值与该值方差的和；得到的最小方差和对应的数组值则为中心
	std::vector<int> result;
	for (int i = 0; i < len; i++)
	{
		int nsum = 0;
		for (int j = 0; j < len; j++)
		{
			nsum += (data[i] - data[j])*(data[i] - data[j]);
		}
		result.push_back(nsum);
	}

	std::vector<int>::iterator biggest = std::min_element(std::begin(result), std::end(result));
	int minindex = std::distance(std::begin(result), biggest);
	std::vector<int>().swap(result);
	return data[minindex];
}

int CKxPlateEdgeCheck::AnalyseSmallBlob(const kxCImageBuf& SrcImg, KxCallStatus& hCall)
{//分析单个缺陷的特征，包括形态以及灰度分布，主要用于负极
	cv::Mat matblobimg, matSrcImg;//matblobimg是二值化图，matSrcImg是原图
	KxImageBufToMat(m_BigBlobImg, matblobimg);
	KxImageBufToMat(SrcImg, matSrcImg);

	cv::findContours(matblobimg, m_contours, m_hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	//std::vector<std::vector<cv::Point> >	 testcontours;
	//cv::findContours(matblobimg, testcontours, m_hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

	cv::Point2f vertVect[4];

	for (int i = 0; i < m_contours.size(); i++)
	{
		//int area2 = cv::contourArea(m_contours[i]);
		cv::RotatedRect box = cv::minAreaRect(m_contours[i]);
		box.points(vertVect);
		double angle = getRcDegree(vertVect);
		double lw_ration = box.size.width > box.size.height ? double(box.size.width) / box.size.height : double(box.size.height) / box.size.width;
		double nlen = box.size.width > box.size.height ? box.size.width : box.size.height;
		double nthickness = box.size.width < box.size.height ? box.size.width : box.size.height;
		if (lw_ration > _MIN_MINAREA_LW_RATIO && angle > _MIN_ANGLE && angle < _MAX_ANGLE && 
			nlen > _NEGATIVE_GLITCH_MIN && nthickness < _NEGATIVE_GLITCH_THICK_MAX)
		{// 宽高比、角度、长度、厚度
			cv::Rect bounrect = getboundRect(vertVect, matSrcImg.cols, matSrcImg.rows);
			cv::Mat matenergyimg(bounrect.size(), matSrcImg.type());
			cv::Mat matsmallsrcimg = matSrcImg(bounrect);
			cv::Mat matsmallblobimg = matblobimg(bounrect);
			cv::bitwise_and(matsmallsrcimg, matsmallblobimg, matenergyimg);

			//cv::threshold(matenergyimg, )
			//下面进行灰度			
			//Ipp8u ndividevalue = 0;
			//IppiSize blobsize = { matenergyimg.cols, matenergyimg.rows };
			//ippiComputeThreshold_Otsu_8u_C1R(matenergyimg.data, matenergyimg.step, blobsize, &ndividevalue);
			//std::cout << ndividevalue << std::endl;

			if (m_hResult.nDefectCount <= _MAX_DEFECT_COUNT - 1)
			{
				int nIndx = m_hResult.nDefectCount;
				m_hResult.mapFeaturelists[nIndx][8] = std::make_pair("标志位", DEFECT_FLAG::LINE);//暂时拿第八位作为标志位(0是特指断的区域，1是线上的，2是其它区域)
				m_hResult.mapFeaturelists[nIndx][0] = std::make_pair("点数", 1.0f);
				m_hResult.mapFeaturelists[nIndx][1] = std::make_pair("能量", 1.0f);
				m_hResult.mapFeaturelists[nIndx][2] = std::make_pair("X坐标", (std::max)(0.0f, bounrect.x* 1.0f));
				m_hResult.mapFeaturelists[nIndx][3] = std::make_pair("Y坐标", (std::max)(0.0f, bounrect.y* 1.0f));
				m_hResult.mapFeaturelists[nIndx][4] = std::make_pair("缺陷宽", bounrect.width * 1.0f);
				m_hResult.mapFeaturelists[nIndx][5] = std::make_pair("缺陷高", bounrect.height * 1.0f);
				m_hResult.mapFeaturelists[nIndx][7] = std::make_pair("物理高度", nlen * 1.95);
				m_hResult.mapFeaturelists[nIndx][9] = std::make_pair("物理宽度", nthickness * 1.95);
				m_hResult.nCheckStatus = _BLOBERR;
				m_hResult.nDefectCount++;
			}
			else
			{
				break;
			}
		}
		
		
		//cv::Rect r = cv::boundingRect(m_contours[]);
		//for (int j = 0; j < 4;  j++)
		//	cv::line(matblobimg, vtx[j], vtx[(j + 1) % 4], cv::Scalar(100), 1, cv::LINE_AA);
	}



	return 0;

}

double CKxPlateEdgeCheck::calcLineDegree(const cv::Point2f& firstPt, const cv::Point2f& secondPt)
{
	double curLineAngle = 0.0f;
	if (secondPt.x - firstPt.x != 0)
	{
		curLineAngle = atan(static_cast<double>(firstPt.y - secondPt.y) / static_cast<double>(secondPt.x - firstPt.x));
		if (curLineAngle < 0)
		{
			curLineAngle += CV_PI;
		}
	}
	else
	{
		curLineAngle = CV_PI / 2.0f; //90度
	}
	return curLineAngle*180.0f / CV_PI;
}

double CKxPlateEdgeCheck::getRcDegree(const cv::Point2f vertVect[4])
{
	double degree = 0.0f;
	//cv::Point2f vertVect[4];
	//box.points(vertVect);
	//line 1
	const double firstLineLen = (vertVect[1].x - vertVect[0].x)*(vertVect[1].x - vertVect[0].x) +
		(vertVect[1].y - vertVect[0].y)*(vertVect[1].y - vertVect[0].y);
	//line 2
	const double secondLineLen = (vertVect[2].x - vertVect[1].x)*(vertVect[2].x - vertVect[1].x) +
		(vertVect[2].y - vertVect[1].y)*(vertVect[2].y - vertVect[1].y);
	if (firstLineLen > secondLineLen)
	{
		degree = calcLineDegree(vertVect[0], vertVect[1]);
	}
	else
	{
		degree = calcLineDegree(vertVect[2], vertVect[1]);
	}
	return degree;
}

cv::Rect CKxPlateEdgeCheck::getboundRect(const cv::Point2f vertVect[4], int nmaxwidth, int nmaxheight)
{// 从最小外接矩阵得到正常外接矩阵
	int minx = 100000000;
	int miny = 100000000;
	int maxx = 0;
	int maxy = 0;
	for (int i = 0; i < 4; i++)
	{
		if (vertVect[i].x < minx)
		{
			minx = vertVect[i].x;
		}
		if (vertVect[i].y < miny)
		{
			miny = vertVect[i].y;
		}
		if (vertVect[i].x > maxx)
		{
			maxx = std::min(int(std::ceil(vertVect[i].x)), nmaxwidth - 1);
		}
		if (vertVect[i].y > maxy)
		{
			maxy = std::min(int(std::ceil(vertVect[i].y)), nmaxheight - 1);
		}
	}

	return cv::Rect(minx, miny, maxx - minx + 1, maxy - miny + 1);

	//return 0;
}

int CKxPlateEdgeCheck::MoveFrontCut2(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall)
{
	// ------------- 应对负极新光源, 目的是准确的分析到毛刺的位置 -------------//

	KxCallStatus hcallstatus;
	int singlewidth = SrcImg.nWidth / _CUT_NUM;
	int midx = 0;
	m_BigBlobImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	IppiSize bigsize = { SrcImg.nWidth, SrcImg.nHeight };
	ippiSet_8u_C1R(0, m_BigBlobImg.buf, m_BigBlobImg.nPitch, bigsize);
	int data[_CUT_NUM];

	Ipp32f * pProject = ippsMalloc_32f(SrcImg.nHeight);

	for (int i = 0; i < _CUT_NUM; i++)
	{
		memset(pProject, 0, sizeof(Ipp32f)*SrcImg.nHeight);
		IppiSize roisize = { singlewidth, SrcImg.nHeight };
		m_SmallCutImg.Init(singlewidth, SrcImg.nHeight);
		kxRect<int> rc(i * singlewidth, 0, (i + 1)*singlewidth - 1, SrcImg.nHeight - 1);
		m_hBaseFun.KxCopyImage(SrcImg, m_SmallCutImg, rc);
		m_SmallThreshImg.Init(singlewidth, SrcImg.nHeight);
		m_hBaseFun.KxThreshImage(m_SmallCutImg, m_SmallThreshImg, m_hPara.m_nWhiteLineThresh, 255);

		m_hBaseFun.KxProjectImage(m_SmallThreshImg, 1, pProject, 255);
		midx = GetMid(pProject, m_SmallThreshImg.nHeight, 3);

		m_SmallImgArray[i] = m_SmallThreshImg;
		data[i] = midx;
	}

	ippsFree(pProject);
	int bestcenter = CalcuMin_Variance(data, _CUT_NUM);
	const int offset = 0;
	int curcenter = 0;

	for (int i = 0; i < _CUT_NUM; i++)
	{
		if (abs(data[i] - bestcenter) > 2)//根据正极的经验来看，差别超过两个像素便会造成影响
		{
			curcenter = bestcenter;
		}
		else
		{
			curcenter = data[i];
		}

		int whitetop = std::max(0, curcenter - offset);
		int whitebot = std::min(m_SmallImgArray[i].nHeight - 1, curcenter + offset);
		m_SmallCenterRc.Init(m_SmallImgArray[i].nWidth, whitebot - whitetop + 1);
		kxRect<int> rc(0, whitetop, m_SmallImgArray[i].nWidth - 1, whitebot);
		IppiSize whitemask = { m_SmallImgArray[i].nWidth, rc.Height() };
		m_hBaseFun.KxCopyImage(m_SmallImgArray[i], m_SmallCenterRc, rc);
		ippiSet_8u_C1R(255, m_SmallImgArray[i].buf + whitetop * m_SmallImgArray[i].nPitch, m_SmallImgArray[i].nPitch, whitemask);

		//TODO: 20190425 下面程序会出现一种bug的可能，铝粉离集流体就差了一两个像素，但在上面的掩模过程中不小心掩模到它了，之后就把这块铝粉算进来了
		m_SmallCenterImg.Init(m_SmallImgArray[i].nWidth, m_SmallImgArray[i].nHeight);
		IppiPoint seed = { 0, curcenter };
		FloodfilltoGetROI(m_SmallImgArray[i], m_SmallCenterImg, seed);

		IppiSize roisize = { singlewidth, SrcImg.nHeight };
		//拷贝中心区到原图上
		ippiCopy_8u_C1R(m_SmallCenterRc.buf, m_SmallCenterRc.nPitch, m_SmallCenterImg.buf + whitetop * m_SmallCenterImg.nPitch, m_SmallCenterImg.nPitch, whitemask);
		//拷贝至大图
		ippiCopy_8u_C1R(m_SmallCenterImg.buf, m_SmallCenterImg.nPitch, m_BigBlobImg.buf + i*m_SmallCenterImg.nWidth, m_BigBlobImg.nPitch, roisize);

		//掩模中心区，得到
		//int blacktop = std::max(0, curcenter - 4);
		//int blackbot = std::min(SrcImg.nHeight - 1, curcenter + 4);
		//IppiSize masksize = { singlewidth, blackbot - blacktop + 1 };
		//ippiSet_8u_C1R(0, m_SmallCenterImg.buf + blacktop * m_SmallCenterImg.nPitch, m_SmallCenterImg.nPitch, masksize);
	}


	//2.搜集流体位置信息
	cv::Rect* allrect = new cv::Rect[m_BigBlobImg.nWidth];

	for (int i = 0; i < m_BigBlobImg.nWidth; i++)
	{
		cv::Point pos1, pos2;
		bool bfindtop = false;
		for (int j = 0; j < m_BigBlobImg.nHeight; j++)
		{
			if (m_BigBlobImg.buf[j*m_BigBlobImg.nWidth + i] != 0)
			{
				pos1 = cv::Point(i, j);
				bfindtop = true;
				break;
			}
		}

		if (bfindtop)
		{
			for (int j = m_BigBlobImg.nHeight - 1; j >= 0; j--)
			{
				if (m_BigBlobImg.buf[j*m_BigBlobImg.nWidth + i] != 0)
				{
					pos2 = cv::Point(i, j);
					break;
				}
			}
			allrect[i] = cv::Rect(pos1, pos2);
		}
		else
		{// 没搜到
			allrect[i] = cv::Rect(-1, -1, 0, 0);
		}
	}



	return 1;

}