#ifndef _KXCARDHH
#define _KXCARDHH

#include "KxAlogrithm.h"
#include "global.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/partitioner.h"
#include "kxCheckResult.h"
//#include "Result.CheckResultInfo.pb.h"
#include "BaseCheckMethod.h"
#include "json.h"
#include "GlueCheck.h"
#include "CombineImg.h"

using namespace tbb;

/*!

	Date:	2020.02.27
	Author: HYH
	Desc:	检查类，在这个类进行调用检测的算法，checktool是为并行方便而设计


*/


class CKxCheck
{
public:
	CKxCheck();
	~CKxCheck();
	
	enum 
	{
		_MAX_RESULT_LEN = 5,
		_MAX_BLOB_COUNT = 128,
		_MAX_AREA_COUNT = 12,
		_MAX_SEND_DEFECTLEN = 8,
	};

	enum SaveImgStatus
	{
		_DEFAULT = 0,
		_SAVEALL = 1,
		_SAVEBAD = 2,
	};

	enum
	{
		_MAX_GROUPNUM = 32,//最多32组ROI
		
		_SPLICING_IMG_SCALEFACTOR = 10,//缩略图宽高分别压缩倍数

	};



	struct Param
	{
		CGlueCheck::SingleParam		params[_MAX_GROUPNUM];
		int				m_nROINUM;
		int				m_nimgscalefactor;//图像缩放系数
		int				m_nscantimes;//扫描次数
		int				m_nmaxrownum; // 扫描列中最多有几个列ROI

	};


private:

	kxCImageBuf					m_TransferImage;
	//kxCImageBuf				    m_ImgFourCornerWarp;
	kxCImageBuf                 m_SrcImg;

	kxCImageBuf                 m_BayerImg;

	kxCImageBuf                 m_DstImg;

	CKxBaseFunction				m_hBaseFun;
	
	BaseCheckMethod*			m_hCheckTools[_MAX_AREA_COUNT];
	Json::Value					m_hCheckResult[_MAX_AREA_COUNT];//有多少种检查方法就有多少种检查结果


	bool						m_bCheckStatus[_MAX_AREA_COUNT];//检测算法是否正常
	//CKxSparse                   m_hSparse[_AreaCount]; // 解析判断
	KxJudgeStandard			    m_hJudgeStandard[_AreaCount];
	int							m_bJudgeStatus[KxJudgeStandard::MAX_RULE_NUM][KxJudgeStandard::MAX_DEFECT_TYPE_COUNT];//一维是判断规则，二维对应所有缺陷
	int							m_nimgsavenum;
	char						m_csaveimgpath[1 << 8];
	SaveImgStatus				m_estatus;
	CheckResultStatus			m_finalcheckstatus;//每次的处理结果，这个值会用在保存坏图上
	Param						m_param;
	CCombineImg					m_hcombineimg;

	kxCImageBuf					m_ImgMaxSizeA;//把每张检测图归一化为一张最大的，放进去检测，这样不会频繁申请内存，而且结果也好发送
	kxCImageBuf					m_ImgMaxSizeB;//把每张检测图归一化为一张最大的，放进去检测，这样不会频繁申请内存，而且结果也好发送

	kxCImageBuf					m_Savebigimg[6];
	kxCImageBuf					m_ImgSplicing;//拼接图（原图压缩之后拼接一起）
	kxCImageBuf					m_ImgResize;

private:
	//转Bayer图像为彩色图像或模拟数据转换
	int TransferImage(const CKxCaptureImage& card);
	//分析结果
	int AnalyseCheckResult(int nCardID, Json::Value* checkresult);
	//清空结果
	int ClearResult(int nCardId);

	//保存图片
	void SaveImg(CheckResultStatus status);
	
	//初始检查方法
	void InitCheckMethod();
	void ReleaseCheckMethod();

	void JudgeCheckStaus(const Json::Value& checkresult, Json::Value& sendresult);// 通过表达式判定检测结果

	void DotCheckImg(const kxCImageBuf& SrcImg);

	void SaveImgToPath(const kxCImageBuf& SrcImg, Json::Value& sendresult);// 根据检测结果将数据保存到

	void CopyImg2SplicImg(const kxCImageBuf& SrcImg, int nrow, int ncol);
	
public:
	//检查一张卡片的全流程,包括预处理，处理
	int Check(const CKxCaptureImage& card);

	//获取检查工具句柄
	inline  BaseCheckMethod& GetCheckTools(int nIndex)
	{
		return *m_hCheckTools[nIndex];
	}

	int ClearIndex();//建议改成reset

	bool ReadParamXml(const char* filePath, char *ErrorInfo);

	//读取判废标准
	bool ReadReadJudgeStandardParaByXml(const char* lpszFile, int nIndex);
	bool ReadReadJudgeStandardParaByXmlinChinese(const char* lpszFile, int nIndex);
	bool ReadReadJudgeStandardParaByXmlinEnglish(const char* lpszFile, int nIndex);
	

	void SetSaveStatus(SaveImgStatus status, char* savepath=NULL);//HYH 2020.02.15 设置存图状态，也即存取什么图。
};

extern  CKxCheck   g_CheckObj;
namespace  Check
{
	inline CKxCheck &g_GetCheckCardObj()
	{
		return  g_CheckObj;
	}
}

#endif