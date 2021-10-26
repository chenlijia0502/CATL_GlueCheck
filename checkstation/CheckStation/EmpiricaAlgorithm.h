#pragma once

/*!
	Date:			2021.5.12
	Author:			HYH
	Description:	���ļ�Ϊÿ����Ŀ�ر��ļ����������ڴ��ļ�����ӿ�����ʹ�õ��Ŀɹ�������Ŀʹ�õ�
					ͨ�ú���������0Ϊ����������
	
	�ύ������ʽ���£�

	//author��	 ABC
	//fun�� �ú���........
	//param1: ......,  param2 .........
	int fun(param1, param2, ...)


*/

#include "KxAlogrithm.h"
#include "opencv2/opencv.hpp"


class CZSMat : public cv::Mat
{
	CZSMat(kxCImageBuf& srcimg)
	{
		Mat(srcimg.nHeight, srcimg.nWidth, CV_8UC(srcimg.nChannel), srcimg.buf);
	}
	~CZSMat() {}
};


class CEmpiricaAlgorithm
{
	// ����Ĭ�Ϸ���0Ϊ����

public:
	CEmpiricaAlgorithm();
	~CEmpiricaAlgorithm();

	enum THRESH_TYPE
	{
		_BINARY,// ����ĳ��ֵ��Ϊ255������ȫΪ0
		_BINARY_INV,// С��ĳ��ֵ��Ϊ255������ȫΪ0

	}; 

private:
	CKxBaseFunction m_hfun;
	cv::Mat m_normalsrc;
	cv::Mat	m_MatSrcProjectV;
	cv::Mat	m_MatTempProjectV;
	cv::Mat	m_imgResultcv;
	cv::Mat	m_imgResizeSrc;
	cv::Mat m_MatSrcProjectH;
	cv::Mat	m_MatTempProjectH;
	kxCImageBuf m_ImgBGR, m_ImgHSV, m_ImgLAB;
	kxCImageBuf		m_ImgSubresult;
	kxCImageBuf		m_MarkImg;
	cv::Mat		m_MatState, m_MatLabel, m_MatCentroids;

public:
	/*
	fun:	ͶӰ�õ����л���ͼ�񣬲����й�һ��
	author: HYH
	nDir   _Horizontal_Project_Dir = 1 ����ˮƽ��_Vertical_Project_Dir = 0 ����ֱ
	*/
	void CreatProjectImg(const kxCImageBuf& SrcImg, cv::Mat& DstImg, int nDir, int nscalefactor = 1);
	void CreatProjectImg(const cv::Mat& SrcImg, cv::Mat& DstImg, int nDir, int nscalefactor = 1);

	/*
	fun:	��ֵ��
	author: HYH
	*/
	int ThreshImg(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nthresh, THRESH_TYPE nthreshtype);

	/*
	fun:	ͼ��ֱ��ˮƽͶӰ֮�����ģ��ƥ�䣨���һ����
	author:	HYH
	minConfidence ��С���Ŷȣ�ˮƽ��ֱ���ζ�λ��ֱ���֮�Ա�
	nDirection ��λ���� _Horizontal_Project_Dir = 1 ����ˮƽ��_Vertical_Project_Dir = 0 ����ֱ
	return:  0ΪOK, 1Ϊ��ֱ��λʧ�ܣ�2Ϊˮƽ��λʧ��
	
	*/
	int Project2Locate(const kxCImageBuf& SrcImg, const kxCImageBuf& TemplateImg, int nDirection, float minConfidence, int& nDx, int& nDy, float& fConfidence);


	/*
	fun: ģ��ƥ��
	nconverlayer��ָ����ͼ���ǲ�ɫʱ���ת����0Ϊ��ת��Ҳ���ǲ�ɫƥ���ɫ���ڰ�ƥ��ڰף� 1��2��3��4 ��Ӧ R/G/B/GRAY
	matchmode Ŀǰֻ֧�� TM_CCOEFF_NORMED �� TM_SQDIFF_NORMED ����ģʽ
	
	*/
	float Matchtemplate(kxPoint<float>& pos, const kxCImageBuf& src, const kxCImageBuf& templ, cv::Mat& tmpResult, cv::Mat& resizesrc, int resizefactor=1, int nconverlayer = 0, cv::TemplateMatchModes matchmode = cv::TM_CCOEFF_NORMED);
	float Matchtemplate(kxPoint<float>& pos, const cv::Mat& src, const cv::Mat& templ, cv::Mat& tmpResult, cv::Mat& resizesrc, int resizefactor=1, int nconverlayer = 0, cv::TemplateMatchModes matchmode = cv::TM_CCOEFF_NORMED);
	float Matchtemplate(kxPoint<float>& pos, const kxCImageBuf& src, const kxCImageBuf& templ, int nconverlayer=0, cv::TemplateMatchModes matchmode = cv::TM_CCOEFF_NORMED);
	float Matchtemplate(kxPoint<float>& pos, const cv::Mat& src, const cv::Mat& templ, int nconverlayer=0, cv::TemplateMatchModes matchmode = cv::TM_CCOEFF_NORMED);

