#pragma once

#include "opencv2/opencv.hpp"
#include "KxAlogrithm.h"
#include "EmpiricaAlgorithm.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/partitioner.h"
using namespace tbb;

class CCombineImg
{
public:
	CCombineImg() { m_bisinit = false; }
	~CCombineImg() {}

	enum
	{
		_MAX_SCAN_NUM = 6,//最多扫描六列
	};

	void Init(int nW, int nH, int ncol, int* nnums, kxRect<int>* modelroi);// ncol是扫描组数，后面数组的大小跟之匹配

	int appendImg(kxCImageBuf& srcimg, int nImgIndex);// 这个nImgIndex是图像ID，每次换新的车会清零. 返回0为发生匹配错误，返回1为正常

	bool IsColFull(int &nIndex);// 判断是否有列数满了，拿出来检测，返回nIndex代表那一列

	bool IsAllFull();//判断是否所有列满了

	void GetImg(kxCImageBuf& dstimgA, kxCImageBuf& dstimgB, int nIndex);

	void ResetCol(int &nIndex);// 该列满了，重置该列

	void Clear();//清楚所有，重置 

private:
	kxCImageBuf		m_ImgBigListA[_MAX_SCAN_NUM];// 输入进来的大图，正着走扫一列

	kxCImageBuf		m_ImgBigListB[_MAX_SCAN_NUM];

	bool			m_bstatus[_MAX_SCAN_NUM];// 每列图像的拼接状态，检测完之后会进行复位

	int				m_nEveryColImgnum[_MAX_SCAN_NUM];//每一列扫描区域的图像数量

	kxRect<int>		m_rectmodel[_MAX_SCAN_NUM * 2];
	
	int				m_nSingleH;

	int				m_nSingleW;

	CEmpiricaAlgorithm	m_hAlg;

	int				m_nScanTimes;//扫描次数

	int				m_nCurScanTimes;//当前pack扫描次数，用于判断当前pack是否已经完全扫完

	bool			m_bisinit;


	bool MatchTemplateAndTransform(int ncol, kxRect<int> matchrect, int &ncaptureoffset);
};