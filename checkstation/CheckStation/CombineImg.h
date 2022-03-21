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
		_MAX_SCAN_NUM = 6,//���ɨ������
	};

	void Init(int nW, int nH, int ncol, int* nnums, kxRect<int>* modelroi);// ncol��ɨ����������������Ĵ�С��֮ƥ��

	int appendImg(kxCImageBuf& srcimg, int nImgIndex);// ���nImgIndex��ͼ��ID��ÿ�λ��µĳ�������. ����0Ϊ����ƥ����󣬷���1Ϊ����

	bool IsColFull(int &nIndex);// �ж��Ƿ����������ˣ��ó�����⣬����nIndex������һ��

	bool IsAllFull();//�ж��Ƿ�����������

	void GetImg(kxCImageBuf& dstimgA, kxCImageBuf& dstimgB, int nIndex);

	void ResetCol(int &nIndex);// �������ˣ����ø���

	void Clear();//������У����� 

private:
	kxCImageBuf		m_ImgBigListA[_MAX_SCAN_NUM];// ��������Ĵ�ͼ��������ɨһ��

	kxCImageBuf		m_ImgBigListB[_MAX_SCAN_NUM];

	bool			m_bstatus[_MAX_SCAN_NUM];// ÿ��ͼ���ƴ��״̬�������֮�����и�λ

	int				m_nEveryColImgnum[_MAX_SCAN_NUM];//ÿһ��ɨ�������ͼ������

	kxRect<int>		m_rectmodel[_MAX_SCAN_NUM * 2];
	
	int				m_nSingleH;

	int				m_nSingleW;

	CEmpiricaAlgorithm	m_hAlg;

	int				m_nScanTimes;//ɨ�����

	int				m_nCurScanTimes;//��ǰpackɨ������������жϵ�ǰpack�Ƿ��Ѿ���ȫɨ��

	bool			m_bisinit;


	bool MatchTemplateAndTransform(int ncol, kxRect<int> matchrect, int &ncaptureoffset);
};