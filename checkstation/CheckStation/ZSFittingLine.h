#pragma once

#include "opencv2/opencv.hpp"
#include "KxAlogrithm.h"


/*
	���ֱ�ߣ�����ԭ������С���˷��������ڵ�Ƚ�ɢ��ʱ��ʹ��ransac���
	��������Ҫ����б���Ƿ����1����Ϊ����1��С��1�������岻һ����������Ϊͼ�����������������ģ�������1С��1������x�����廹��y������
*/

class CZSFittingLine
{
public:
	CZSFittingLine() {};
	~CZSFittingLine() {};


	/*
		pArray		: �������飬����yֵ������x��������
		nArrayStart : ��ʼ��λ��
		nLen		: ���鳤��
		nSampleCount: ������������ʹ�þ������飬���ȡ��
		nFitLineDots: ��ϵ���
		fBestAngle  : ��Ͻ��-�Ƕ�
		fBesta		: ��Ͻ��-A
		fBestb		: ��Ͻ��-B

		y = ax + b ��angle�Ƕ��ߵĲ���	��
	*/
	float FitLineByRansac(float* pArray, int nArrayStart, int nLen, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb);


	/*
	pX			: xֵ����
	pY			: yֵ����
	nArrayStart : ��ʼ��λ��
	nLen		: ���鳤��
	nSampleCount: ������������ʹ�þ������飬���ȡ��
	nFitLineDots: ��ϵ���
	fBestAngle  : ��Ͻ��-�Ƕ�
	fBesta		: ��Ͻ��-A
	fBestb		: ��Ͻ��-B

	y = ax + b ��angle�Ƕ��ߵĲ���	��
	*/
	float FitLineByRansac(float* pX, float* pY, int nArrayStart, int nLen, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb);

private:
	float FitLineByRansac(kxPoint<float>* ptFit, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb);

	bool FiltLine(Ipp32f* pX, Ipp32f* pY, int n, float& fAngle, float& a, float& b);
	bool FiltLine2D(kxPoint<float>* pts, int n, float& fAngle, float& a, float& b);
	void combine(int* arr, int start, int* result, int count, int num, int& nTotal, int** Result);
};