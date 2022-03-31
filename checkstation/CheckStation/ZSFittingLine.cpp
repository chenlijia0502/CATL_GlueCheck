#include "stdafx.h"
#include "ZSFittingLine.h"


//y = ax + b
bool CZSFittingLine::FiltLine(Ipp32f* pX, Ipp32f* pY, int n, float& fAngle, float& a, float& b)
{
	Ipp32f pSumX, pSumY, pSumXY, pSumXX;
	ippsSum_32f(pX, n, &pSumX, ippAlgHintFast);
	ippsSum_32f(pY, n, &pSumY, ippAlgHintFast);
	ippsDotProd_32f(pX, pY, n, &pSumXY);
	ippsDotProd_32f(pX, pX, n, &pSumXX);
	if (abs(pSumX*pSumX - n * pSumXX) < 1e-8)
	{
		fAngle = 90;
		return false;
	}
	else
	{
		b = (pSumXY * pSumX - pSumY * pSumXX) / (pSumX*pSumX - n * pSumXX);
		a = (pSumY * pSumX - n * pSumXY) / (pSumX*pSumX - n * pSumXX);
		fAngle = float(atan(a) * 180 / PI);
		return true;
	}

}

//y = ax + b
bool CZSFittingLine::FiltLine2D(kxPoint<float>* pts, int n, float& fAngle, float& a, float& b)
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

void CZSFittingLine::combine(int* arr, int start, int* result, int count, int num, int& nTotal, int** Result)
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


float CZSFittingLine::FitLineByRansac(float* pArray, int nArrayStart, int nLen, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb)
{
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
	float fInliersPercentage = FitLineByRansac(ptFit, nDots, nFitLineDots, fBestAngle, fBesta, fBestb);

	return fInliersPercentage;
}

float CZSFittingLine::FitLineByRansac(float* pX, float* pY, int nArrayStart, int nLen, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb)
{
	int nDots = nSampleCount;
	if (nLen < nSampleCount)
	{
		nDots = nLen;
	}

	int nStep = nLen / nDots;

	kxPoint<float>* ptFit = new kxPoint<float>[nDots];
	for (int i = 0; i < nDots; i++)
	{
		ptFit[i].x = pX[i*nStep + 1 + nArrayStart];
		ptFit[i].y = pY[i*nStep + 1 + nArrayStart];
	}

	float fInliersPercentage = FitLineByRansac(ptFit, nDots, nFitLineDots, fBestAngle, fBesta, fBestb);

	return fInliersPercentage;
}

float CZSFittingLine::FitLineByRansac(kxPoint<float>* ptFit, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb)
{
	kxPoint<float>* pFitArray = new kxPoint<float>[nFitLineDots];

	int nMaxVoteCount = INT_MIN;
	float fInliersPercentage = 0.0;
	

	int N = nSampleCount;
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
		for (int i = 0; i < nFitLineDots; i++)
		{
			int nIndex = result[nIterCount][i];
			pFitArray[i] = ptFit[nIndex];
		}
		float fAngle, a, b;
		int nVoteCount = 0;
		if (FiltLine2D(pFitArray, nFitLineDots, fAngle, a, b))
		{
			for (int k = 0; k < nSampleCount; k++)
			{
				if (abs(ptFit[k].y - a * ptFit[k].x - b) < 2)//容许误差2pixel
				{
					nVoteCount++;
				}
			}
		}
		else
		{
			for (int k = 0; k < nSampleCount; k++)
			{
				if (abs(ptFit[k].x - pFitArray[0].x) < 2)
				{
					nVoteCount++;
					b = ptFit[k].x;//2022.3.23 总要给b赋个值
				}
			}

		}

		if (nVoteCount > nMaxVoteCount)
		{
			nMaxVoteCount = nVoteCount;
			fInliersPercentage = (float)nVoteCount / nSampleCount;
			fBesta = a;
			fBestAngle = fAngle;
			fBestb = b;
		}

		nIterCount++;
	}

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


float CZSFittingLine::FitLineByRansac(kxPoint<int>* ptFit, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb)
{
	kxPoint<float>* pf = new kxPoint<float>[nSampleCount];

	for (int i = 0; i < nSampleCount; i++)
	{
		pf[i] = ptFit[i];
	}

	float percent = FitLineByRansac(pf, nSampleCount, nFitLineDots, fBestAngle, fBesta, fBestb);

	delete[] pf;
	return percent;
}