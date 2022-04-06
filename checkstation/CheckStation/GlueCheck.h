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
		_MAX_BLOBIMG = 30,// �����ͨ���������
		_SINGLE_BLOBIMG_H = 2000,//ÿ�����������ͼ���С
		_IMG_OVERLAP = 20, // ͼ���ص���С
		//_MAX_MASK_NUM = 50, // ��Ĥ�����������������
	};

	struct SingleParam
	{
		kxRect<int>		m_rcCheckROI;
		//std::vector(kxRect<int>)		m_prcHighmaskROI[_MAX_MASK_NUM];
		std::vector<kxRect<int>>	m_vecrcHighmaskROI;
		std::vector<kxRect<int>>	m_vecrcLowmaskROI;


		int				m_nGrabTimes;//ɨ���У��豸�ƶ�ɨ��ڼ���,˵����ǰɨ��ͼ�����ڵڼ���
		int				m_nCurGrabID;// ��ǰɨ���еڼ���roi, ���������еڼ���roiȷ����
		int				m_nIsnotcheck;//�Ƿ����ε�ǰ���


		int				m_ndefectthresh;
		int				m_ndefectdots;//ȱ�ݵ���

		int				m_noffsethigh;//��������ƫ�ƣ�����վ��ȡ���ֱ�ӳ���10��Ŀ����ͳһ��255�Ļҽ�
		int				m_noffsetlow;//��������ƫ�ƣ�����վ��ȡ���ֱ�ӳ���10��Ŀ����ͳһ��255�Ļҽ�
		int				m_noffsetcolor;//ɫ�������ȣ�Ҳ��Hֵ�������ȣ�����վ��ȡ���ֱ�ӳ���10��Ŀ����ͳһ��255�Ļҽ�
		int				m_nstandardH;//��׼ɫ��H ֵ

		kxCImageBuf		m_ImgTemplate;//ѹ����ģ�壬ר�����ڼ��Ϳ���Ƿ�����ȱ�ģ�ѹ��ϵ������ģ  20220324 ��ʱû����


		// ����У������
		int				m_nveroffsetpos;//��ֱƫ�ƾ���
		int				m_nhoroffsetpos;//ˮƽƫ�ƾ���
		int				m_ngrayoffset;//ƫ�ƻҶ�ֵ, ֻ��Լ��
		int				m_nHoffset;// ƫ��ɫ��ֵ��ֻ��Լ�͡���Ϊʵ��õ��Ľ��

	};




private:
	CKxBaseFunction			 m_hFun;
	kxCImageBuf				 m_ImgBase;
	kxCImageBuf				 m_WarpImg;
	CKxBlobAnalyse			 m_hBlobFun;
	SingleParam				 m_param;

	int						 m_nthresh;//��ȡ����Ҷ�
	int						 m_nmindots;//������С����
	cv::Mat					 m_matarraybgr[3];
	kxCImageBuf				 m_ImgGray;
	kxCImageBuf				 m_ImgThresh;
	kxCImageBuf				 m_ImgClose;
	kxCImageBuf				 m_ImgOpen;

	kxCImageBuf				 m_ImgRGB[3];
	kxCImageBuf				 m_ImgHSV[3];

	CEmpiricaAlgorithm		 m_hAlg;

	kxCImageBuf				 m_ImgmaxRegion;
	kxCImageBuf				 m_ImgGlueMask;//��������
	kxCImageBuf				 m_ImgColorGlueMask;//��������

	kxCImageBuf				 m_ImgGlueMaskFillholes;//Ϳ��������Ĥ�����׶���Ŀ������ȡ����ʱû����ȡ
	kxCImageBuf				 m_ImgColorGlueMaskFillholes;

	kxCImageBuf				 m_ImgR_Mask;

	kxCImageBuf				 m_ImgG_R;
	kxCImageBuf				 m_ImgG_B;
	kxCImageBuf				 m_ImgsubResult;

	kxCImageBuf				 m_ImgTemplateLow;//��ɫ��ģ��
	kxCImageBuf				 m_ImgTemplateHigh;//��ɫ��ģ��
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
	int						 m_nID;//��ǰ�ǵڼ���blob�����ڴ洢ȱ��ͼƬ��Ϣ
	kxRect<int>				 m_recttarget;

	kxCImageBuf				 m_Blobimg1;// ���Ͻ����
	kxCImageBuf				 m_Blobimg2;// ���Ҷȸ߽��
	kxCImageBuf				 m_Blobimg3;// ���Ҷȵͽ��
	kxCImageBuf				 m_Blobimg4;// ��� Hͨ��ɫ����

	CZSFittingLine			 m_hfitline;

	kxCImageBuf				 m_ImgEdgeGrayMask;
	kxCImageBuf				 m_ImgEdgeHMask;

