#pragma once

#include "opencv2/opencv.hpp"
#include "KxAlogrithm.h"


/*
	拟合直线，基本原理还是最小二乘法，但对于点比较散的时候使用ransac拟合
	参数里面要包含斜率是否大于1，因为大于1跟小于1决定主体不一样，这是因为图像索引是整数决定的，而大于1小于1决定了x是主体还是y是主体
*/

class CZSFittingLine
{
public:
	CZSFittingLine() {};
	~CZSFittingLine() {};


	/*
		pArray		: 输入数组，代表y值，其中x数组索引
		nArrayStart : 起始点位置
		nLen		: 数组长度
		nSampleCount: 采样集，这里使用均分数组，间隔取点
		nFitLineDots: 拟合点数
		fBestAngle  : 拟合结果-角度
		fBesta		: 拟合结果-A
		fBestb		: 拟合结果-B

		y = ax + b （angle是对线的补充	）
	*/
	float FitLineByRansac(float* pArray, int nArrayStart, int nLen, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb);


	/*
	pX			: x值数组
	pY			: y值数组
	nArrayStart : 起始点位置
	nLen		: 数组长度
	nSampleCount: 采样集，这里使用均分数组，间隔取点
	nFitLineDots: 拟合点数
	fBestAngle  : 拟合结果-角度
	fBesta		: 拟合结果-A
	fBestb		: 拟合结果-B

	y = ax + b （angle是对线的补充	）
	*/
	float FitLineByRansac(float* pX, float* pY, int nArrayStart, int nLen, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb);

private:
	float FitLineByRansac(kxPoint<float>* ptFit, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb);

	bool FiltLine(Ipp32f* pX, Ipp32f* pY, int n, float& fAngle, float& a, float& b);
	bool FiltLine2D(kxPoint<float>* pts, int n, float& fAngle, float& a, float& b);
	void combine(int* arr, int start, int* result, int count, int num, int& nTotal, int** Result);
};