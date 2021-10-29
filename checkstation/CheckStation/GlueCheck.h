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

	void SetParam(void *param) 
	{
		m_param = *((SingleParam*)param);
	}

	struct SingleParam
	{
		kxRect<int>		m_rcCheckROI;
		int				m_nGrabTimes;//ɨ���У��豸�ƶ�ɨ��ڼ���,˵����ǰɨ��ͼ�����ڵڼ���
		int				m_nGrabDirection;
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
	kxCImageBuf				 m_ImgOpen;

	kxCImageBuf				 m_ImgRGB[3];



private:
	void checkcolordiff(const kxCImageBuf& SrcImg);// ��ɫ��

	void checkyiwu(const kxCImageBuf& SrcImg);// ����������Ͻ�

	void checkqipao(const kxCImageBuf& SrcImg); // ������

	void checkEdge(const kxCImageBuf& SrcImg);



};