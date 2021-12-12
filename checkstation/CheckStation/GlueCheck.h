#pragma once

#include "BaseCheckMethod.h"
#include "KxWarpStretch.h"
#include "KxBaseFunction.h"
#include "KxBlobAnalyse.h"
#include "EmpiricaAlgorithm.h"

class CGlueCheck : public BaseCheckMethod
{
public:
	CGlueCheck();
	~CGlueCheck();

	int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult);

	bool ReadParamXml(const char* filePath, char *ErrorInfo, const char * templatepath = NULL);

	void SetParam(void *param) 
	{
		m_param = *((SingleParam*)param);
	}

	struct SingleParam
	{
		kxRect<int>		m_rcCheckROI;
		int				m_nGrabTimes;//扫描列，设备移动扫描第几组,说明当前扫描图像属于第几组
		int				m_nGrabDirection;

		int				m_ndefectthresh;
		int				m_ndefectdots;
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

	CEmpiricaAlgorithm		 m_hAlg;

	kxCImageBuf				 m_ImgmaxRegion;
	kxCImageBuf				 m_ImgGlueMask;//蓝胶掩膜
	kxCImageBuf				 m_ImgR_Mask;

	kxCImageBuf				 m_ImgG_R;
	kxCImageBuf				 m_ImgG_B;
	kxCImageBuf				 m_ImgsubResult;


private:
	void checkcolordiff(const kxCImageBuf& SrcImg);// 检色差

	void checkyiwu(const kxCImageBuf& SrcImg);// 检异物，包含断胶

	void checkqipao(const kxCImageBuf& SrcImg); // 检气泡

	void checkEdge(const kxCImageBuf& SrcImg);

	void GetGlueMask();//用特定方法提取掩膜

};