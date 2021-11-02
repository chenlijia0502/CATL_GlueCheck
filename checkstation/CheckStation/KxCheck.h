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
		_SOLVE_IMG_HEIGHT = 2000, //单次处理图像大小
		_SOLVE_IMG_OVERLAY = 50,  //处理图像的重叠区域
	};



	struct Param
	{
		CGlueCheck::SingleParam		params[_MAX_GROUPNUM];
		int				m_nROINUM;
	};

	struct PrepareToCheck
	{
		void clear()
		{
			m_ncopyrow = 0;
			m_nsolverowstart = 0;
			m_bCanCheck = false;
		}

		bool GetSolveImg(kxCImageBuf& dstimg)
		{
			//从m_BigImg中裁切 _SOLVE_IMG_HEIGHT 行图像，用于单次处理
			if (m_nsolverowstart >= m_BigImg.nHeight - 1)
			{
				return false;//已经拷贝最后一行
			}
			else
			{
				if (m_nsolverowstart + _SOLVE_IMG_HEIGHT < m_BigImg.nHeight)
				{
					dstimg.SetImageBuf(m_BigImg.buf + m_nsolverowstart * m_BigImg.nPitch, m_BigImg.nWidth, 
						_SOLVE_IMG_HEIGHT, m_BigImg.nPitch, m_BigImg.nChannel, false);
				}
				else
				{
					dstimg.Init(m_BigImg.nWidth, _SOLVE_IMG_HEIGHT, m_BigImg.nChannel);

					IppiSize roisize = { m_BigImg.nWidth, _SOLVE_IMG_HEIGHT };

					Ipp8u zeroarray[3] = { 0, 0, 0 };

					ippiSet_8u_C3R(zeroarray, dstimg.buf, dstimg.nPitch, roisize);

					IppiSize copysize = { m_BigImg.nWidth, m_BigImg.nHeight - m_nsolverowstart - 1 };

					ippiCopy_8u_C3R(m_BigImg.buf + m_nsolverowstart * m_BigImg.nPitch, m_BigImg.nPitch, dstimg.buf, 
						dstimg.nPitch, copysize);

				}

				m_nsolverowstart += (_SOLVE_IMG_HEIGHT - _SOLVE_IMG_OVERLAY);// 处理行号往前递进，同时减掉重叠路径


				
			}

		}
		
		kxCImageBuf		m_BigImg;
		int				m_ncopyrow;	//拷贝行
		int				m_nsolverowstart;//处理起始行

		bool			m_bCanCheck;//是否有图可以检测

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
	PrepareToCheck				m_struct2check[_MAX_GROUPNUM];

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

	void JudgeWhichROI(const CKxCaptureImage& SrcCapImg);

	
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