//public:
//	void SaveDebugImg(const char* name, kxCImageBuf& saveimg);// ����ͼ��
//
//

private:
	void checkcolordiff(const kxCImageBuf& SrcImg);// ��ɫ��

	void checkyiwu(const kxCImageBuf& SrcImg, Json::Value &checkresult);// ����������Ͻ�

	void checkqipao(const kxCImageBuf& SrcImg); // ������

	void checkEdge(const kxCImageBuf& SrcImg);

	void GetGlueMask(const kxCImageBuf* RGB);//���ض�������ȡ��Ĥ


	void MergeImg(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg);

	void GetMaxGray(const kxCImageBuf& ColorImg, kxCImageBuf& DstGrayImg);

	void CutImg2MulImg(const kxCImageBuf& CheckImg);

	void ParallelBlob(Json::Value &checkresult);//����blob,��ͼ��Ƭ����blob

	void SearchEdge(const kxCImageBuf& GrayImg, int ndir, int nThreshvalue, int& nedge1, int& nedge2);//���������������м��������������õ�������

	void ExtractGreen(const kxCImageBuf* RGB, kxRect<int> roi, kxCImageBuf& DstImg);


	// 2022.2.5

	// ������Ե��nDir = 0�Ǵ������ѣ����ݶ�������-1 0 1��ʽ��1��֮ 
	// nDir = 2�Ǵ��µ����ѣ����ݶ�������1 0 -1��ʽ��3��֮ 
	void GetEdgePoints(const kxCImageBuf& GrayImg, int nDir, int nthresh, kxPoint<int> * searchresult, int nnums);

	// ���ⷽ������ƥ�䡣�ȹ�һ�����󻬶��Լ��������Լ��ķ�ʽҲ��Щ��ͬ
	void SliderMatch(kxCImageBuf& SrcImg, kxCImageBuf& Templateimg);

	void SliderSub(kxCImageBuf& SrcImg, kxCImageBuf& TemplateImg, kxCImageBuf& dstCCImg, int nscalefactor);

	void CheckLow(kxCImageBuf& SrcImg);//����ó��Լ����ڶϽ�


	//2022.02.28 
	void GetTargetROI(const kxCImageBuf& SrcImg, kxRect<int> rcCheck, kxRect<int>& targetrect);//�ڲ�Խ��ǰ���»�ȡ�Ӵ��ROI

	void MergeImgNew(const kxCImageBuf& SrcImg1, const kxCImageBuf& SrcImg2, kxRect<int> targetrect, kxCImageBuf& DstImg);

	void GetTargetImg(const kxCImageBuf& SrcImg, int nCheckROIW, int nCheckROIH, kxCImageBuf& DstImg, kxRect<int>& targetrect);// ��ȡ���ļ�������Լ������С

	void ExtractGreenNew(const kxCImageBuf* RGB,  kxCImageBuf& DstImg);

	void SliderMatchNew(kxCImageBuf& SrcImg, int ntargetw, int ntargeth, kxCImageBuf& Templateimg, kxCImageBuf& blobimg);

	void ModelFillholes(kxCImageBuf& SrcImg, kxCImageBuf& DstImg);

	void checkwithmodel(const kxCImageBuf& SrcImg, const kxCImageBuf& gluearea, kxCImageBuf& dsthigh, kxCImageBuf& dstlow);// ͨ��ģ�巽ʽ��ȱ���ҳ�

	void CreateBaseModel(const kxCImageBuf& SrcImg, const kxCImageBuf& gluemask, kxCImageBuf& DstHigh, kxCImageBuf& DstLow);

	int JudgeDefectType(kxCImageBuf& blobimg, kxRect<int>rect);

	//void Transhsv2check(kxCImageBuf& SrcImg, kxCImageBuf& DstHigh, kxCImageBuf& DstLow);// ��RBGת��Ϊhsv���м��

	void GetHmask(const kxCImageBuf& SrcImg, kxCImageBuf& DstMask);// ��ȡ�������H������Ĥ

	void checkwithmodelNew(const kxCImageBuf& SrcImg, const kxCImageBuf& gluearea, kxRect<int> ROI, kxCImageBuf& dsthigh, kxCImageBuf& dstlow, kxCImageBuf& dstH);// RGB ��ߵ�, H��ɫ��

	void GetMaskHL(kxCImageBuf& maskhigh, kxCImageBuf& masklow);// ��ȡ�ߵ���Ĥͼ��

	void MaskEdge(kxCImageBuf& SrcDstImg, kxRect<int> roi, int nedge);// ��Ĥ��Ե������ȥ����Ե����

	void GetEdgeCorrectionMask(const kxCImageBuf& SrcImg, kxRect<int> roi, kxCImageBuf& dstmaskgray, kxCImageBuf& dstmaskH);// ��ȡ��ԵУ���õ�ģ��
};