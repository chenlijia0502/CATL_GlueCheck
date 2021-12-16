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
	Desc:	�����࣬Ŀ������Ϊ��ⷽ���Ļ��࣬�ü̳���һ��Ҫʵ����Щ����������������ܹ����߼�

	*/
public:
	//BaseCheckMethod(){};
	//~BaseCheckMethod(){};

	//��һ������ΪͼƬ���ڶ�������Ϊ������ͼ������������Ϊ���������������ȱ�ݵ�����, json��
	virtual int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Json::Value &checkresult) { return 0; }

	virtual int Check(const kxCImageBuf& SrcImgA, const kxCImageBuf& SrcImgB, kxCImageBuf& DstImg, Json::Value &checkresult) { return 0; }

	virtual int Check(const cv::Mat& SrcImg, cv::Mat& DstImg, Json::Value &checkresult) { return 0; }

	//��һ������Ϊ����·�����ڶ�����Ϊ��������ʧ�ܷ��ص�ԭ�򣬵���������Ϊģ��ͼ·������Ϊ�еķ�������Ҫģ��ͼ��
	virtual bool ReadParamXml(const char* filePath, char *ErrorInfo, const char * templatepath = NULL) = 0;

	//�ж��Ƿ�Ϊѧϰ���״̬
	virtual bool IsLearnCompletedStatus() { return true; }

	//����ѧϰ���״̬
	virtual void SetLearnCompletedStatus(bool bStatus) {}

	virtual void SetParam(void *param) {};


};