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
#include "ZSFittingLine.h"
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

	enum
	{
		_MAX_BLOBIMG = 30,// 最大连通域分析数量
		_SINGLE_BLOBIMG_H = 2000,//每个进入分析的图像大小
		_IMG_OVERLAP = 20, // 图像重叠大小
		//_MAX_MASK_NUM = 50, // 掩膜最大数量，高亮、暗
	};

	struct SingleParam
	{
		kxRect<int>		m_rcCheckROI;
		//std::vector(kxRect<int>)		m_prcHighmaskROI[_MAX_MASK_NUM];
		std::vector<kxRect<int>>	m_vecrcHighmaskROI;
		std::vector<kxRect<int>>	m_vecrcLowmaskROI;


		int				m_nGrabTimes;//扫描列，设备移动扫描第几组,说明当前扫描图像属于第几组
		int				m_nCurGrabID;// 当前扫描列第几个roi, 根据组数中第几个roi确定的
		int				m_nIsnotcheck;//是否屏蔽当前检测


		int				m_ndefectthresh;
		int				m_ndefectdots;//缺陷点数

		int				m_noffsethigh;//高灵敏度偏移，从主站读取结果直接乘以10，目的是统一到255的灰阶
		int				m_noffsetlow;//低灵敏度偏移，从主站读取结果直接乘以10，目的是统一到255的灰阶
		int				m_noffsetcolor;//色差灵敏度，也即H值的灵敏度，从主站读取结果直接乘以10，目的是统一到255的灰阶
		int				m_nstandardH;//标准色调H 值

		kxCImageBuf		m_ImgTemplate;//压缩的模板，专门用于检测涂胶是否大面积缺的，压缩系数看建模  20220324 暂时没用了


		// 反光校正参数
		int				m_nveroffsetpos;//垂直偏移距离
		int				m_nhoroffsetpos;//水平偏移距离
		int				m_ngrayoffset;//偏移灰度值, 只针对检高
		int				m_nHoffset;// 偏移色调值，只针对检低。因为实验得到的结果

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
	kxCImageBuf				 m_ImgGlueMask;//蓝胶区域
	kxCImageBuf				 m_ImgColorGlueMask;//蓝胶区域

	kxCImageBuf				 m_ImgGlueMaskFillholes;//涂胶区域掩膜，填充孔洞的目的是提取蓝胶时没有提取
	kxCImageBuf				 m_ImgColorGlueMaskFillholes;

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

	kxCImageBuf				 m_ImgMerge;
	kxCImageBuf				 m_ImgCheck;

	kxCImageBuf				 m_ImgSrcA;
	kxCImageBuf				 m_ImgSrcB;

	kxCImageBuf				 m_pBLOBIMG[_MAX_BLOBIMG];
	CKxBlobAnalyse			 m_phblob[_MAX_BLOBIMG];
	int						 m_nblobimgnum;

	kxCImageBuf				 m_ImgZero2split;
	int						 m_nID;//当前是第几个blob，用于存储缺陷图片信息
	kxRect<int>				 m_recttarget;

	kxCImageBuf				 m_Blobimg1;// 检测断胶结果
	kxCImageBuf				 m_Blobimg2;// 检测灰度高结果
	kxCImageBuf				 m_Blobimg3;// 检测灰度低结果
	kxCImageBuf				 m_Blobimg4;// 检测 H通道色差结果

	CZSFittingLine			 m_hfitline;

	kxCImageBuf				 m_ImgEdgeGrayMask;
	kxCImageBuf				 m_ImgEdgeHMask;

//public:
//	void SaveDebugImg(const char* name, kxCImageBuf& saveimg);// 保存图像
//
//

private:
	void checkcolordiff(const kxCImageBuf& SrcImg);// 检色差

	void checkyiwu(const kxCImageBuf& SrcImg, Json::Value &checkresult);// 检异物，包含断胶

	void checkqipao(const kxCImageBuf& SrcImg); // 检气泡

	void checkEdge(const kxCImageBuf& SrcImg);

	void GetGlueMask(const kxCImageBuf* RGB);//用特定方法提取掩膜


	void MergeImg(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg);

	void GetMaxGray(const kxCImageBuf& ColorImg, kxCImageBuf& DstGrayImg);

	void CutImg2MulImg(const kxCImageBuf& CheckImg);

	void ParallelBlob(Json::Value &checkresult);//并行blob,把图切片进行blob

	void SearchEdge(const kxCImageBuf& GrayImg, int ndir, int nThreshvalue, int& nedge1, int& nedge2);//根据搜索方向，由中间向两侧搜索，得到两个边

	void ExtractGreen(const kxCImageBuf* RGB, kxRect<int> roi, kxCImageBuf& DstImg);


	// 2022.2.5

	// 搜索边缘，nDir = 0是从左到右搜，且梯度算子是-1 0 1形式。1则反之 
	// nDir = 2是从下到上搜，且梯度算子是1 0 -1形式。3则反之 
	void GetEdgePoints(const kxCImageBuf& GrayImg, int nDir, int nthresh, kxPoint<int> * searchresult, int nnums);

	// 特殊方法滑动匹配。先归一化，后滑动对减，滑动对减的方式也有些不同
	void SliderMatch(kxCImageBuf& SrcImg, kxCImageBuf& Templateimg);

	void SliderSub(kxCImageBuf& SrcImg, kxCImageBuf& TemplateImg, kxCImageBuf& dstCCImg, int nscalefactor);

	void CheckLow(kxCImageBuf& SrcImg);//检测蚊虫以及胶内断胶


	//2022.02.28 
	void GetTargetROI(const kxCImageBuf& SrcImg, kxRect<int> rcCheck, kxRect<int>& targetrect);//在不越界前提下获取加大版ROI

	void MergeImgNew(const kxCImageBuf& SrcImg1, const kxCImageBuf& SrcImg2, kxRect<int> targetrect, kxCImageBuf& DstImg);

	void GetTargetImg(const kxCImageBuf& SrcImg, int nCheckROIW, int nCheckROIH, kxCImageBuf& DstImg, kxRect<int>& targetrect);// 获取核心检测区域以及区域大小

	void ExtractGreenNew(const kxCImageBuf* RGB,  kxCImageBuf& DstImg);

	void SliderMatchNew(kxCImageBuf& SrcImg, int ntargetw, int ntargeth, kxCImageBuf& Templateimg, kxCImageBuf& blobimg);

	void ModelFillholes(kxCImageBuf& SrcImg, kxCImageBuf& DstImg);

	void checkwithmodel(const kxCImageBuf& SrcImg, const kxCImageBuf& gluearea, kxCImageBuf& dsthigh, kxCImageBuf& dstlow);// 通过模板方式把缺陷找出

	void CreateBaseModel(const kxCImageBuf& SrcImg, const kxCImageBuf& gluemask, kxCImageBuf& DstHigh, kxCImageBuf& DstLow);

	int JudgeDefectType(kxCImageBuf& blobimg, kxRect<int>rect);

	//void Transhsv2check(kxCImageBuf& SrcImg, kxCImageBuf& DstHigh, kxCImageBuf& DstLow);// 将RBG转换为hsv进行检测

	void GetHmask(const kxCImageBuf& SrcImg, kxCImageBuf& DstMask);// 获取不并入的H检测的掩膜

	void checkwithmodelNew(const kxCImageBuf& SrcImg, const kxCImageBuf& gluearea, kxRect<int> ROI, kxCImageBuf& dsthigh, kxCImageBuf& dstlow, kxCImageBuf& dstH);// RGB 检高低, H检色差

	void GetMaskHL(kxCImageBuf& maskhigh, kxCImageBuf& masklow);// 获取高低掩膜图案

	void MaskEdge(kxCImageBuf& SrcDstImg, kxRect<int> roi, int nedge);// 掩膜边缘，暴力去除边缘干扰

	void GetEdgeCorrectionMask(const kxCImageBuf& SrcImg, kxRect<int> roi, kxCImageBuf& dstmaskgray, kxCImageBuf& dstmaskH);// 获取边缘校正用的模板
};