#pragma once

#include "BaseCheckMethod.h"
#include "KxWarpStretch.h"
#include "KxBaseFunction.h"
#include "KxBlobAnalyse.h"

class CGlueCheck : public BaseCheckMethod
{
public:
	CGlueCheck();
	~CGlueCheck();

	int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult);
	bool ReadParamXml(const char* filePath, char *ErrorInfo, const char * templatepath = NULL);


private:
	CKxBaseFunction			 m_hFun;
	kxCImageBuf				 m_ImgBase;
	kxCImageBuf				 m_WarpImg;
	CKxBlobAnalyse			 m_hBlobFun;


	int						 m_nthresh;//提取异物灰度
	int						 m_nmindots;//异物最小点数
	cv::Mat					 m_matarraybgr[3];
	kxCImageBuf				 m_ImgGray;
	kxCImageBuf				 m_ImgThresh;
	kxCImageBuf				 m_ImgOpen;

	kxCImageBuf				 m_ImgRGB[3];



private:
	void checkcolordiff(const kxCImageBuf& SrcImg);


};