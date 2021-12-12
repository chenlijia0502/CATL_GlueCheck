//��ؼ�Ƭ��Ե���
//add by lyl 2017/11/1
#ifndef _KXPLATEEDGECHECKH
#define _KXPLATEEDGECHECKH

#include "KxAlogrithm.h"
#include "ipps.h"
#include <map>
#include <string>
#include "opencv2/opencv.hpp"

class CKxPlateEdgeCheck
{
public:
	CKxPlateEdgeCheck();
	~CKxPlateEdgeCheck();

	struct  Parameter
	{
		Parameter()
		{
			m_nPolepiceType = 0;
			m_nSearchEdgeThresh = 20;
			m_nBrokenThresh = 60;
			m_nArchThresh = 60;
			m_nWhiteLineThresh = 50;
			m_nOtherAreaThresh = 60;
		}
		int    m_nPolepiceType;				//��Ƭ���ͣ���0��1
		int    m_nSearchEdgeThresh;         //�ѱ�����
		int	   m_nBrokenThresh;				//���߼������
		int    m_nArchThresh;				//���������
		int    m_nWhiteLineThresh;          //ë�̣�Ҳ�������壩�������
		int    m_nOtherAreaThresh;          //��˿�������
	};

	struct Broken
	{
		Broken();
		Broken(int a, int b)
		{
			nstart = a;
			nend = b;
			nlen = nend - nstart + 1;
		}

		int nstart;
		int nend;
		int nlen;
	};

	struct RectBlob
	{
		RectBlob(int x1, int y1, int w, int h)
		{
			x = x1;
			y = y1;
			width = w;
			height = h;
		}
		int x;
		int y; 
		int width;
		int height;
	};




	enum//����Ѱ�Ҽ�Ƭ����λ������
	{
		_CROP_HEIGHT = 400,
		_EDGE_HEIGHT = 60,  //�ٶ��߸�60������
		_EDGE_EXTEND = 70, //������70
	};

	enum 
	{
		_UPER = 0,    //������
		_MID = 1,    //����
		_DOWN = 2,    //������

		_OK      = 0,     //����
		_BLOBERR = 1,     //��ͨ�����

		_WHITELINE_HEIGHT = 11,  //���߸�11������
		_BROKEN_LEN = 10,   //ϸ����ʱ��ϣ������ʾ�ɽ��ܷ�Χ,�����������ޣ����������õ��Ǹ���һ��
		_POLEPIECE_HEIGTH = 55, //��Ƭ��������110um

		_MARK_DILATE = 2,
	};

	//-------------- HYH ----------------//

	enum//��������ȱ�ݵĸ���
	{
		_MAX_DEFECT_COUNT = 32,  //���ȱ�ݸ���
		_POLEPIECE_MEASURE_HEIGHT = 1,//��һ������¼���
		_WHITELINE_POS = 1,//��һ������ʾ
		_MAOCI_MAX = 2, //���ֻ��¼��ô�������ë��
		_BROKEN_MAX = 8, //���ֻ��¼��ô����ϵ�����
		_ARCH = 4, //���ֻ��¼��ô�����
		_DIRTY = 8, //����¼��ô���ණ��
	};

	enum//�ر�ı�־
	{
		_RATE_GLITCH_OR_ARCH = 2, // �̶��ı�־����rateС�ڵ���2ʱ����ë�̣���rate����2ʱ���ǹ�
		_GLITCH_ARCH_SMALLEST = 10,//ë�̡�����̣����Ǹ��ݾ��������
		_GLITCH_LARGETEST = 15, //ë�������ô��

		//����
		_MIN_MINAREA_LW_RATIO = 2, // ��С��Ӿ���Ŀ�߱��ܽ��ܵ���Сֵ
		_MIN_ANGLE = 30, //��С��Ӿ������б�Ƿ�Χ������ǳ�����������ˮƽ����ļнǣ�
		_MAX_ANGLE = 150,
		_NEGATIVE_GLITCH_MIN = 5, // ����ë����̣���λ����
		_NEGATIVE_GLITCH_THICK_MAX = 10, //����ë���Ǻ�ϸС�ļ����������ú���յ����ƣ���λ����
	};

