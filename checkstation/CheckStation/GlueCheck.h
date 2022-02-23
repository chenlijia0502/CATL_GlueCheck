#pragma once

#include "BaseCheckMethod.h"
#include "KxWarpStretch.h"
#include "KxBaseFunction.h"
#include "KxBlobAnalyse.h"
#include "EmpiricaAlgorithm.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/partitioner.h"
using namespace tbb;


class CGlueCheck : public BaseCheckMethod
{
public:
	CGlueCheck();
	~CGlueCheck();

	//int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult);

	int Check(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg, Json::Value &checkresult);

	bool ReadParamXml(const char* filePath, char *ErrorInfo, const char * templatepath = NULL);

	void SetParam(void *param) 
	{
		m_param = *((SingleParam*)param);
	}

	void SetCheckBlobID(int nId) { m_nID = nId;  }

	struct SingleParam
	{
		kxRect<int>		m_rcCheckROI;
		int				m_nGrabTimes;//扫描列，设备移动扫描第几组,说明当前扫描图像属于第几组
		int				m_nCurGrabID;// 当前扫描列第几个roi
		int				m_nIsnotcheck;//是否屏蔽当前检测


		int				m_ndefectthresh;
		int				m_ndefectdots;//缺陷点数

		int				m_noffsethigh;//高灵敏度偏移，从主站读取结果直接乘以25，目的是统一到255的灰阶
		int				m_noffsetlow;//低灵敏度偏移，从主站读取结果直接乘以25，目的是统一到255的灰阶

		kxCImageBuf		m_ImgTemplate;

	};

	enum
	{
		_MAX_BLOBIMG = 30,// 最大连通域分析数量
		_SINGLE_BLOBIMG_H = 2000,//每个进入分析的图像大小
		_IMG_OVERLAP = 20, // 图像重叠大小
	};


private:
	CKxBaseFunction			 m_hFun;
	kxCImageBuf				 m_ImgBase;
	kxCImageBuf				 m_WarpImg;
	CKxBlobAnalyse			 m_hBlobFun;
	SingleParam				 m_param;

	int						 m_nthresh;//提取异物灰度
	int						 m_nmindots;//异物最小点数
	cv::Mat					 m_matarraybgr[3];
	kxCImageBuf				 m_ImgGray;
	kxCImageBuf				 m_ImgThresh;
	kxCImageBuf				 m_ImgClose;
	kxCImageBuf				 m_ImgOpen;

	kxCImageBuf				 m_ImgRGB[3];
	kxCImageBuf				 m_ImgHSV[3];

	CEmpiricaAlgorithm		 m_hAlg;

	kxCImageBuf				 m_ImgmaxRegion;
	kxCImageBuf				 m_ImgGlueMask;//蓝胶掩膜
	kxCImageBuf				 m_ImgColorGlueMask;//蓝胶掩膜

	kxCImageBuf				 m_ImgR_Mask;

	kxCImageBuf				 m_ImgG_R;
	kxCImageBuf				 m_ImgG_B;
	kxCImageBuf				 m_ImgsubResult;

	kxCImageBuf				 m_ImgTemplateLow;//彩色低模板
	kxCImageBuf				 m_ImgTemplateHigh;//彩色高模板
	kxCImageBuf				 m_ImgBaseTemplate;

	kxCImageBuf				 m_ImgCheckLow;
	kxCImageBuf				 m_ImgCheckHigh;

	kxCImageBuf				 m_ImgCheckLowGray;
	kxCImageBuf				 m_ImgCheckHighGray;

	kxCImageBuf				 m_ImgCheck;

	kxCImageBuf				 m_ImgSrcA;
	kxCImageBuf				 m_ImgSrcB;

	kxCImageBuf				 m_pBLOBIMG[_MAX_BLOBIMG];
	CKxBlobAnalyse			 m_phblob[_MAX_BLOBIMG];
	int						 m_nblobimgnum;

	kxCImageBuf				 m_ImgZero2split;
	int						 m_nID;//当前是第几个blob，用于存储缺陷图片信息
	kxRect<int>				 m_recttarget;


private:
	void checkcolordiff(const kxCImageBuf& SrcImg);// 检色差

	void checkyiwu(const kxCImageBuf& SrcImg, Json::Value &checkresult);// 检异物，包含断胶

	void checkqipao(const kxCImageBuf& SrcImg); // 检气泡

	void checkEdge(const kxCImageBuf& SrcImg);

	void GetGlueMask(const kxCImageBuf* RGB);//用特定方法提取掩膜

	void CreateBaseModel(const kxCImageBuf& CheckImg);//创建基础模板，彩色高模板，彩色低模板

	void MergeImg(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg);

	void GetMaxGray(const kxCImageBuf& ColorImg, kxCImageBuf& DstGrayImg);

	void CutImg2MulImg(const kxCImageBuf& CheckImg);

	void ParallelBlob(Json::Value &checkresult);//并行blob,把图切片进行blob

	void SearchEdge(const kxCImageBuf& GrayImg, int ndir, int nThreshvalue, int& nedge1, int& nedge2);//根据搜索方向，由中间向两侧搜索，得到两个边

	void ExtractGreen(const kxCImageBuf* RGB, kxRect<int> roi, kxCImageBuf& DstImg);


	// 2022.2.5

	// 搜索边缘，nDir = 0是从左到右，且梯度算子是-1 0 1形式。1则反之 
	void GetEdgePoints(const kxCImageBuf& GrayImg, int nDir, int nthresh, kxPoint<int> * searchresult, int nnums);

	// 特殊方法滑动匹配。先归一化，后滑动对减，滑动对减的方式也有些不同
	void SliderMatch(kxCImageBuf& SrcImg, kxCImageBuf& Templateimg);

	void SliderSub(kxCImageBuf& SrcImg, kxCImageBuf& TemplateImg, kxCImageBuf& dstCCImg, int nscalefactor);


	void CheckLow(kxCImageBuf& SrcImg);//检测蚊虫以及胶内断胶

};