	/*
	fun:	�����в�õ����ƥ�䣬Templateimg��SrcImg�ϻ�������С�в�����TemplateImg�Ǽ�������opencv SSD��һ�����������ǰ�ͨ���޷������
	ntype   Ϊ0ʱTemplateImg�Ǽ����������Ǳ�����

	2021.09.12 Ŀǰֻ֧�ֵ�ͨ��
	*/
	void MatchUseSSD(const kxCImageBuf& SrcImg, const kxCImageBuf& Templateimg, int ntype, kxPoint<int>& matchresult);// 


	/*
	fun:	����׶�����������ȱ����ΪҲ�ǿ׶���
	author:	HYH
	SrcImg Ϊ������ͨ���ֵ��ͼ
	*/
	void CalculateHoleNums(const kxCImageBuf& SrcImg, int &nholenums);
	void CalculateHoleNums(const cv::Mat& SrcImg, int &nholenums);


	// �ָ�RGB��ͨ��ͼ
	//void SplitRGB(const kxCImageBuf& SrcImg, kxCImageBuf* dstimgarray);



	int ZSConvertImageLayer(const unsigned char* srcBuf, int srcPitch, int nImgType,
		unsigned char* dstBuf, int dstPitch, int width, int height, int nChangeType, KxCallStatus& hCall);
	int ZSConvertImageLayer(const cv::Mat& SrcImg, cv::Mat& DstImg, int nChangeType);


	//void ZSResizeSuper(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);

	// ��ԾɵĿ�������˱�Ե���ص�����
	int ZSErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask = NULL, IppiBorderType bordertype = ippBorderRepl, int nbordervalue = 0);

	int ZSDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask = NULL, IppiBorderType bordertype = ippBorderRepl, int nbordervalue = 0);


	/*
		fun:	��SrcImg �� templateimg�ֳ��Ŀ�ƥ�䣬�ҵ�ȫ����ѽ��
		author:	HYH
		ƥ�䷽���� TM_SQDIFF
	*/
	float MatchTemplateInBlock(cv::Mat &SrcImg, cv::Mat &templateimg, cv::Point& pos);
	float MatchTemplateInBlock(const kxCImageBuf& SrcImg, const kxCImageBuf& templateimg, kxPoint<int>& pos);

	/*
		fun: ����һ�Ŷ�ֵ��ͼ��centerpointΪ���ģ��õ�nDirectionNum�������Ϸ������ݵĵ���֮�ͣ�����ᱻ�Ż� presult ������
		author: HYH
		nDirectionNum�� ��ҪΪ2 4 8��2��0����90�㣬4��0�㣬45�㣬90�㣬135�㣬8������ϸ�֡��������Ĭ��4

	*/
	void CalEachDirectionPointNum(const cv::Mat& SrcImg, cv::Point& centerpoint, int *presult, int nDirectionNum=4);
	void CalEachDirectionPointNum(const kxCImageBuf& SrcImg, kxPoint<int>& centerpoint, int *presult, int nDirectionNum=4);

	/*
		����һ�Ŷ�ֵ��ͼ��centerpointΪ���ģ��õ�angle�Ƕȷ����Ϸ������ݵĵ���֮��
	*/
	void CalSingleDirectionPointNum(const cv::Mat& SrcImg, cv::Point& centerpoint, int angle, int &nnum);
	void CalSingleDirectionPointNum(const kxCImageBuf& SrcImg, kxPoint<int>& centerpoint, int angle, int &nnum);


	/*
		�˳�С��
		nconnectivity �� 4��8,����ͨ������ͨ
	*/
	int ZSFilterSpeckles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize, int nconnectivity);


	/*
		����blob�ķ�����С��nMaxSpeckleSize����ͨ���˳�
	*/
	void ZSFilterSpecklesWithBlob(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize, int nconnectivity);

	void ZSFilterSpecklesWithBlob(const cv::Mat& SrcImg, cv::Mat& DstImg, int nMaxSpeckleSize, int nconnectivity);

};