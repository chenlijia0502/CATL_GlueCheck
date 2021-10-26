#pragma once

/*!
	Date:			2021.5.12
	Author:			HYH
	Description:	此文件为每个项目必备文件，开发者在此文件中添加开发中使用到的可供其他项目使用的
					通用函数（返回0为函数正常）
	
	提交函数格式如下：

	//author：	 ABC
	//fun： 该函数........
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
	// 该类默认返回0为正常

public:
	CEmpiricaAlgorithm();
	~CEmpiricaAlgorithm();

	enum THRESH_TYPE
	{
		_BINARY,// 大于某个值设为255，其余全为0
		_BINARY_INV,// 小于某个值设为255，其余全为0

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
	fun:	投影得到单行或单列图像，并进行归一化
	author: HYH
	nDir   _Horizontal_Project_Dir = 1 代表水平，_Vertical_Project_Dir = 0 代表垂直
	*/
	void CreatProjectImg(const kxCImageBuf& SrcImg, cv::Mat& DstImg, int nDir, int nscalefactor = 1);
	void CreatProjectImg(const cv::Mat& SrcImg, cv::Mat& DstImg, int nDir, int nscalefactor = 1);

	/*
	fun:	二值化
	author: HYH
	*/
	int ThreshImg(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nthresh, THRESH_TYPE nthreshtype);

	/*
	fun:	图像垂直、水平投影之后进行模板匹配（会归一化）
	author:	HYH
	minConfidence 最小置信度，水平垂直两次定位需分别与之对比
	nDirection 定位方向 _Horizontal_Project_Dir = 1 代表水平，_Vertical_Project_Dir = 0 代表垂直
	return:  0为OK, 1为垂直定位失败，2为水平定位失败
	
	*/
	int Project2Locate(const kxCImageBuf& SrcImg, const kxCImageBuf& TemplateImg, int nDirection, float minConfidence, int& nDx, int& nDy, float& fConfidence);


	/*
	fun: 模板匹配
	nconverlayer是指输入图像是彩色时如何转换，0为不转换也就是彩色匹配彩色、黑白匹配黑白； 1、2、3、4 对应 R/G/B/GRAY
	matchmode 目前只支持 TM_CCOEFF_NORMED 、 TM_SQDIFF_NORMED 两种模式
	
	*/
	float Matchtemplate(kxPoint<float>& pos, const kxCImageBuf& src, const kxCImageBuf& templ, cv::Mat& tmpResult, cv::Mat& resizesrc, int resizefactor=1, int nconverlayer = 0, cv::TemplateMatchModes matchmode = cv::TM_CCOEFF_NORMED);
	float Matchtemplate(kxPoint<float>& pos, const cv::Mat& src, const cv::Mat& templ, cv::Mat& tmpResult, cv::Mat& resizesrc, int resizefactor=1, int nconverlayer = 0, cv::TemplateMatchModes matchmode = cv::TM_CCOEFF_NORMED);
	float Matchtemplate(kxPoint<float>& pos, const kxCImageBuf& src, const kxCImageBuf& templ, int nconverlayer=0, cv::TemplateMatchModes matchmode = cv::TM_CCOEFF_NORMED);
	float Matchtemplate(kxPoint<float>& pos, const cv::Mat& src, const cv::Mat& templ, int nconverlayer=0, cv::TemplateMatchModes matchmode = cv::TM_CCOEFF_NORMED);

	/*
	fun:	滑动残差得到最佳匹配，Templateimg在SrcImg上滑动找最小残差结果，TemplateImg是减数。跟opencv SSD不一样的是这里是八通道无符号相减
	ntype   为0时TemplateImg是减数，其余是被减数

	2021.09.12 目前只支持单通道
	*/
	void MatchUseSSD(const kxCImageBuf& SrcImg, const kxCImageBuf& Templateimg, int ntype, kxPoint<int>& matchresult);// 


	/*
	fun:	计算孔洞个数（明显缺口认为也是孔洞）
	author:	HYH
	SrcImg 为单八连通域二值化图
	*/
	void CalculateHoleNums(const kxCImageBuf& SrcImg, int &nholenums);
	void CalculateHoleNums(const cv::Mat& SrcImg, int &nholenums);


	// 分割RGB三通道图
	//void SplitRGB(const kxCImageBuf& SrcImg, kxCImageBuf* dstimgarray);



	int ZSConvertImageLayer(const unsigned char* srcBuf, int srcPitch, int nImgType,
		unsigned char* dstBuf, int dstPitch, int width, int height, int nChangeType, KxCallStatus& hCall);
	int ZSConvertImageLayer(const cv::Mat& SrcImg, cv::Mat& DstImg, int nChangeType);


	//void ZSResizeSuper(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);

	// 相对旧的库里，增加了边缘像素的设置
	int ZSErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask = NULL, IppiBorderType bordertype = ippBorderRepl, int nbordervalue = 0);

	int ZSDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask = NULL, IppiBorderType bordertype = ippBorderRepl, int nbordervalue = 0);


	/*
		fun:	将SrcImg 与 templateimg分成四块匹配，找到全局最佳结果
		author:	HYH
		匹配方法： TM_SQDIFF
	*/
	float MatchTemplateInBlock(cv::Mat &SrcImg, cv::Mat &templateimg, cv::Point& pos);
	float MatchTemplateInBlock(const kxCImageBuf& SrcImg, const kxCImageBuf& templateimg, kxPoint<int>& pos);

	/*
		fun: 输入一张二值化图像，centerpoint为中心，得到nDirectionNum个方向上非零数据的点数之和，结果会被放回 presult 数组中
		author: HYH
		nDirectionNum： 主要为2 4 8，2即0°与90°，4即0°，45°，90°，135°，8即更加细分。其余情况默认4

	*/
	void CalEachDirectionPointNum(const cv::Mat& SrcImg, cv::Point& centerpoint, int *presult, int nDirectionNum=4);
	void CalEachDirectionPointNum(const kxCImageBuf& SrcImg, kxPoint<int>& centerpoint, int *presult, int nDirectionNum=4);

	/*
		输入一张二值化图像，centerpoint为中心，得到angle角度方向上非零数据的点数之和
	*/
	void CalSingleDirectionPointNum(const cv::Mat& SrcImg, cv::Point& centerpoint, int angle, int &nnum);
	void CalSingleDirectionPointNum(const kxCImageBuf& SrcImg, kxPoint<int>& centerpoint, int angle, int &nnum);


	/*
		滤除小点
		nconnectivity ： 4、8,四连通、八连通
	*/
	int ZSFilterSpeckles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize, int nconnectivity);


	/*
		采用blob的方法把小于nMaxSpeckleSize的连通域滤除
	*/
	void ZSFilterSpecklesWithBlob(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize, int nconnectivity);

	void ZSFilterSpecklesWithBlob(const cv::Mat& SrcImg, cv::Mat& DstImg, int nMaxSpeckleSize, int nconnectivity);

};