#include "stdafx.h"

#include "KxBlobAnalyse.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "tbb/partitioner.h"
using namespace tbb;


void KxMakeConnectedComponents::operator()(const tbb::blocked_range<size_t>& r)
{
	CKxBlobAnalyse::SingleBlobInfo* pComponents = m_pComponents;
	for (size_t y = r.begin(); y != r.end(); ++y)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			int nLabel = m_ImgLabel[y*m_nWidth + x];
			int nEnergy = m_SrcImg[y*m_nWidth + x];
			int nIndex = nLabel - CKxBlobAnalyse::_Min_Lable;
			if (nIndex >= 0)
			{
				if (pComponents[nIndex].m_nLabel == 0)
				{
					//pComponents[nIndex].init(nLabel, x, y);
					//std::cout << nLabel << std::endl;

					pComponents[nIndex].m_nLabel = nLabel;
					pComponents[nIndex].m_PtSeed.setup(x, y);
					pComponents[nIndex].m_nDots = 1;
					pComponents[nIndex].m_nEnergy = nEnergy;
					pComponents[nIndex].m_rc.setup(x, y, x, y);
					pComponents[nIndex].m_nSize = pComponents[nIndex].m_rc.Width() * pComponents[nIndex].m_rc.Height();

				}
				else
				{
					//pComponents[nIndex].insertPoint(x, y);
					pComponents[nIndex].m_nDots += 1;
					pComponents[nIndex].m_nEnergy += nEnergy;// hyh 20191024 这里的power暂时没有意义，因为并未传入原图

					pComponents[nIndex].m_rc.SetLeft(gMin(x, pComponents[nIndex].m_rc.left));
					pComponents[nIndex].m_rc.SetRight(gMax(x, pComponents[nIndex].m_rc.right));
					pComponents[nIndex].m_rc.SetTop(gMin(y, pComponents[nIndex].m_rc.top));
					pComponents[nIndex].m_rc.SetBottom(gMax(y, pComponents[nIndex].m_rc.bottom));
					pComponents[nIndex].m_nSize = pComponents[nIndex].m_rc.Width() * pComponents[nIndex].m_rc.Height();



					//int nheight = ;
					//pComponents[nIndex].m_rc.setup(x, y, x, y);

				}
			}
		}
	}
}

void KxMakeConnectedComponents::join(KxMakeConnectedComponents& y)
{
	for (int i = 0; i < m_nComponentCount; i++)
	{
		if (y.m_pComponents[i].m_nLabel == 0)//过来合并的label为0直接返回
		{
			continue;
		}
		if (m_pComponents[i].m_nLabel == 0)
		{
			//m_nLabel = one.getLabel();
			//m_nDots = one.getDots();
			//m_nPower = one.getPower();
			//m_rcBound = one.m_rcBound;

			m_pComponents[i].m_nLabel = y.m_pComponents[i].m_nLabel;
			m_pComponents[i].m_PtSeed = y.m_pComponents[i].m_PtSeed;
			m_pComponents[i].m_nDots = y.m_pComponents[i].m_nDots;
			m_pComponents[i].m_nEnergy = y.m_pComponents[i].m_nEnergy;// hyh 20191024 这里的power暂时没有意义，因为并未传入原图
			m_pComponents[i].m_rc = y.m_pComponents[i].m_rc;
		}
		else if (m_pComponents[i].m_nLabel == y.m_pComponents[i].m_nLabel)
		{
			//m_nDots += one.getDots();
			//m_nPower += one.getPower();
			//m_rcBound.left = gMin(one.m_rcBound.left, m_rcBound.left);
			//m_rcBound.right = gMax(one.m_rcBound.right, m_rcBound.right);
			//m_rcBound.top = gMin(one.m_rcBound.top, m_rcBound.top);
			//m_rcBound.bottom = gMax(one.m_rcBound.bottom, m_rcBound.bottom);

			m_pComponents[i].m_nDots += y.m_pComponents[i].m_nDots;
			m_pComponents[i].m_nEnergy += y.m_pComponents[i].m_nEnergy;// hyh 20191024 这里的power暂时没有意义，因为并未传入原图
			m_pComponents[i].m_rc.left = gMin(m_pComponents[i].m_rc.left, y.m_pComponents[i].m_rc.left);
			m_pComponents[i].m_rc.right = gMax(m_pComponents[i].m_rc.right, y.m_pComponents[i].m_rc.right);
			m_pComponents[i].m_rc.top = gMin(m_pComponents[i].m_rc.top, y.m_pComponents[i].m_rc.top);
			m_pComponents[i].m_rc.bottom = gMax(m_pComponents[i].m_rc.bottom, y.m_pComponents[i].m_rc.bottom);
		}
	}




}



CKxBlobAnalyse::CKxBlobAnalyse()
{
	m_nCount = 0;
	m_nCountBlob = 0;
	m_nMinDots = 1;
	m_nOpenSize = 1;

	m_nGridXStep = 1;
	m_nGridYStep = 1;
	m_nGridX = 0;
	m_nGridY = 0;

	m_pBlobInfo = NULL;
	m_pSortBlobInfo = NULL;
	m_pBlobArea = NULL;
	m_pGrid = NULL;

	m_bOpenAdditionalInfo = true;
	m_bUseFoodFillAlogrithm = true;

	m_nConnectType = _USE8;

}

CKxBlobAnalyse::~CKxBlobAnalyse()
{
	Clear();
	ClearSortInfo();

}

void CKxBlobAnalyse::Clear()
{
	// HYH 并行版本清除
	//if (m_pBlobInfo)
	//{
	//	delete []m_pBlobInfo;
	//	m_pBlobInfo = NULL;
	//}
	if (m_pBlobArea)
	{
		delete[]m_pBlobArea;
		m_pBlobArea = NULL;
	}
	m_nCount = 0;
	m_nCountBlob = 0;

}

int CKxBlobAnalyse::ToBlob(const unsigned char* buf, int nWidth, int nHeight, int nPitch)
{
	KxCallStatus hCall;
	kxRect<int> rc;
	rc.setup(0, 0, nWidth - 1, nHeight - 1);
	return ToBlob(buf, nPitch, rc, hCall);
}

int CKxBlobAnalyse::ToBlob(const unsigned char* buf, int nWidth, int nHeight, int nPitch, KxCallStatus& hCall)
{
	kxRect<int> rc;
	rc.setup(0, 0, nWidth - 1, nHeight - 1);
	return ToBlob(buf, nPitch, rc, hCall);
}

int CKxBlobAnalyse::ToBlob(const unsigned char* buf, int nPitch, const kxRect<int>& rcBlob)
{
	KxCallStatus hCall;
	return ToSimpBlob(buf, nPitch, rcBlob, hCall);
}

int CKxBlobAnalyse::ToBlob(const unsigned char* buf, int nPitch, const kxRect<int>& rcBlob, KxCallStatus& hCall)
{
	return ToSimpBlob(buf, nPitch, rcBlob, hCall);

}

int CKxBlobAnalyse::ToBlob(const kxCImageBuf& SrcImg)
{
	KxCallStatus hCall;
	return ToBlob(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, hCall);

}

int CKxBlobAnalyse::ToBlob(const kxCImageBuf& SrcImg, KxCallStatus& hCall)
{
	return ToBlob(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, hCall);

}