	enum
	{
		ZHENG = 0,
		FU = 1,
	};

	enum DEFECT_FLAG//��־λ��0����ָ�ϵ�����1�����ϵģ�2����������3��ָ��Ƭ��ȣ�4��ָ���߱���, 5���쳣��6��ͼ�����ռ��
	{
		BROKE = 0,
		LINE = 1,
		OTHERAREA = 2,
		THICKNESS = 3,
		SCALE = 4,
		ABNORMAL = 5,
		DIAOFENRATE = 6,
	};

	enum
	{
		_CUT_NUM = 5,
	};


	struct  Result
	{
		Result()
		{
			Init();
		}

		void Init()
		{
			nCheckStatus = 0;  //��ʾ�������
			szErrInfo = "";
			nDefectCount = 0;
			mapFeaturelists.clear();
			mapFeaturelists.resize(_MAX_DEFECT_COUNT);
			for (int i = 0; i < _MAX_DEFECT_COUNT; i++)
			{
				mapFeaturelists[i].push_back(std::pair<std::string, float>("����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("X����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("Y����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("ȱ�ݿ�", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("ȱ�ݸ�", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("����ռ����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("����߶�", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("��־λ", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("������", 0.0f));

			}
		}

		void Clear()
		{
			nCheckStatus = 0;  //��ʾ�������
			szErrInfo = "";
			nDefectCount = 0;
			//mapFeaturelists.clear();
			//mapFeaturelists.resize(_MAX_DEFECT_COUNT);		
			for (int i = 0; i < _MAX_DEFECT_COUNT; i++)
			{
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("����", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("����", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("X����", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("Y����", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("ȱ�ݿ�", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("ȱ�ݸ�", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("λ��", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("�߿�", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("��־λ", 0.0f));
				mapFeaturelists[i][0] = std::make_pair("����", 0.0f);
				mapFeaturelists[i][1] = std::make_pair("����", 0.0f);
				mapFeaturelists[i][2] = std::make_pair("X����", 0.0f);
				mapFeaturelists[i][3] = std::make_pair("Y����", 0.0f);
				mapFeaturelists[i][4] = std::make_pair("ȱ�ݿ�", 0.0f);
				mapFeaturelists[i][5] = std::make_pair("ȱ�ݸ�", 0.0f);
				mapFeaturelists[i][6] = std::make_pair("����ռ����", 0.0f);
				mapFeaturelists[i][7] = std::make_pair("����߶�", 0.0f);
				mapFeaturelists[i][8] = std::make_pair("��־λ", 0.0f);
				mapFeaturelists[i][9] = std::make_pair("������", 0.0f);


			}		
		}
		int                                        nCheckStatus;    //���״̬
		std::string                                szErrInfo;       //��ע��Ϣ
		int                                        nEdgeStart;      //����ʼ
		int                                        nEdgeEnd;
		int                                        nWhiteLinePos;   //���ߵ�λ��
		int                                        nDefectCount;    //ȱ�ݸ���
		std::vector<std::vector<std::pair<std::string, float>>>  mapFeaturelists; //ÿ��ȱ�ݵ������б�
	};


protected:
	CKxBaseFunction   m_hBaseFun;
	CKxBlobAnalyse    m_hBlobAnalyse;
	CKxBlobAnalyse    m_hBlobAnalyseResult;
	CKxGradientProcess m_hGrad;
	Parameter         m_hPara;
	Result            m_hResult;
	kxCImageBuf       m_GrayImg, m_BinaryImg, m_CloseImg, m_CropImg;
	kxCImageBuf		  m_CropImgResize;
	kxCImageBuf       m_EdgeRegionImg, m_TmpImg2, m_MarkImg;
	kxCImageBuf       m_SmoothImg;
	kxCImageBuf       m_WhiteLineBinaryImg, m_WhiteLineCloseImg, m_WhiteImg;
	kxCImageBuf       m_EdgeAreaImg, m_RotateImg;
	kxCImageBuf       m_OtherAreaImg;
	int               m_nWhiteLineVerPos;
	int               m_nWhiteLineStart, m_nWhiteLineEnd;
	
	kxCImageBuf		  m_ArchthreshImg;
	kxCImageBuf		  m_ArcherodeImg;
	kxCImageBuf		  m_PlateThreshImg;
	kxCImageBuf		  m_PlateCenterArea;
	kxCImageBuf		  m_PlateThreshImgClose;
	kxCImageBuf		  m_PlateClone;
	cv::Mat 		  m_MatPlateThreshImg;
	cv::Mat			  m_MatPlateThreshImgLabel32S;
	cv::Mat			  m_MatWhitelineArea;
	cv::Mat			  m_PlateArea0;
	cv::Mat			  m_PlateArea1;
	cv::Mat			  m_PlateArea2;
	kxCImageBuf		  m_PlateArea2kxImg;
	kxCImageBuf		  m_Whitelinearea;
	kxCImageBuf		  m_WhitelineareaClose;
	kxCImageBuf		  m_WhitelineareaClone;
	kxCImageBuf		  m_PlateOtherArea;
	kxCImageBuf		  m_WhitelineareaCloneDilate;
	kxCImageBuf		  m_maskImg;
	kxCImageBuf		  m_CropThreshImg;
	kxCImageBuf		  m_BrokenAreaImg;
	kxCImageBuf       m_GlitchAreaImg;
	kxCImageBuf       m_ArchAreaImg;
	kxCImageBuf		  m_ArchCloseImg;
	kxCImageBuf		  m_RidofImg;
	kxCImageBuf       mDefectMarkImg;
	kxCImageBuf       m_AnotherAreaWhiteImg;
	kxCImageBuf		  m_AnotherThreshImg;
	kxCImageBuf		  m_AnotherAreaImg;
	kxCImageBuf		  m_GlitchThreshImg;
	kxCImageBuf		  m_MaskDilateImg;
	kxCImageBuf		  m_AnotherThreshCloseImg;
	kxCImageBuf		  m_AnotherThreshOpenImg;
	kxCImageBuf		  m_WhiteLineMask;
	kxCImageBuf		  m_SingleMid;


	//-------------��������----------------//
	kxCImageBuf	      m_SmallCutImg;
	kxCImageBuf	      m_SmallCenterImg;
	kxCImageBuf		  m_SmallThreshImg;
	kxCImageBuf		  m_SmallCenterRc;

	kxCImageBuf		  m_BigBlobImg;
	kxCImageBuf		  m_BigBlobNoMaskImg;//δ����ģ��blob
	kxCImageBuf		  m_SmallImgArray[_CUT_NUM];

	std::vector<cv::Vec4i>					 m_hierarchy;
	std::vector<std::vector<cv::Point> >	 m_contours;

	// -------- 2019/03/13 : ����ë�̷ּ� ----------//
	//cv::Mat			  m_MatPlateThreshImg;


private:
	bool FiltLine(Ipp32f* pX, Ipp32f* pY, int n, float& fAngle, float& a, float& b);
	bool FiltLine2D(kxPoint<float>* pts, int n, float& fAngle, float& a, float& b);
	void combine(int* arr, int start, int* result, int count, int num, int& nTotal, int** Result);
	float FitLineByRansac(float* pArray, int nArrayStart, int nLen, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb);
	//������������
	int CheckWhiteLine(const kxCImageBuf& SrcImg, KxCallStatus& hCall);
	int CheckOtherArea(const kxCImageBuf& SrcImg, KxCallStatus& hCall);

	// ---- 2018/12/27 ------
	int ConfirmArch(const kxCImageBuf& SrcImg, KxCallStatus& hCall);
	int MoveFrontFix(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall);
	void sortbroken(std::vector<Broken> brokenrecord, int* resultidx, int neednum);
	float getrate(Ipp32f* whitelineproject, int len, int nstart, int nend, int blobarea);
	void mergeidx(cv::Mat listidx1, int glitchnum, cv::Mat listidx2, int archnum, int* result, int &finallen);
	float calculatedis(kxCImageBuf singleblobimg, int x, int y, int width, int height, int blobcentery, int blobheight);

	//------ 2019/02/27 : ������� -----------//
	int PreSolve(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, kxRect<int>& rcEdge, KxCallStatus& hCall);
	int FitlineAndRotateImg(const kxCImageBuf& SrcImg, kxCImageBuf& CropImg, kxRect<int> rcEdge, double &disT, double &disB, KxCallStatus& hCall);

	int BrokenSolve(const kxCImageBuf& SrcImg, kxCImageBuf& WhiteLineArea, kxCImageBuf& DstImgClose, int &middleindex, Ipp32f* midrecords);
	int WhiteLineSolve(kxCImageBuf& WhiteLineArea, int middleindex, Ipp32f* midrecords, int& averageup, int& averagedown);
	int AnotherSolve(kxCImageBuf&SrcImg, kxCImageBuf& WhitelineImg, int averageup, int averagedown);
	
	// --------- 2019/03/05 : ��ȡ��� -----------//
	int PlateHeightSolve(const kxCImageBuf& CropImg, int middle);

	// -------- 2019/03/13 : ����ë�̷ּ� ----------//
	int MoveFrontFix2(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall);
	int BrokenSolve2(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int&middleindex, KxCallStatus& hCall);
	int WhiteLineSolve2(const kxCImageBuf& SrcImg, kxCImageBuf& whitelinearea, std::vector <RectBlob>& vec_blob, int middleidx, KxCallStatus& hCall);
	int GetMid(Ipp32f* pProject, int len, int selectnum);
	int AnotherSolve2(const kxCImageBuf& SrcImg, const kxCImageBuf& MaskImg, std::vector <RectBlob>& vec_blob, int middleidx, KxCallStatus& hCall);

	// ---------- 2019/03/27 : �Թ���ë�̵����ֽ�һ��ǿ�� ------------//
	float getrate2(const kxCImageBuf& WhitelineArea, int heightmid, int nstart, int nend);


	// ��������
	int MoveFrontCut(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall);
	int AnalyseSmallBlob(const kxCImageBuf& SrcImg, KxCallStatus& hCall);

	// �����¹�Դ
	int MoveFrontCut2(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall);


public:

	void KxImageBufToMat(const kxCImageBuf& SrcImg, cv::Mat& DstImg);
	void MatToKxImageBuf(cv::Mat& SrcImg, kxCImageBuf&DstImg);
	int GetCenterROI(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int threshvalue, int &middleindex, KxCallStatus& hCall, Ipp32f *midrecord = NULL, int offset=2);
	int NestingVersus(kxCImageBuf& SrcDstImg, std::vector <RectBlob>& vec_blob);
	cv::Point CKxPlateEdgeCheck::GetSeedPoint(kxCImageBuf& SrcDstImg, RectBlob& vec_blob);

	int FloodfilltoGetROI(kxCImageBuf& SrcImg, kxCImageBuf& DstImg, IppiPoint& seed);
	int CalcuMin_Variance(int data[], int len);

	double calcLineDegree(const cv::Point2f& firstPt, const cv::Point2f& secondPt);
	double getRcDegree(const cv::Point2f vertVect[4]);
	cv::Rect getboundRect(const cv::Point2f vertVect[4], int nmaxwidth, int nmaxheight);


	
	//���ͼ��
	int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	//��ȡ����
	bool ReadParaXml(const char* filePath);
	//��ȡ���
	Result& GetResult()
	{
		return m_hResult;
	}
};



#endif