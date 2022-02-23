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
		int				m_nGrabTimes;//ɨ���У��豸�ƶ�ɨ��ڼ���,˵����ǰɨ��ͼ�����ڵڼ���
		int				m_nCurGrabID;// ��ǰɨ���еڼ���roi
		int				m_nIsnotcheck;//�Ƿ����ε�ǰ���


		int				m_ndefectthresh;
		int				m_ndefectdots;//ȱ�ݵ���

		int				m_noffsethigh;//��������ƫ�ƣ�����վ��ȡ���ֱ�ӳ���25��Ŀ����ͳһ��255�Ļҽ�
		int				m_noffsetlow;//��������ƫ�ƣ�����վ��ȡ���ֱ�ӳ���25��Ŀ����ͳһ��255�Ļҽ�

		kxCImageBuf		m_ImgTemplate;

	};

	enum
	{
		_MAX_BLOBIMG = 30,// �����ͨ���������
		_SINGLE_BLOBIMG_H = 2000,//ÿ�����������ͼ���С
		_IMG_OVERLAP = 20, // ͼ���ص���С
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
	kxCImageBuf				 m_ImgGlueMask;//������Ĥ
	kxCImageBuf				 m_ImgColorGlueMask;//������Ĥ

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

	kxCImageBuf				 m_ImgCheck;

	kxCImageBuf				 m_ImgSrcA;
	kxCImageBuf				 m_ImgSrcB;

	kxCImageBuf				 m_pBLOBIMG[_MAX_BLOBIMG];
	CKxBlobAnalyse			 m_phblob[_MAX_BLOBIMG];
	int						 m_nblobimgnum;

	kxCImageBuf				 m_ImgZero2split;
	int						 m_nID;//��ǰ�ǵڼ���blob�����ڴ洢ȱ��ͼƬ��Ϣ
	kxRect<int>				 m_recttarget;


private:
	void checkcolordiff(const kxCImageBuf& SrcImg);// ��ɫ��

	void checkyiwu(const kxCImageBuf& SrcImg, Json::Value &checkresult);// ����������Ͻ�

	void checkqipao(const kxCImageBuf& SrcImg); // ������

	void checkEdge(const kxCImageBuf& SrcImg);

	void GetGlueMask(const kxCImageBuf* RGB);//���ض�������ȡ��Ĥ

	void CreateBaseModel(const kxCImageBuf& CheckImg);//��������ģ�壬��ɫ��ģ�壬��ɫ��ģ��

	void MergeImg(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg);

	void GetMaxGray(const kxCImageBuf& ColorImg, kxCImageBuf& DstGrayImg);

	void CutImg2MulImg(const kxCImageBuf& CheckImg);

	void ParallelBlob(Json::Value &checkresult);//����blob,��ͼ��Ƭ����blob

	void SearchEdge(const kxCImageBuf& GrayImg, int ndir, int nThreshvalue, int& nedge1, int& nedge2);//���������������м��������������õ�������

	void ExtractGreen(const kxCImageBuf* RGB, kxRect<int> roi, kxCImageBuf& DstImg);


	// 2022.2.5

	// ������Ե��nDir = 0�Ǵ����ң����ݶ�������-1 0 1��ʽ��1��֮ 
	void GetEdgePoints(const kxCImageBuf& GrayImg, int nDir, int nthresh, kxPoint<int> * searchresult, int nnums);

	// ���ⷽ������ƥ�䡣�ȹ�һ�����󻬶��Լ��������Լ��ķ�ʽҲ��Щ��ͬ
	void SliderMatch(kxCImageBuf& SrcImg, kxCImageBuf& Templateimg);

	void SliderSub(kxCImageBuf& SrcImg, kxCImageBuf& TemplateImg, kxCImageBuf& dstCCImg, int nscalefactor);


	void CheckLow(kxCImageBuf& SrcImg);//����ó��Լ����ڶϽ�

};