int CKxBlobAnalyse::MergeSomeConnections(const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pDst, int nDstPitch, KxCallStatus& hCall)
{
	IppStatus  status = ippStsNoErr;
	//use open operate to merge some connections
	m_OpenImg.Init(nWidth, nHeight);
	if (m_nOpenSize > 1)
	{
		IppiBorderType borderType = ippBorderRepl;
		Ipp8u borderValue = 0;
		IppiSize roiSize = { nWidth, nHeight };
		IppiSize maskSize = { m_nOpenSize, m_nOpenSize };
		int nSpecSize = 0, nBufferSize = 0;
		if (check_sts(status = ippiMorphAdvGetSize_8u_C1R(roiSize, maskSize, &nSpecSize, &nBufferSize),
			"ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			return 0;
		}

		IppiMorphAdvState* pSpec = (IppiMorphAdvState*)ippsMalloc_8u(nSpecSize);
		Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
		m_MaskImg.Init(maskSize.width, maskSize.height);
		ippsSet_8u(1, m_MaskImg.buf, maskSize.width*maskSize.height);

		if (check_sts(status = ippiMorphAdvInit_8u_C1R(roiSize, m_MaskImg.buf, maskSize, pSpec, pBuffer),
			"ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			return 0;
		}

		if (check_sts(status = ippiMorphCloseBorder_8u_C1R(buf, nPitch, pDst, nDstPitch, roiSize, borderType, borderValue, pSpec, pBuffer),
			"ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			return 0;
		}

		ippsFree(pSpec);
		ippsFree(pBuffer);

	}
	else
	{
		IppiSize Roi = { nWidth, nHeight };

		if (check_sts(status = ippiCopy_8u_C1R(buf, nPitch, pDst, nWidth, Roi), "ippiCopy", hCall))
		{
			return 0;
		}


	}


	return 1;
}

int CKxBlobAnalyse::ToSimpBlob(const unsigned char* buf, int nPitch, const kxRect<int>& rcBlob, KxCallStatus& hCall)
{
	Clear(); //clear the data struct

	hCall.Clear();
	IppStatus status;

	IppiSize Roi = { rcBlob.Width(), rcBlob.Height() };
	if (m_BufferImg.buf == NULL || Roi.width != m_Img.nWidth || Roi.height != m_Img.nHeight)
	{
		int  nBufferSize;
		status = ippiLabelMarkersGetBufferSize_16u_C1R(Roi, &nBufferSize);

		if (check_sts(status, "ToSimpBlob_ippiLabelMarkersGetBufferSize", hCall))
		{
			return 0;
		}

		m_BufferImg.Init(nBufferSize, 1);
	}
	//copy a Img
	m_Img.Init(Roi.width, Roi.height);
	status = ippiCopy_8u_C1R(buf, nPitch, m_Img.buf, m_Img.nPitch, Roi);

	if (check_sts(status, "ToSimpBlob_ippiCopy", hCall))
	{
		return 0;
	}

	//merge some connections
	m_PreImg.Init(Roi.width, Roi.height);
	KxCallStatus hTempCall;
	MergeSomeConnections(buf, Roi.width, Roi.height, nPitch, m_PreImg.buf, m_PreImg.nPitch, hTempCall);

	if (check_sts(hTempCall, "ToSimpBlob_MergeSomeConnections", hCall))
	{
		return 0;
	}


	m_pTmpImg.Init(Roi.width, Roi.height);
	status = ippiConvert_8u16u_C1R(m_PreImg.buf + rcBlob.top * m_PreImg.nPitch + rcBlob.left, m_PreImg.nPitch, m_pTmpImg.buf, m_pTmpImg.nPitch, Roi);

	if (check_sts(status, "ToSimpBlob_ippiConvert", hCall))
	{
		return 0;
	}

	//first,label the connections components
	int nCount = 0;
	status = ippiLabelMarkers_16u_C1IR(m_pTmpImg.buf, m_pTmpImg.nPitch, Roi, _Min_Lable, _Min_Lable + _MAX_Lable_Count,
		(m_nConnectType == _USE8 ? ippiNormInf : ippiNormL1), &nCount, m_BufferImg.buf);

	if (check_sts(status, "ToSimpBlob_ippiLabelMarkers", hCall))
	{
		return 0;
	}

	if (nCount == 0)
	{
		return 1;
	}

	if (nCount >= _MAX_Lable_Count)
	{
		status = IppStatus(kxBlobAnalyseCountErr);
		if (check_sts(status, "ToSimpBlob_TheBlobCountsGreateMaxLabelCount", hCall))
		{
			return 0;
		}
	}

	if (nCount > _Max_FloodBlob_Count)
	{
		if (m_nGridX != rcBlob.Width() / m_nGridXStep || m_nGridY != rcBlob.Height() / m_nGridYStep)
		{
			SetGridXY(rcBlob.Width() / m_nGridXStep, rcBlob.Height() / m_nGridYStep);
		}
		hTempCall.Clear();
		ToGridBlob(m_PreImg.buf, m_PreImg.nPitch, rcBlob, hTempCall);
		m_bUseFoodFillAlogrithm = false;

		if (check_sts(hTempCall, "ToSimpBlob_ToGridBlob", hCall))
		{
			return 0;
		}
	}
	else
	{
		hTempCall.Clear();
		UseFloodAlgorithmComputeBlob(m_pTmpImg, nCount, hTempCall);
		if (check_sts(hTempCall, "ToSimpBlob_UseFloodAlgorithmComputeBlob", hCall))
		{
			return 0;
		}

		m_bUseFoodFillAlogrithm = true;
	}

	return 1;
}

int CKxBlobAnalyse::UseFloodAlgorithmComputeBlob(kxImg16u& Img16u, int nCount, KxCallStatus& hCall)
{
	// HYH 2021.04.06 ippsFind_8u 的ippscv函数已快被ipp抛弃，所以这里舍弃

	//IppStatus status;

	//hCall.Clear();
	////second, use flood fill algorithm to find every blob's info
	//m_pBlobInfo = new SingleBlobInfo[nCount];
	//status = ippsZero_8u((Ipp8u*)m_pBlobInfo, nCount*sizeof(SingleBlobInfo));

	//if (check_sts(status, "ippsZero", hCall))
	//{
	//	return 0;
	//}

	//int nBufferSize;
	//IppiSize Roi = { Img16u.nWidth, Img16u.nHeight };


	//status = ippiFloodFillGetSize(Roi, &nBufferSize);

	//if (check_sts(status, "ippiFloodFillGetSize", hCall))
	//{
	//	return 0;
	//}


	//m_BufferImgX.Init(nBufferSize, 1);


	//m_pImg16u.Init(Roi.width, Roi.height);
	//status = ippiCopy_16u_C1R(Img16u.buf, Img16u.nPitch, m_pImg16u.buf, m_pImg16u.nPitch, Roi);

	//if (check_sts(status, "ippiCopy_16u", hCall))
	//{
	//	return 0;
	//}


	//IppiConnectedComp pRegion;
	//int nLen = Roi.width * Roi.height;
	//int nPos = 0;

	//Ipp16u* ToFind = new Ipp16u[nCount];
	//for (int i = 0; i < nCount; i++)
	//{
	//	ToFind[i] = _Min_Lable + i;
	//}
	//IppiPoint seed;
	//int k = 0;
	////Ipp16u valFind;
	////int nPos1;
	//int nStart = 0;
	//int nIndex = 0;
	//int nSearchLen = nLen;
	//int nBlob = 0;

	//int n = _Min_Lable + _MAX_Lable_Count + 1;

	//while (k < nCount)
	//{
	//	//first, find a best seed
	//	//status =  ippsFindCAny_16u(m_pImg16u.buf + nStart, nSearchLen, ToFind, k, &nPos);
	//	status = ippsFind_8u((Ipp8u*)m_pImg16u.buf, nSearchLen*sizeof(Ipp16u), (Ipp8u*)(&ToFind[k]), sizeof(Ipp16u), &nPos);

	//	if (check_sts(status, "ippsFindCAny", hCall))
	//	{
	//		return 0;
	//	}

	//	seed.y = nPos / (Roi.width*sizeof(Ipp16u));
	//	seed.x = (nPos % (Roi.width*sizeof(Ipp16u))) / sizeof(Ipp16u);

	//	//valFind = m_pImg16u.buf[seed.y * m_pImg16u.nWidth + seed.x];

	//	//status = ippsFindC_16u(ToFind, k, valFind, &nPos1);

	//	//if (check_sts(status, "ippsFindC", hCall))
	//	//{
	//	//	return 0;
	//	//}

	//	//status = ippsRemove_16u_I(ToFind, &k, nPos1, 1);

	//	//if (check_sts(status, "ippsRemove", hCall))
	//	//{
	//	//	return 0;
	//	//}

	//	status = ippiFloodFill_8Con_16u_C1IR(m_pImg16u.buf, m_pImg16u.nPitch, Roi, seed, n++, &pRegion, m_BufferImgX.buf);

	//	if (check_sts(status, "ippiFloodFill_8Con", hCall))
	//	{
	//		return 0;
	//	}

	//	m_pBlobInfo[nBlob].m_nDots = (int)pRegion.area;
	//	m_pBlobInfo[nBlob].m_rc.setup(pRegion.rect.x, pRegion.rect.y, pRegion.rect.x + pRegion.rect.width - 1, pRegion.rect.y + pRegion.rect.height - 1);
	//	m_pBlobInfo[nBlob].m_nLabel = (int)pRegion.value[0];
	//	m_pBlobInfo[nBlob].m_PtSeed.x = seed.x;
	//	m_pBlobInfo[nBlob].m_PtSeed.y = seed.y;

	//	if (pRegion.area > m_nMinDots)
	//	{
	//		nIndex++;
	//	}

	//	nBlob++;
	//	k++;
	//	//nStart += nPos;
	//	//nSearchLen = nLen - nStart;

	//}
	//m_nCount = nCount;
	//m_nCountBlob = nIndex;


	//delete[]ToFind;

	return 1;

}

void CKxBlobAnalyse::ClearSortInfo()
{
	if (m_pSortBlobInfo)
	{
		delete[]m_pSortBlobInfo;
		m_pSortBlobInfo = NULL;
	}

}

int  CKxBlobAnalyse::SortByDots(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
	IppStatus  status;
	KxCallStatus hTmpCall;
	hCall.Clear();
	hTmpCall.Clear();

	nSortCount = gMin(nSortCount, m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pDots = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pDots[i] = m_pBlobInfo[i].m_nDots;
	}
	status = ippsSortIndexDescend_64f_I(pDots, pIndex, m_nCount);

	if (check_sts(status, "SortByDots_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[m_nCountBlob];
	status = ippsZero_8u((Ipp8u*)m_pSortBlobInfo, m_nCountBlob*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortByDots_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		//compute blob's general information
		IppiSize roiSize = { m_pSortBlobInfo[i].m_rc.Width(), m_pSortBlobInfo[i].m_rc.Height() };

		m_bOpenAdditionalInfo = nOpenComputeAdanceFeatures > 0 ? true : false;
		if (m_bOpenAdditionalInfo)
		{
			//compute some smallest rectangle's information
			IppiSize roi = { m_pImg16u.nWidth, m_pImg16u.nHeight };
			kxRect<int> rc;
			int nFacotorX, nFacotorY;
			if (m_bUseFoodFillAlogrithm)
			{
				rc = m_pSortBlobInfo[i].m_rc;
				nFacotorX = 1;
				nFacotorY = 1;
			}
			else
			{
				rc = m_pBlobArea[pIndex[i]];
				nFacotorX = m_nGridXStep;
				nFacotorY = m_nGridYStep;
			}

			Ipp16u newval = m_pSortBlobInfo[i].m_nLabel;
			IppiSize Roi = { rc.Width(), rc.Height() };
			status = ippiCopy_16u_C1R(m_pImg16u.buf + rc.top * m_pImg16u.nWidth + rc.left, m_pImg16u.nPitch,
				m_pTmpImg.buf, m_pTmpImg.nPitch, Roi);

			if (check_sts(status, "SortByDots_ippiCopy_first", hCall))
			{
				return 0;
			}

			status = ippiThreshold_LTValGTVal_16u_C1IR(m_pTmpImg.buf, m_pTmpImg.nPitch, Roi, newval, 0, newval, 0);

			if (check_sts(status, "SortByDots_ippiThreshold_LTValGTVal", hCall))
			{
				return 0;
			}

			//status = ippiCompareC_16u_C1R(m_pTmpImg.buf, m_pTmpImg.nPitch, newval-1, m_PreImg.buf, m_PreImg.nPitch,Roi, ippCmpGreater);
			status = ippiConvert_16u8u_C1R(m_pTmpImg.buf, m_pTmpImg.nPitch, m_PreImg.buf, m_PreImg.nPitch, Roi);
			if (check_sts(status, "SortByDots_ippiConvert_16u8u_C1R", hCall))
			{
				return 0;
			}


			if (m_bUseFoodFillAlogrithm)
			{
				status = ippiAnd_8u_C1IR(m_Img.buf + rc.top * m_Img.nPitch + rc.left, m_Img.nPitch, m_PreImg.buf, m_PreImg.nPitch, Roi);

				if (check_sts(status, "SortByDots_ippiAnd", hCall))
				{
					return 0;
				}

				Ipp64f pSum;
				status = ippiSum_8u_C1R(m_PreImg.buf, m_PreImg.nPitch, Roi, &pSum);
				if (check_sts(status, "SortByDots_ippiSum", hCall))
				{
					return 0;
				}

				m_pSortBlobInfo[i].m_nEnergy = int(pSum);
			}
			else
			{
				m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
			}

			double factorX, factorY;
			int nNormWidth, nNormHeight;
			if ((Roi.width + Roi.height > 200) && (Roi.height > 5) && (Roi.width > 5))
			{
				IppiSize srcSize = { Roi.width, Roi.height };
				IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };
				nNormWidth = gMin(int(_NORM_WIDTH), Roi.width);
				nNormHeight = gMin(int(_NORM_HEIGHT), Roi.height);
				IppiSize dstRoi = { nNormWidth, nNormHeight };
				factorX = dstRoi.width*1.0 / srcRoi.width;
				factorY = dstRoi.height*1.0 / srcRoi.height;

				//m_OpenImg.Init(nNormWidth, nNormHeight);
				m_SrcImg.SetImageBuf(m_PreImg.buf, srcSize.width, srcSize.height, m_PreImg.nPitch, m_PreImg.nChannel, false);
				m_DstImg.SetImageBuf(m_OpenImg.buf, nNormWidth, nNormHeight, m_OpenImg.nPitch, m_OpenImg.nChannel, false);
				m_hBaseFun.KxResizeImage(m_SrcImg, m_DstImg, KxLinear, hTmpCall);
				if (check_sts(hTmpCall, "KxResizeImage", hCall))
				{
					return 0;
				}

				//ippiResize_8u_C1R(m_PreImg.buf, srcSize, m_PreImg.nPitch, srcRoi, m_OpenImg.buf,
				//	m_OpenImg.nPitch, dstRoi, factorX, factorY, IPPI_INTER_LINEAR);
				//IppiRect dstRect = {0, 0, dstRoi.width, dstRoi.height};
				//int nBuffer;
				//status = ippiResizeGetBufSize(srcRoi, dstRect, 1, IPPI_INTER_LINEAR, &nBuffer);

				//if (check_sts(status, "SortByDots_ippiResizeGetBufSize", hCall))
				//{
				//	return 0;
				//}
				//
				//            Ipp8u* pBuffer = new Ipp8u[nBuffer];
				//status = ippiResizeSqrPixel_8u_C1R(m_PreImg.buf, srcSize, m_PreImg.nPitch, srcRoi, m_OpenImg.buf,
				//	m_OpenImg.nPitch, dstRect, factorX, factorY, 0, 0, IPPI_INTER_LINEAR, pBuffer);

				//if (check_sts(status, "SortByDots_ippiResizeSqrPixel", hCall))
				//{
				//	return 0;
				//}

				//delete []pBuffer;
			}
			else
			{
				nNormWidth = Roi.width;
				nNormHeight = Roi.height;
				IppiSize srcSize = { Roi.width, Roi.height };
				m_DstImg.SetImageBuf(m_OpenImg.buf, nNormWidth, nNormHeight, m_OpenImg.nPitch, m_OpenImg.nChannel, false);
				status = ippiCopy_8u_C1R(m_PreImg.buf, m_PreImg.nPitch, m_DstImg.buf, m_DstImg.nPitch, srcSize);

				if (check_sts(status, "SortByDots_ippiCopy_second", hCall))
				{
					return 0;
				}


				factorX = 1.0;
				factorY = 1.0;
			}


			m_hKxMinRect.Check(m_DstImg.buf, nNormWidth, nNormHeight, m_DstImg.nPitch);

			m_pSortBlobInfo[i].m_fAngle = m_hKxMinRect.GetResult().m_fAngle;
			m_pSortBlobInfo[i].m_nCircumference = int(m_hKxMinRect.GetResult().m_nCircumference*nFacotorX*nFacotorY*1.0 / (factorX*factorY));

			int nH = m_hKxMinRect.GetResult().m_nMinRectHeight;
			int nW = m_hKxMinRect.GetResult().m_nMinRectWidth;
			float fAngle = float(m_hKxMinRect.GetResult().m_fAngle * PI / 180);
			m_pSortBlobInfo[i].m_nMinRectHeight = int(sqrt((nH*sin(fAngle) / factorX)*(nH*sin(fAngle) / factorX) + (nH*cos(fAngle) / factorY)*(nH*cos(fAngle) / factorY)) + 0.5);
			m_pSortBlobInfo[i].m_nMinRectWidth = int(sqrt((nW*sin(fAngle) / factorY)*(nW*sin(fAngle) / factorY) + (nW*cos(fAngle) / factorX)*(nW*cos(fAngle) / factorX)) + 0.5);
			m_pSortBlobInfo[i].m_fMinArea = m_pSortBlobInfo[i].m_nMinRectHeight*m_pSortBlobInfo[i].m_nMinRectWidth;


			m_pSortBlobInfo[i].m_fRatio = float(gMax(m_pSortBlobInfo[i].m_nMinRectWidth, m_pSortBlobInfo[i].m_nMinRectHeight) / gMin(m_pSortBlobInfo[i].m_nMinRectWidth, m_pSortBlobInfo[i].m_nMinRectHeight));
			for (int k = 0; k <4; k++)
			{
				m_pSortBlobInfo[i].m_Pt[k].x = int((m_hKxMinRect.GetResult().m_Pt[k].x / factorX + rc.left)*nFacotorX*1.0);
				m_pSortBlobInfo[i].m_Pt[k].y = int((m_hKxMinRect.GetResult().m_Pt[k].y / factorY + rc.top)*nFacotorY*1.0);
			}
		}
	}


	delete[]pDots;
	delete[]pIndex;

	return 1;

}

void CKxBlobAnalyse::SetGridXY(int nGridX, int nGridY)
{
	assert(nGridX > 0 && nGridY > 0);

	if (m_pGrid)
		delete m_pGrid;
	m_nGridX = nGridX;
	m_nGridY = nGridY;
	m_bOpenGrid = true;
	m_pGrid = new CGridD[m_nGridX * m_nGridY];
}

void CKxBlobAnalyse::InitGrid(const kxRect<int>& rc)
{
	int  nX = rc.Width() / m_nGridX;
	int  nY = rc.Height() / m_nGridY;
	assert(nX > 0 && nY > 0);
	int  nXL = rc.Width() % m_nGridX;
	int  nYL = rc.Height() % m_nGridY;
	for (int i = 0; i < m_nGridX; i++)
	{
		m_pGrid[i].m_rc.top = rc.top;
		if (i == 0)
			m_pGrid[i].m_rc.left = rc.left;
		else
			m_pGrid[i].m_rc.left = m_pGrid[i - 1].m_rc.right + 1;
		m_pGrid[i].m_rc.right = m_pGrid[i].m_rc.left + nX - 1;
		if (nXL)
		{
			m_pGrid[i].m_rc.right++;
			nXL--;
		}
		m_pGrid[i].m_rc.bottom = m_pGrid[i].m_rc.top + nY - 1;
	}
	for (int y = 1; y < m_nGridY; y++)
	{
		int  kL = 0;
		if (nYL)
		{
			kL = 1;
			nYL--;
		}
		for (int x = 0; x < m_nGridX; x++)
		{
			m_pGrid[y*m_nGridX + x].m_rc.left = m_pGrid[(y - 1)*m_nGridX + x].m_rc.left;
			m_pGrid[y*m_nGridX + x].m_rc.right = m_pGrid[(y - 1)*m_nGridX + x].m_rc.right;
			m_pGrid[y*m_nGridX + x].m_rc.top = m_pGrid[(y - 1)*m_nGridX + x].m_rc.bottom + 1;
			m_pGrid[y*m_nGridX + x].m_rc.bottom = m_pGrid[y*m_nGridX + x].m_rc.top + nY - 1 + kL;
		}
	}
}

int  CKxBlobAnalyse::ToGridBlob(const unsigned char* buf, int nPitch, const kxRect<int>& rcBlob, KxCallStatus& hCall)
{
	IppStatus  status;
	//KxCallStatus hCall;
	hCall.Clear();

	if (m_rcCheck.top != rcBlob.top || m_rcCheck.bottom != rcBlob.bottom
		|| m_rcCheck.left != rcBlob.left || m_rcCheck.right != rcBlob.right)
	{
		InitGrid(rcBlob);
	}
	//-----------------------------------------------------------------------------------------
	for (int i = 0; i < (m_nGridX*m_nGridY); i++)
	{
		m_pGrid[i].m_nDots = 0;
		m_pGrid[i].m_nPower = 0;
		for (int y = m_pGrid[i].m_rc.top; y <= m_pGrid[i].m_rc.bottom; y++)
		{
			for (int x = m_pGrid[i].m_rc.left; x <= m_pGrid[i].m_rc.right; x++)
			{
				int  nOff = y * nPitch + x;
				if (buf[nOff])
				{
					m_pGrid[i].m_nDots++;
					m_pGrid[i].m_nPower += buf[nOff];
				}
			}
		}
	}
	//-------------------------------------------------------------------------------------------
	IppiSize   roiSize;
	roiSize.height = m_nGridY;
	roiSize.width = m_nGridX;

	m_pImg16u.Init(m_nGridX, m_nGridY);

	for (int y = 0; y < m_nGridY; y++)
	{
		for (int x = 0; x < m_nGridX; x++)
		{
			if (m_pGrid[y*m_nGridX + x].m_nDots < 1)
			{
				m_pImg16u.buf[y*roiSize.width + x] = 0;
				m_pGrid[y*m_nGridX + x].m_nDots = 0;
				m_pGrid[y*m_nGridX + x].m_nPower = 0;
			}
			else
			{
				m_pImg16u.buf[y*roiSize.width + x] = 0xFF;
			}

		}
	}

	Clear();

	//first,label the connections components
	int nBufferSize;
	status = ippiLabelMarkersGetBufferSize_16u_C1R(roiSize, &nBufferSize);

	if (check_sts(status, "ippiLabelMarkersGetBufferSize", hCall))
	{
		return 0;
	}

	Ipp8u* pBuffer;
	pBuffer = new Ipp8u[nBufferSize];
	status = ippiLabelMarkers_16u_C1IR(m_pImg16u.buf, m_pImg16u.nPitch, roiSize, _Min_Lable, _Min_Lable + _MAX_Lable_Count,
		(m_nConnectType == _USE8 ? ippiNormInf : ippiNormL1), &m_nCount, pBuffer);

	if (check_sts(status, "ippiLabelMarkers_16u", hCall))
	{
		return 0;
	}

	delete[]pBuffer;

	if (m_nCount == 0)
	{
		return 1;
	}
	m_pBlobInfo = new SingleBlobInfo[m_nCount];
	status = ippsZero_8u((Ipp8u*)m_pBlobInfo, m_nCount*sizeof(SingleBlobInfo));

	if (check_sts(status, "ippsZero_8u", hCall))
	{
		return 0;
	}


	m_pBlobArea = new kxRect<int>[m_nCount];

	for (int i = 0; i < m_nCount; i++)
	{
		m_pBlobArea[i].setup(INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);
	}

	for (int y = 0; y < m_pImg16u.nHeight; y++)
	{
		for (int x = 0; x < m_pImg16u.nWidth; x++)
		{
			int  n = (int)m_pImg16u.buf[y*m_pImg16u.nWidth + x] - _Min_Lable;
			if (n >= 0)
			{
				m_pBlobInfo[n].m_nLabel = (int)m_pImg16u.buf[y*m_pImg16u.nWidth + x];
				m_pBlobInfo[n].m_PtSeed.setup(x, y);
				m_pBlobInfo[n].m_nDots += m_pGrid[y*m_nGridX + x].m_nDots;
				m_pBlobInfo[n].m_nEnergy += m_pGrid[y*m_nGridX + x].m_nPower;
				m_pBlobArea[n].left = gMin(m_pBlobArea[n].left, x);
				m_pBlobArea[n].top = gMin(m_pBlobArea[n].top, y);
				m_pBlobArea[n].right = gMax(m_pBlobArea[n].right, x);
				m_pBlobArea[n].bottom = gMax(m_pBlobArea[n].bottom, y);
			}

		}
	}

	int nIndex = 0;
	for (int i = 0; i < m_nCount; i++)
	{
		int nLeft = m_pGrid[m_pBlobArea[i].top*m_nGridX + m_pBlobArea[i].left].m_rc.left;
		int nTop = m_pGrid[m_pBlobArea[i].top*m_nGridX + m_pBlobArea[i].left].m_rc.top;
		int nRight = m_pGrid[m_pBlobArea[i].bottom*m_nGridX + m_pBlobArea[i].right].m_rc.right;
		int nBottom = m_pGrid[m_pBlobArea[i].bottom*m_nGridX + m_pBlobArea[i].right].m_rc.bottom;
		m_pBlobInfo[i].m_rc.setup(nLeft, nTop, nRight, nBottom);

		if (m_pBlobInfo[i].m_nDots > m_nMinDots)
		{
			nIndex++;
		}

	}

	m_nCountBlob = nIndex;

	return 1;
}

int CKxBlobAnalyse::ToBlobParallel(const kxCImageBuf& SrcImg, int nSortByMode, int nMaxSortDots, int nMergeSize, int nOpenComputeAdanceFeatures)
{
	KxCallStatus hCall;
	return ToBlobParallel(SrcImg, nSortByMode, nMaxSortDots, nMergeSize, nOpenComputeAdanceFeatures, hCall);
}

int CKxBlobAnalyse::ToBlobParallel(const kxCImageBuf& SrcImg, int nSortByMode, int nMaxSortDots, int nMergeSize, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
	Clear(); //clear the data struct
	hCall.Clear();
	IppStatus status;
	m_nOpenSize = nMergeSize;
	IppiSize Roi = { SrcImg.nWidth, SrcImg.nHeight };
	if (m_BufferImg.buf == NULL || Roi.width != m_Img.nWidth || Roi.height != m_Img.nHeight)
	{
		int  nBufferSize;
		status = ippiLabelMarkersGetBufferSize_16u_C1R(Roi, &nBufferSize);

		if (check_sts(status, "ToBlobParallel_ippiLabelMarkersGetBufferSize", hCall))
		{
			return 0;
		}

		m_BufferImg.Init(nBufferSize, 1);
	}
	//copy a Img
	m_Img.Init(Roi.width, Roi.height);
	status = ippiCopy_8u_C1R(SrcImg.buf, SrcImg.nPitch, m_Img.buf, m_Img.nPitch, Roi);//HYH 这张图，在归一x、y的时候可能当做能量图像

	if (check_sts(status, "ToBlobParallel_ippiCopy", hCall))
	{
		return 0;
	}

	//merge some connections
	m_PreImg.Init(Roi.width, Roi.height);
	KxCallStatus hTempCall;
	hTempCall.Clear();
	MergeSomeConnections(SrcImg.buf, Roi.width, Roi.height, SrcImg.nPitch, m_PreImg.buf, m_PreImg.nPitch, hTempCall);

	if (check_sts(hTempCall, "ToBlobParallel_MergeSomeConnections", hCall))
	{
		return 0;
	}


	m_pTmpImg.Init(Roi.width, Roi.height);
	m_pImg16u.Init(Roi.width, Roi.height);

	status = ippiConvert_8u16u_C1R(m_PreImg.buf, m_PreImg.nPitch, m_pTmpImg.buf, m_pTmpImg.nPitch, Roi);

	if (check_sts(status, "ToBlobParallel_ippiConvert", hCall))
	{
		return 0;
	}

	//first,label the connections components
	int nCount = 0;
	status = ippiLabelMarkers_16u_C1IR(m_pTmpImg.buf, m_pTmpImg.nPitch, Roi, _Min_Lable, _Min_Lable + _MAX_Lable_Count,
		(m_nConnectType == _USE8 ? ippiNormInf : ippiNormL1), &nCount, m_BufferImg.buf);

	if (check_sts(status, "ToBlobParallel_ippiLabelMarkers", hCall))
	{
		return 0;
	}

	if (nCount == 0)
	{
		return 1;
	}
	if (nCount > _MAX_Lable_Count)
	{
		return -1;
	}

	m_nCount = nCount;


	/*!————————————
	源码丢失，尝试修复
	add in 2019.10.24 by hyh、csq
	*/

	makeComponents();

	m_nCountBlob = m_nCount;

	//————————————//


	//sort by dots or energy or size
	switch (nSortByMode)
	{
	case _SORT_BYDOTS:
	{
						 SortByDot(nMaxSortDots, nOpenComputeAdanceFeatures, hTempCall);
						 break;
	}
	case _SORT_BYENERGY:
	{
						   SortByEnergy(nMaxSortDots, nOpenComputeAdanceFeatures, hTempCall);
						   break;
	}
	case _SORT_BYSIZE:
	{
						 SortBySize(nMaxSortDots, nOpenComputeAdanceFeatures, hTempCall);
						 break;
	}
	case _SORT_BYWIDTH:
	{
						  SortByWidth(nMaxSortDots, nOpenComputeAdanceFeatures);
						  break;
	}
	case _SORT_BYHEIGHT:
	{
						   SortByHeight(nMaxSortDots, nOpenComputeAdanceFeatures);
						   break;
	}


	default:
		break;
	}

	if (check_sts(hTempCall, "ToBlobParallel_", hCall))
	{
		return 0;
	}

	return 1;


}

void CKxBlobAnalyse::GetBlobImage(int nLabel, kxRect<int> rc, kxCImageBuf& blobimg)
{
	if (m_pTmpImg.nWidth == 0 || m_pTmpImg.nHeight == 0)
		return;

	Ipp16u newval = nLabel;
	IppiSize Roi = { rc.Width(), rc.Height() };
	m_ImgFilter16u.Init(Roi.width, Roi.height);
	kxCImageBuf	cutimg;
	cutimg.Init(Roi.width, Roi.height);
	blobimg.Init(Roi.width, Roi.height);
	ippiCopy_16u_C1R(m_pTmpImg.buf + rc.top * m_pTmpImg.nWidth + rc.left, m_pTmpImg.nPitch,
		m_ImgFilter16u.buf, m_ImgFilter16u.nPitch, Roi);
	ippiThreshold_LTValGTVal_16u_C1IR(m_ImgFilter16u.buf, m_ImgFilter16u.nPitch, Roi, newval, 0, newval, 0);
	ippiConvert_16u8u_C1R(m_ImgFilter16u.buf, m_ImgFilter16u.nPitch, cutimg.buf, cutimg.nPitch, Roi);

	ippiCompareC_8u_C1R(cutimg.buf, cutimg.nPitch, 0, blobimg.buf, blobimg.nPitch, Roi, ippCmpGreater);
}

int CKxBlobAnalyse::SelectMaxRegionByDots(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	return SelectMaxRegionByDots(SrcImg, DstImg, hCall);
}

int CKxBlobAnalyse::SelectMaxRegionByDots(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	
	/*算法库缺失，尝试修复*/
	ToBlobByCV(SrcImg, _SORT_BYDOTS, 1, 1);
	//ToBlobParallel(SrcImg, _SORT_BYDOTS, 1, 1, 0);
	SingleBlobInfo maxblob = GetSortSingleBlob(0);
	kxRect<int> rc;
	rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
	GetBlobImage(maxblob.m_nLabel, rc, DstImg);

	return 1;
}

int  CKxBlobAnalyse::SortByEnergy(int nSortCount, int nOpenComputeAdanceFeatures)
{
	KxCallStatus hCall;
	return SortByEnergy(m_nCount, nOpenComputeAdanceFeatures, hCall);
}

int  CKxBlobAnalyse::SortByEnergy(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
	IppStatus  status;
	KxCallStatus hTmpCall;
	hCall.Clear();
	hTmpCall.Clear();

	nSortCount = gMin(nSortCount, m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pEnergys = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pEnergys[i] = m_pBlobInfo[i].m_nEnergy;
	}
	status = ippsSortIndexDescend_64f_I(pEnergys, pIndex, m_nCount);

	if (check_sts(status, "SortByEnergy_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[m_nCountBlob];
	status = ippsZero_8u((Ipp8u*)m_pSortBlobInfo, m_nCountBlob*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortByEnergy_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		m_pSortBlobInfo[i].m_nSize = m_pBlobInfo[pIndex[i]].m_nSize;
		m_pSortBlobInfo[i].m_fSumEnergy = m_nEnergySum;
		m_pSortBlobInfo[i].m_PtSeed = m_pBlobInfo[pIndex[i]].m_PtSeed;//HYH 2020.03.16 

		//最小外界矩形特征
		if (nOpenComputeAdanceFeatures)
		{
			ComputeBlobMinRectangle(m_pSortBlobInfo[i]);
		}

	}


	delete[]pEnergys;
	delete[]pIndex;

	return 1;

}

int  CKxBlobAnalyse::SortBySize(int nSortCount, int nOpenComputeAdanceFeatures)
{
	KxCallStatus hCall;
	return SortBySize(m_nCount, nOpenComputeAdanceFeatures, hCall);
}

int  CKxBlobAnalyse::SortByDot(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
	IppStatus  status;
	KxCallStatus hTmpCall;
	hCall.Clear();
	hTmpCall.Clear();

	nSortCount = gMin(nSortCount, m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pDots = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pDots[i] = m_pBlobInfo[i].m_nDots;
	}
	status = ippsSortIndexDescend_64f_I(pDots, pIndex, m_nCount);

	if (check_sts(status, "SortByDot_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[m_nCountBlob];
	status = ippsZero_8u((Ipp8u*)m_pSortBlobInfo, m_nCountBlob*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortByDot_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		m_pSortBlobInfo[i].m_nSize = m_pBlobInfo[pIndex[i]].m_nSize;
		m_pSortBlobInfo[i].m_fSumEnergy = m_nEnergySum;
		m_pSortBlobInfo[i].m_PtSeed = m_pBlobInfo[pIndex[i]].m_PtSeed;//HYH 2020.03.16 

		//最小外界矩形特征
		if (nOpenComputeAdanceFeatures)
		{
			ComputeBlobMinRectangle(m_pSortBlobInfo[i]);
		}

	}


	delete[]pDots;
	delete[]pIndex;

	return 1;

}

int CKxBlobAnalyse::ComputeBlobMinRectangle(SingleBlobInfo& hSortBlobInfo)
{
	KxCallStatus hCall;
	return ComputeBlobMinRectangle(hSortBlobInfo, hCall);
}

int CKxBlobAnalyse::ComputeBlobMinRectangle(SingleBlobInfo& hSortBlobInfo, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	Ipp16u newval = hSortBlobInfo.m_nLabel;
	IppiSize Roi = { hSortBlobInfo.m_rc.Width(), hSortBlobInfo.m_rc.Height() };

	IppStatus status = ippiCopy_16u_C1R(m_pTmpImg.buf + hSortBlobInfo.m_rc.top * m_pTmpImg.nWidth + hSortBlobInfo.m_rc.left, m_pTmpImg.nPitch,
		m_pImg16u.buf, m_pImg16u.nPitch, Roi);

	if (check_sts(status, "ComputeBlobMinRectangle_ippiCopy_first", hCall))
	{
		return 0;
	}

	status = ippiThreshold_LTValGTVal_16u_C1IR(m_pImg16u.buf, m_pImg16u.nPitch, Roi, newval, 0, newval, 0);

	if (check_sts(status, "ComputeBlobMinRectangle_ippiThreshold_LTValGTVal", hCall))
	{
		return 0;
	}

	status = ippiConvert_16u8u_C1R(m_pImg16u.buf, m_pImg16u.nPitch, m_PreImg.buf, m_PreImg.nPitch, Roi);


	m_hKxMinRect.Check(m_PreImg.buf, Roi.width, Roi.height, m_PreImg.nPitch);


	hSortBlobInfo.m_nMinRectHeight = m_hKxMinRect.GetResult().m_nMinRectHeight;
	hSortBlobInfo.m_nMinRectWidth = m_hKxMinRect.GetResult().m_nMinRectWidth;

	hSortBlobInfo.m_fRatio = m_hKxMinRect.GetResult().m_fRatio;

	return 1;
}

void CKxBlobAnalyse::makeComponents()
{

	KxMakeConnectedComponents makeObj(m_pTmpImg.buf, m_Img.buf, m_pTmpImg.nWidth, m_nCount);
	tbb::parallel_reduce(tbb::blocked_range<size_t>(0, m_pTmpImg.nHeight), makeObj);
	m_pBlobInfo = makeObj.m_pComponents;

	//cv::Mat src(m_pTmpImg.nHeight, m_pTmpImg.nWidth, CV_16UC1, m_pTmpImg.buf);

	//for (int i = 0; i < m_nCount; i++)
	//{
	//	SingleBlobInfo blob = m_pBlobInfo[i];
	//	if (blob.m_nDots == 618261)
	//	{
	//		std::cout << blob.m_nLabel << std::endl;
	//	}
	//}
	//int nindex = 4;
	//kxRect<int> re = m_pBlobInfo[nindex].m_rc;
	//std::cout << nindex << "  " << m_pBlobInfo[nindex].m_nLabel << "  " << m_pBlobInfo[nindex].m_nDots << "  " << re.left << "  " << re.top << "   " << re.right << "   " << re.bottom << std::endl;

	//MakeConnectedComponents makeObj(m_pTmpImg.buf, m_pTmpImg.nWidth, m_nCount);
	//tbb::parallel_reduce(tbb::blocked_range<size_t>(0, m_pTmpImg.nHeight), makeObj);
}

int CKxBlobAnalyse::SelectRegion(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, std::string szType, int nMinRange, int nMaxRange)
{
	KxCallStatus hCall;
	return SelectRegion(SrcImg, DstImg, szType, nMinRange, nMaxRange, hCall);
}

int CKxBlobAnalyse::SelectRegion(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, std::string szType, int nMinRange, int nMaxRange, KxCallStatus& hCall)
{
	/*
	Clear(); //clear the data struct

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	IppiSize Roi = { SrcImg.nWidth, SrcImg.nHeight };
	ippiCopy_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, Roi);

	hCall.Clear();
	IppStatus status;
	m_nOpenSize = 1;

	if (m_BufferImg.buf == NULL || Roi.width != m_Img.nWidth || Roi.height != m_Img.nHeight)
	{
	int  nBufferSize;
	status = ippiLabelMarkersGetBufferSize_16u_C1R(Roi, &nBufferSize);

	if (check_sts(status, "SelectMaxRegionByDots_ippiLabelMarkersGetBufferSize", hCall))
	{
	return 0;
	}

	m_BufferImg.Init(nBufferSize, 1);
	}
	//copy a Img
	m_Img.Init(Roi.width, Roi.height);
	status = ippiCopy_8u_C1R(SrcImg.buf, SrcImg.nPitch, m_Img.buf, m_Img.nPitch, Roi);

	if (check_sts(status, "SelectMaxRegionByDots_ippiCopy", hCall))
	{
	return 0;
	}

	//merge some connections
	m_PreImg.Init(Roi.width, Roi.height);
	KxCallStatus hTempCall;
	hTempCall.Clear();
	m_nOpenSize = 1;
	MergeSomeConnections(SrcImg.buf, Roi.width, Roi.height, SrcImg.nPitch, m_PreImg.buf, m_PreImg.nPitch, hTempCall);

	if (check_sts(hTempCall, "SelectMaxRegionByDots_MergeSomeConnections", hCall))
	{
	return 0;
	}


	m_pTmpImg.Init(Roi.width, Roi.height);
	m_pImg16u.Init(Roi.width, Roi.height);

	status = ippiConvert_8u16u_C1R(m_PreImg.buf, m_PreImg.nPitch, m_pTmpImg.buf, m_pTmpImg.nPitch, Roi);

	if (check_sts(status, "SelectMaxRegionByDots_ippiConvert", hCall))
	{
	return 0;
	}

	//first,label the connections components
	int nCount = 0;
	status = ippiLabelMarkers_16u_C1IR(m_pTmpImg.buf, m_pTmpImg.nPitch, Roi, _Min_Lable, _Min_Lable + _MAX_Lable_Count,
	(m_nConnectType == _USE8 ? ippiNormInf : ippiNormL1), &nCount, m_BufferImg.buf);

	if (check_sts(status, "SelectMaxRegionByDots_ippiLabelMarkers", hCall))
	{
	return 0;
	}

	if (nCount == 0)
	{
	return 0;
	}

	m_nCount = nCount;
	//m_pBlobInfo = new SingleBlobInfo[nCount];
	*/

	// —— 2020.03.16 代码缺失，尝试补全 ——//
	int nminindex = 0;
	int nmaxindex = m_nCountBlob;

	if (szType == "Dots")
	{
		ToBlobParallel(SrcImg, _SORT_BYDOTS, _MAX_Lable_Count, 1, 0);//先进行全部的整理

		// blob是默认从大到小排序
		for (int i = m_nCountBlob - 1; i >= 0; i--)
		{
			if (m_pSortBlobInfo[i].m_nDots >= nMinRange)
			{
				nminindex = i;
				break;
			}
		}


		for (int i = 0; i < nminindex; i++)
		{
			if (m_pSortBlobInfo[i].m_nDots <= nMaxRange)
			{
				nmaxindex = i;
				break;
			}
		}
	}
	else if (szType == "Energy")
	{
		ToBlobParallel(SrcImg, _SORT_BYENERGY, _MAX_Lable_Count, 1, 0);//先进行全部的整理

		// blob是默认从大到小排序

		for (int i = m_nCountBlob - 1; i >= 0; i--)
		{
			if (m_pSortBlobInfo[i].m_nEnergy >= nMinRange)
			{
				nminindex = i;
				break;
			}
		}


		for (int i = 0; i < nminindex; i++)
		{
			if (m_pSortBlobInfo[i].m_nEnergy <= nMaxRange)
			{
				nmaxindex = i;
				break;
			}
		}
	}
	else if (szType == "Size")
	{
		ToBlobParallel(SrcImg, _SORT_BYSIZE, _MAX_Lable_Count, 1, 0);//先进行全部的整理

		// blob是默认从大到小排序

		for (int i = m_nCountBlob - 1; i >= 0; i--)
		{
			if (m_pSortBlobInfo[i].m_nSize >= nMinRange)
			{
				nminindex = i;
				break;
			}
		}


		for (int i = 0; i < nminindex; i++)
		{
			if (m_pSortBlobInfo[i].m_nSize <= nMaxRange)
			{
				nmaxindex = i;
				break;
			}
		}
	}

	else if (szType == "Width")
	{
		ToBlobParallel(SrcImg, _SORT_BYWIDTH, _MAX_Lable_Count, 1, 0);//先进行全部的整理

		// blob是默认从大到小排序

		for (int i = m_nCountBlob - 1; i >= 0; i--)
		{
			if (m_pSortBlobInfo[i].m_rc.Width() >= nMinRange)
			{
				nminindex = i;
				break;
			}
		}


		for (int i = 0; i < nminindex; i++)
		{
			if (m_pSortBlobInfo[i].m_rc.Width() <= nMaxRange)
			{
				nmaxindex = i;
				break;
			}
		}
	}
	else if (szType == "Height")
	{
		ToBlobParallel(SrcImg, _SORT_BYHEIGHT, _MAX_Lable_Count, 1, 0);//先进行全部的整理

		// blob是默认从大到小排序

		for (int i = m_nCountBlob - 1; i >= 0; i--)
		{
			if (m_pSortBlobInfo[i].m_rc.Height() >= nMinRange)
			{
				nminindex = i;
				break;
			}
		}


		for (int i = 0; i < nminindex; i++)
		{
			if (m_pSortBlobInfo[i].m_rc.Height() <= nMaxRange)
			{
				nmaxindex = i;
				break;
			}
		}
	}
	else
	{
		return 0;
	}

	IppiSize Roi = { SrcImg.nWidth, SrcImg.nHeight };

	m_ImgTempcopy.Init(m_pTmpImg.nWidth, m_pTmpImg.nHeight);
	ippiCopy_16u_C1R(m_pTmpImg.buf, m_pTmpImg.nPitch, m_ImgTempcopy.buf, m_ImgTempcopy.nPitch, Roi);
	int targetvalue = 255;
	for (int i = nmaxindex; i < nminindex + 1; i++)
	{
		int nBufferSize;
		ippiFloodFillGetSize(Roi, &nBufferSize);
		m_ImgRegionbuf.Init(nBufferSize, 1);
		IppiPoint seed = { m_pSortBlobInfo[i].m_PtSeed.x, m_pSortBlobInfo[i].m_PtSeed.y };
		IppiConnectedComp pRegion;
		ippiFloodFill_8Con_16u_C1IR(m_ImgTempcopy.buf, m_ImgTempcopy.nPitch, Roi, seed, targetvalue, &pRegion, m_ImgRegionbuf.buf);
	}
	kxRect<int> rc;
	rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
	GetBlobImageWithImg(m_ImgTempcopy, targetvalue, rc, DstImg);

	return 1;
}

void CKxBlobAnalyse::GetBlobImageWithImg(kxImg16u& solveimg, int nLabel, kxRect<int> rc, kxCImageBuf& blobimg)
{
	if (solveimg.nWidth == 0 || solveimg.nHeight == 0)
		return;

	Ipp16u newval = nLabel;
	IppiSize Roi = { rc.Width(), rc.Height() };
	m_ImgFilter16u.Init(Roi.width, Roi.height);
	blobimg.Init(Roi.width, Roi.height);
	ippiCopy_16u_C1R(solveimg.buf + rc.top * solveimg.nWidth + rc.left, solveimg.nPitch,
		m_ImgFilter16u.buf, m_ImgFilter16u.nPitch, Roi);
	ippiThreshold_LTValGTVal_16u_C1IR(m_ImgFilter16u.buf, m_ImgFilter16u.nPitch, Roi, newval, 0, newval, 0);
	ippiConvert_16u8u_C1R(m_ImgFilter16u.buf, m_ImgFilter16u.nPitch, blobimg.buf, blobimg.nPitch, Roi);

}

int CKxBlobAnalyse::SortBySize(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
	/*!
	2020.03.16 这里是后期修复代码添加的，对于m_nCount、m_nCountBlob这两个值是否一样没必要太纠结，因为
	当格子状blob的时候这两个值不一样，但格子blob被淘汰，所以这两个值也是被保留

	原先它用能量排序，现在改成用size
	*/
	IppStatus  status;
	KxCallStatus hTmpCall;
	hCall.Clear();
	hTmpCall.Clear();

	nSortCount = gMin(nSortCount, m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pSize = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pSize[i] = m_pBlobInfo[i].m_rc.Width() * m_pBlobInfo[i].m_rc.Height();
	}
	status = ippsSortIndexDescend_64f_I(pSize, pIndex, m_nCount);

	if (check_sts(status, "SortBySize_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[m_nCountBlob];
	status = ippsZero_8u((Ipp8u*)m_pSortBlobInfo, m_nCountBlob*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortBySize_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		m_pSortBlobInfo[i].m_nSize = m_pBlobInfo[pIndex[i]].m_nSize;
		m_pSortBlobInfo[i].m_fSumEnergy = m_nEnergySum;
		m_pSortBlobInfo[i].m_PtSeed = m_pBlobInfo[pIndex[i]].m_PtSeed;//HYH 2020.03.16 

		//最小外界矩形特征
		if (nOpenComputeAdanceFeatures)
		{
			ComputeBlobMinRectangle(m_pSortBlobInfo[i]);
		}

	}


	delete[]pSize;
	delete[]pIndex;

	return 1;

}

int CKxBlobAnalyse::SortByWidth(int nSortCount, int nOpenComputeAdanceFeatures)
{
	/*!
	2020.03.16 这里是后期修复代码添加的，对于m_nCount、m_nCountBlob这两个值是否一样没必要太纠结，因为
	当格子状blob的时候这两个值不一样，但格子blob被淘汰，所以这两个值也是被保留

	*/

	KxCallStatus hCall;
	IppStatus  status;
	KxCallStatus hTmpCall;

	hTmpCall.Clear();

	nSortCount = gMin(nSortCount, m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pWidth = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pWidth[i] = m_pBlobInfo[i].m_rc.Width();
	}
	status = ippsSortIndexDescend_64f_I(pWidth, pIndex, m_nCount);

	if (check_sts(status, "SortBySize_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[nSortCount];
	status = ippsZero_8u((Ipp8u*)m_pSortBlobInfo, nSortCount*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortBySize_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		m_pSortBlobInfo[i].m_nSize = m_pBlobInfo[pIndex[i]].m_nSize;
		m_pSortBlobInfo[i].m_fSumEnergy = m_nEnergySum;
		m_pSortBlobInfo[i].m_PtSeed = m_pBlobInfo[pIndex[i]].m_PtSeed;//HYH 2020.03.16 

		//最小外界矩形特征
		if (nOpenComputeAdanceFeatures)
		{
			ComputeBlobMinRectangle(m_pSortBlobInfo[i]);
		}

	}


	delete[]pWidth;
	delete[]pIndex;

	return 1;
}

int CKxBlobAnalyse::SortByHeight(int nSortCount, int nOpenComputeAdanceFeatures)
{
	/*!
	2020.03.16 这里是后期修复代码添加的，对于m_nCount、m_nCountBlob这两个值是否一样没必要太纠结，因为
	当格子状blob的时候这两个值不一样，但格子blob被淘汰，所以这两个值也是被保留

	*/

	KxCallStatus hCall;
	IppStatus  status;
	KxCallStatus hTmpCall;

	hTmpCall.Clear();

	nSortCount = gMin(nSortCount, m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pHeight = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pHeight[i] = m_pBlobInfo[i].m_rc.Height();
	}
	status = ippsSortIndexDescend_64f_I(pHeight, pIndex, m_nCount);

	if (check_sts(status, "SortBySize_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[nSortCount];
	status = ippsZero_8u((Ipp8u*)m_pSortBlobInfo, nSortCount*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortBySize_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		m_pSortBlobInfo[i].m_nSize = m_pBlobInfo[pIndex[i]].m_nSize;
		m_pSortBlobInfo[i].m_fSumEnergy = m_nEnergySum;
		m_pSortBlobInfo[i].m_PtSeed = m_pBlobInfo[pIndex[i]].m_PtSeed;//HYH 2020.03.16 

		//最小外界矩形特征
		if (nOpenComputeAdanceFeatures)
		{
			ComputeBlobMinRectangle(m_pSortBlobInfo[i]);
		}

	}


	delete[]pHeight;
	delete[]pIndex;

	return 1;
}

int CKxBlobAnalyse::ToBlobByCV(const kxCImageBuf& SrcImg, int nSortByMode, int nMaxSortNums, int nMergeSize, int connectivity)
{
	Clear(); //clear the data struct
	IppStatus status;
	m_nOpenSize = nMergeSize;
	IppiSize Roi = { SrcImg.nWidth, SrcImg.nHeight };

	//copy a Img
	m_Img.Init(Roi.width, Roi.height);
	status = ippiCopy_8u_C1R(SrcImg.buf, SrcImg.nPitch, m_Img.buf, m_Img.nPitch, Roi);//HYH 这张图，在归一x、y的时候可能当做能量图像


	//merge some connections
	m_PreImg.Init(Roi.width, Roi.height);
	KxCallStatus hTempCall;
	hTempCall.Clear();
	MergeSomeConnections(SrcImg.buf, Roi.width, Roi.height, SrcImg.nPitch, m_PreImg.buf, m_PreImg.nPitch, hTempCall);// 开闭运算



	m_pTmpImg.Init(Roi.width, Roi.height);
	//m_pImg16u.Init(Roi.width, Roi.height);

	//status = ippiConvert_8u16u_C1R(m_PreImg.buf, m_PreImg.nPitch, m_pTmpImg.buf, m_pTmpImg.nPitch, Roi);

	m_MatBlobImg = cv::Mat(m_PreImg.nHeight, m_PreImg.nWidth, CV_8UC1, m_PreImg.buf, m_PreImg.nPitch);
	m_MatLabel = cv::Mat(m_pTmpImg.nHeight, m_pTmpImg.nWidth, CV_16UC1, m_pTmpImg.buf, m_pTmpImg.nPitch);
	cv::connectedComponentsWithStats(m_MatBlobImg, m_MatLabel, m_MatState, m_MatCentroids, connectivity, CV_16U);
	
	

	m_nCount = gMax(0, m_MatState.rows - 1);


	if (m_nCount == 0)
	{
		return 1;
	}
	if (m_nCount > _MAX_Lable_Count)
	{
		return -1;
	}

	m_MatState.at<int>(0, 4) = 0;

	m_nCountBlob = m_nCount;

	ClearSortInfo();
	m_pSortBlobInfo = new SingleBlobInfo[m_nCountBlob];
	ippsZero_8u((Ipp8u*)m_pSortBlobInfo, m_nCountBlob * sizeof(SingleBlobInfo));

	int nmaxsortnum = gMin(nMaxSortNums, m_nCountBlob);

	for (int i = 0; i < nmaxsortnum; i++)
	{
		CKxBlobAnalyse::SingleBlobInfo pObj;
		int nmaxrow = GetMaxInfo2Obj(m_MatState, m_pSortBlobInfo[i]);
		m_MatState.at<int>(nmaxrow, 4) = 0;
		//memcpy(&m_pSortBlobInfo[i], &pObj, sizeof(CKxBlobAnalyse::SingleBlobInfo));
	}


	//————————————//


	//sort by dots or energy or size
	//switch (nSortByMode)
	//{
	//case _SORT_BYDOTS:
	//{
	//	SortByDot(nMaxSortDots, nOpenComputeAdanceFeatures, hTempCall);
	//	break;
	//}
	//case _SORT_BYENERGY:
	//{
	//	SortByEnergy(nMaxSortDots, nOpenComputeAdanceFeatures, hTempCall);
	//	break;
	//}
	//case _SORT_BYSIZE:
	//{
	//	SortBySize(nMaxSortDots, nOpenComputeAdanceFeatures, hTempCall);
	//	break;
	//}
	//case _SORT_BYWIDTH:
	//{
	//	SortByWidth(nMaxSortDots, nOpenComputeAdanceFeatures);
	//	break;
	//}
	//case _SORT_BYHEIGHT:
	//{
	//	SortByHeight(nMaxSortDots, nOpenComputeAdanceFeatures);
	//	break;
	//}
	//default:
	//	break;
	//}


	return 1;
}

int CKxBlobAnalyse::GetMaxInfo2Obj(cv::Mat& Info, SingleBlobInfo& Obj)
{
	cv::Mat dotscol = Info.col(4);
	double minval, maxval;
	cv::Point minpoint, maxpoint;
	//cv::minMaxIdx(dotscol,  minindex, maxindex);
	cv::minMaxLoc(dotscol, &minval, &maxval, &minpoint, &maxpoint);
	Obj.m_nDots = maxval;
	int x = Info.at<int>(maxpoint.y, 0);
	int y = Info.at<int>(maxpoint.y, 1);
	int width = Info.at<int>(maxpoint.y, 2);
	int height = Info.at<int>(maxpoint.y, 3);

	Obj.m_rc.setup(x, y, x + width - 1, y + height - 1);
	Obj.m_nLabel = maxpoint.y;
	return maxpoint.y;
}

void CKxBlobAnalyse::GetResidualImage(int nLabel, kxRect<int> rc, kxCImageBuf& residualimg)
{
	if (m_pTmpImg.nWidth == 0 || m_pTmpImg.nHeight == 0)
		return;

	Ipp16u newval = nLabel;
	IppiSize Roi = { rc.Width(), rc.Height() };
	m_ImgFilter16u.Init(Roi.width, Roi.height);
	residualimg.Init(Roi.width, Roi.height);
	ippiCopy_16u_C1R(m_pTmpImg.buf + rc.top * m_pTmpImg.nWidth + rc.left, m_pTmpImg.nPitch,
		m_ImgFilter16u.buf, m_ImgFilter16u.nPitch, Roi);
	ippiThreshold_LTValGTVal_16u_C1IR(m_ImgFilter16u.buf, m_ImgFilter16u.nPitch, Roi, newval, 0, newval, 0);

	kxCImageBuf cutimg;
	cutimg.Init(Roi.width, Roi.height);
	ippiConvert_16u8u_C1R(m_ImgFilter16u.buf, m_ImgFilter16u.nPitch, cutimg.buf, cutimg.nPitch, Roi);
	ippiCompareC_8u_C1R(cutimg.buf, cutimg.nPitch, 0, residualimg.buf, residualimg.nPitch, Roi, ippCmpGreater);
	ippiAnd_8u_C1IR(m_PreImg.buf + rc.top * m_pTmpImg.nWidth + rc.left, m_PreImg.nPitch, residualimg.buf, residualimg.nPitch, Roi);
}
