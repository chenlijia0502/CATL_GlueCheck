//电池极片边缘检查
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
		int    m_nPolepiceType;				//极片类型：正0负1
		int    m_nSearchEdgeThresh;         //搜边亮度
		int	   m_nBrokenThresh;				//断线检查亮度
		int    m_nArchThresh;				//拱检查亮度
		int    m_nWhiteLineThresh;          //毛刺（也即集流体）检查亮度
		int    m_nOtherAreaThresh;          //铝丝检查亮度
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




	enum//对于寻找极片所在位置所需
	{
		_CROP_HEIGHT = 400,
		_EDGE_HEIGHT = 60,  //假定边高60个像素
		_EDGE_EXTEND = 70, //上下扩70
	};

	enum 
	{
		_UPER = 0,    //白线上
		_MID = 1,    //白线
		_DOWN = 2,    //白线下

		_OK      = 0,     //正常
		_BLOBERR = 1,     //连通域错误

		_WHITELINE_HEIGHT = 11,  //白线高11个像素
		_BROKEN_LEN = 10,   //细线有时会断，这个表示可接受范围,这个是最低下限，跟界面设置的那个不一样
		_POLEPIECE_HEIGTH = 55, //极片有数据是110um

		_MARK_DILATE = 2,
	};

	//-------------- HYH ----------------//

	enum//正极各种缺陷的个数
	{
		_MAX_DEFECT_COUNT = 32,  //最大缺陷个数
		_POLEPIECE_MEASURE_HEIGHT = 1,//用一个来记录厚度
		_WHITELINE_POS = 1,//用一个来表示
		_MAOCI_MAX = 2, //最多只记录这么多个可能毛刺
		_BROKEN_MAX = 8, //最多只记录这么多个断的区域
		_ARCH = 4, //最多只记录这么多个拱
		_DIRTY = 8, //最多记录这么多脏东西
	};

	enum//特别的标志
	{
		_RATE_GLITCH_OR_ARCH = 2, // 固定的标志，当rate小于等于2时，是毛刺；当rate大于2时，是拱
		_GLITCH_ARCH_SMALLEST = 10,//毛刺、拱最短，这是根据经验的评估
		_GLITCH_LARGETEST = 15, //毛刺最多这么宽

		//负极
		_MIN_MINAREA_LW_RATIO = 2, // 最小外接矩阵的宽高比能接受的最小值
		_MIN_ANGLE = 30, //最小外接矩阵的倾斜角范围（算的是长的那条边与水平方向的夹角）
		_MAX_ANGLE = 150,
		_NEGATIVE_GLITCH_MIN = 5, // 负极毛刺最短，单位像素
		_NEGATIVE_GLITCH_THICK_MAX = 10, //负极毛刺是很细小的尖锐，所以设置厚度收到限制，单位像素
	};

	enum
	{
		ZHENG = 0,
		FU = 1,
	};

	enum DEFECT_FLAG//标志位，0是特指断的区域，1是线上的，2是其它区域，3是指极片厚度，4是指白线比例, 5是异常，6是图像掉粉占比
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
			nCheckStatus = 0;  //表示正常检查
			szErrInfo = "";
			nDefectCount = 0;
			mapFeaturelists.clear();
			mapFeaturelists.resize(_MAX_DEFECT_COUNT);
			for (int i = 0; i < _MAX_DEFECT_COUNT; i++)
			{
				mapFeaturelists[i].push_back(std::pair<std::string, float>("点数", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("能量", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("X坐标", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("Y坐标", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("缺陷宽", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("缺陷高", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("中心占比率", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("物理高度", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("标志位", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("物理宽度", 0.0f));

			}
		}

		void Clear()
		{
			nCheckStatus = 0;  //表示正常检查
			szErrInfo = "";
			nDefectCount = 0;
			//mapFeaturelists.clear();
			//mapFeaturelists.resize(_MAX_DEFECT_COUNT);		
			for (int i = 0; i < _MAX_DEFECT_COUNT; i++)
			{
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("点数", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("能量", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("X坐标", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("Y坐标", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("缺陷宽", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("缺陷高", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("位置", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("边宽", 0.0f));
				//mapFeaturelists[i].push_back(std::pair<std::string, float>("标志位", 0.0f));
				mapFeaturelists[i][0] = std::make_pair("点数", 0.0f);
				mapFeaturelists[i][1] = std::make_pair("能量", 0.0f);
				mapFeaturelists[i][2] = std::make_pair("X坐标", 0.0f);
				mapFeaturelists[i][3] = std::make_pair("Y坐标", 0.0f);
				mapFeaturelists[i][4] = std::make_pair("缺陷宽", 0.0f);
				mapFeaturelists[i][5] = std::make_pair("缺陷高", 0.0f);
				mapFeaturelists[i][6] = std::make_pair("中心占比率", 0.0f);
				mapFeaturelists[i][7] = std::make_pair("物理高度", 0.0f);
				mapFeaturelists[i][8] = std::make_pair("标志位", 0.0f);
				mapFeaturelists[i][9] = std::make_pair("物理宽度", 0.0f);


			}		
		}
		int                                        nCheckStatus;    //检查状态
		std::string                                szErrInfo;       //备注信息
		int                                        nEdgeStart;      //线起始
		int                                        nEdgeEnd;
		int                                        nWhiteLinePos;   //白线的位置
		int                                        nDefectCount;    //缺陷个数
		std::vector<std::vector<std::pair<std::string, float>>>  mapFeaturelists; //每个缺陷的特征列表
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


	//-------------负极区域----------------//
	kxCImageBuf	      m_SmallCutImg;
	kxCImageBuf	      m_SmallCenterImg;
	kxCImageBuf		  m_SmallThreshImg;
	kxCImageBuf		  m_SmallCenterRc;

	kxCImageBuf		  m_BigBlobImg;
	kxCImageBuf		  m_BigBlobNoMaskImg;//未被掩模的blob
	kxCImageBuf		  m_SmallImgArray[_CUT_NUM];

	std::vector<cv::Vec4i>					 m_hierarchy;
	std::vector<std::vector<cv::Point> >	 m_contours;

	// -------- 2019/03/13 : 拱与毛刺分检 ----------//
	//cv::Mat			  m_MatPlateThreshImg;


private:
	bool FiltLine(Ipp32f* pX, Ipp32f* pY, int n, float& fAngle, float& a, float& b);
	bool FiltLine2D(kxPoint<float>* pts, int n, float& fAngle, float& a, float& b);
	void combine(int* arr, int start, int* result, int count, int num, int& nTotal, int** Result);
	float FitLineByRansac(float* pArray, int nArrayStart, int nLen, int nSampleCount, int nFitLineDots, float& fBestAngle, float& fBesta, float& fBestb);
	//检查白线完整性
	int CheckWhiteLine(const kxCImageBuf& SrcImg, KxCallStatus& hCall);
	int CheckOtherArea(const kxCImageBuf& SrcImg, KxCallStatus& hCall);

	// ---- 2018/12/27 ------
	int ConfirmArch(const kxCImageBuf& SrcImg, KxCallStatus& hCall);
	int MoveFrontFix(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall);
	void sortbroken(std::vector<Broken> brokenrecord, int* resultidx, int neednum);
	float getrate(Ipp32f* whitelineproject, int len, int nstart, int nend, int blobarea);
	void mergeidx(cv::Mat listidx1, int glitchnum, cv::Mat listidx2, int archnum, int* result, int &finallen);
	float calculatedis(kxCImageBuf singleblobimg, int x, int y, int width, int height, int blobcentery, int blobheight);

	//------ 2019/02/27 : 整理程序 -----------//
	int PreSolve(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, kxRect<int>& rcEdge, KxCallStatus& hCall);
	int FitlineAndRotateImg(const kxCImageBuf& SrcImg, kxCImageBuf& CropImg, kxRect<int> rcEdge, double &disT, double &disB, KxCallStatus& hCall);

	int BrokenSolve(const kxCImageBuf& SrcImg, kxCImageBuf& WhiteLineArea, kxCImageBuf& DstImgClose, int &middleindex, Ipp32f* midrecords);
	int WhiteLineSolve(kxCImageBuf& WhiteLineArea, int middleindex, Ipp32f* midrecords, int& averageup, int& averagedown);
	int AnotherSolve(kxCImageBuf&SrcImg, kxCImageBuf& WhitelineImg, int averageup, int averagedown);
	
	// --------- 2019/03/05 : 求取厚度 -----------//
	int PlateHeightSolve(const kxCImageBuf& CropImg, int middle);

	// -------- 2019/03/13 : 拱与毛刺分检 ----------//
	int MoveFrontFix2(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall);
	int BrokenSolve2(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int&middleindex, KxCallStatus& hCall);
	int WhiteLineSolve2(const kxCImageBuf& SrcImg, kxCImageBuf& whitelinearea, std::vector <RectBlob>& vec_blob, int middleidx, KxCallStatus& hCall);
	int GetMid(Ipp32f* pProject, int len, int selectnum);
	int AnotherSolve2(const kxCImageBuf& SrcImg, const kxCImageBuf& MaskImg, std::vector <RectBlob>& vec_blob, int middleidx, KxCallStatus& hCall);

	// ---------- 2019/03/27 : 对拱跟毛刺的区分进一步强化 ------------//
	float getrate2(const kxCImageBuf& WhitelineArea, int heightmid, int nstart, int nend);


	// 负极程序
	int MoveFrontCut(const kxCImageBuf& SrcImg, int &middle, KxCallStatus& hCall);
	int AnalyseSmallBlob(const kxCImageBuf& SrcImg, KxCallStatus& hCall);

	// 负极新光源
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


	
	//检查图像
	int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	//读取参数
	bool ReadParaXml(const char* filePath);
	//获取结果
	Result& GetResult()
	{
		return m_hResult;
	}
};



#endif