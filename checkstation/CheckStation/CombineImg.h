#pragma once

#include "opencv2/opencv.hpp"
#include "KxAlogrithm.h"
#include "EmpiricaAlgorithm.h"


class CCombineImg
{
public:
	CCombineImg() {}
	~CCombineImg() {}

	enum
	{
		_MAX_SCAN_NUM = 6,//最多扫描六列
	};

	void Init(int nW, int nH, int ncol, int* nnums, kxRect<int>* modelroi);// ncol是扫描组数，后面数组的大小跟之匹配

	void appendImg(kxCImageBuf& srcimg, int nImgIndex);// 这个nImgIndex是图像ID，每次换新的车会清零

	bool IsColFull(int &nIndex);// 判断是否有列数满了，拿出来检测，返回nIndex代表那一列

	void GetImg(kxCImageBuf& dstimgA, kxCImageBuf& dstimgB, int nIndex);

	void ResetCol(int &nIndex);// 该列满了，重置该列

private:
	kxCImageBuf		m_ImgBigListA[_MAX_SCAN_NUM];// 输入进来的大图，正着走扫一列

	kxCImageBuf		m_ImgBigListB[_MAX_SCAN_NUM];

	bool			m_bstatus[_MAX_SCAN_NUM];

	int				m_nEveryColImgnum[_MAX_SCAN_NUM];//每一列扫描区域的图像数量

	kxRect<int>		m_rectmodel[_MAX_SCAN_NUM];
	
	int				m_nSingleH;

	int				m_nSingleW;

	CEmpiricaAlgorithm	m_hAlg;


	void MatchTemplateAndTransform(int ncol);
};