#pragma once

#include "BaseCheckMethod.h"
#include "KxWarpStretch.h"
#include "KxBaseFunction.h"
#include "KxBlobAnalyse.h"

class CCalcuPlaneness : public BaseCheckMethod
{
public:
	CCalcuPlaneness();
	~CCalcuPlaneness();

	int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult) { return 0; };
	
	bool ReadParamXml(const char* filePath, char *ErrorInfo, const char * templatepath = NULL) { return false; };

	int Check(const cv::Mat& SrcImg, cv::Mat& DstImg, Json::Value &checkresult);



private:
	CKxBaseFunction			 m_hFun;
	kxCImageBuf				 m_ImgBase;
	kxCImageBuf				 m_WarpImg;
	CKxBlobAnalyse			 m_hBlobFun;

private:
	void CalcuSurfacePlaness(const cv::Mat& points, cv::Mat& plane);

};