#pragma once

#include "KxAlogrithm.h"
#include "ipps.h"
#include <map>
#include <string>
#include "opencv2/opencv.hpp"
#include "json.h"


class BaseCheckMethod
{
	/*!
	Date:	2020.02.15
	Author:	HYH
	Desc:	抽象类，目的是作为检测方法的基类，让继承类一定要实现这些抽象函数，完成整个架构的逻辑

	*/
public:
	//BaseCheckMethod(){};
	//~BaseCheckMethod(){};

	//第一个参数为图片，第二个参数为处理结果图，第三个参数为检测结果参数（比如缺陷点数等, json）
	virtual int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult) { return 0; }

	virtual int Check(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg, Json::Value &checkresult) { return 0; }

	virtual int Check(const cv::Mat& SrcImg, cv::Mat& DstImg, Json::Value &checkresult) { return 0; }

	//第一个参数为配置路径，第二参数为读参数是失败返回的原因，第三个参数为模板图路径，因为有的方法不需要模板图；
	virtual bool ReadParamXml(const char* filePath, char *ErrorInfo, const char * templatepath = NULL) = 0;

	//判断是否为学习完成状态
	virtual bool IsLearnCompletedStatus() { return true; }

	//设置学习完成状态
	virtual void SetLearnCompletedStatus(bool bStatus) {}

	virtual void SetParam(void *param) {};


};