#pragma  once
//#include "KxIdentify.h"
#include "KxAlogrithm.h"
//#include "KxSparse.h"
#include "SaveQue.h"
#include "global.h"
#pragma pack(push,1)

struct KxJudgeStandard
{
	enum 
	{
		MAX_DEFECT_TYPE_COUNT = 128,
		MAX_RULE_LENGTH = 128,
		MAX_ERR_LENGTH = 64,
		MAX_RULE_NUM = 12,//最多有这么多规则
	}; 
	KxJudgeStandard()
	{
		strcpy_s(m_szVersion, _VersionNameLen, "JudgeStandard1.0");
		//m_nRuleCount = 0;
		//memset(m_nMode, 0, sizeof(int)*MAX_DEFECT_TYPE_COUNT);
		//memset(m_nDefectCount, 0, sizeof(int)*MAX_DEFECT_TYPE_COUNT);
		//for (int i = 0; i < MAX_DEFECT_TYPE_COUNT; i++)
		//{
		//	strcpy_s(m_szErrName[i], MAX_ERR_LENGTH, "");
		//	strcpy_s(m_szRules[i], MAX_RULE_LENGTH, "");
		//}
		m_nRuleCount = 1;
		m_nDefectCount[0] = 1;
		strcpy_s(m_szErrName[0], MAX_ERR_LENGTH, "Err1");
		strcpy_s(m_szRules[0], MAX_RULE_LENGTH, "x[0] > 10");
		memset(m_nAlarmMode, _Check_Err, sizeof(int)*MAX_DEFECT_TYPE_COUNT);
	}

    char m_szVersion[_VersionNameLen];  //记录参数的版本信息
	int  m_nRuleCount;
	int  m_nMode[MAX_DEFECT_TYPE_COUNT];
	char m_szErrName[MAX_DEFECT_TYPE_COUNT][MAX_ERR_LENGTH]; 
	char m_szRules[MAX_DEFECT_TYPE_COUNT][MAX_RULE_LENGTH]; 
	int  m_nDefectCount[MAX_DEFECT_TYPE_COUNT];

	int  m_nAlarmMode[MAX_DEFECT_TYPE_COUNT];//2019.03.07 报错方式

};

struct kxErrorMsg
{
	enum
	{
		_FEATURE_COUNT = 12,//最多12个特征 2020.08.04
	};
	kxErrorMsg()
	{
		m_ErrImg.Init(10, 10);
		memset(m_nFeaturelist, 0, sizeof(int)*_FEATURE_COUNT);

	}
	int       m_nFeaturelist[_FEATURE_COUNT];

	kxCImageBuf  m_ErrImg;

};







class kxCheckError               //检测错误
{
public:
	kxCheckError();
	~kxCheckError();

	enum
	{
		_ERR_COUNT = 100,    //根据需要修改[*******************]      
	};


public:
	static int     GetErrMaxCount() { return _ERR_COUNT; }
	int            GetErrCount()	{ return m_nErrCount;}
	kxErrorMsg&    GetErrMsg( int nIndex ) {	return m_checkErr[ nIndex ];}
    const kxErrorMsg& GetErrMsg( int nIndex ) const {	return m_checkErr[ nIndex ]; }
	kxErrorMsg& GetCurMsg(){return  m_checkErr[m_nErrCount];}

    void Clear(unsigned char* buf, int pitch, int nIndex, int nStatus);

	void   Deal();

public:
	int             m_nIndex;
	int             m_nErrCount;             ///<实际错误数
	kxErrorMsg      m_checkErr[ _ERR_COUNT ];
	int             m_nStatus;  //标记特殊错误；
	unsigned char*  m_pBuf;   	////////图像信息
	int			    m_nPitch;   ////////图像信息
	int barQuality;
	char barRes[1<<7];
	char barExpe[1<<7];
	int dataQuality;
	char dataRes[1<<7];
	char dataExpe[1<<7];
	int numQuality;
	char numRes[1<<7];
	char numExpe[1<<7];
	int barX;
	int barY;
	int dataX;
	int dataY;
	int numX;
	int numY;
	int disX;
	int disY;
	kxImageBuf barImage;
	kxImageBuf dataImage;
	kxImageBuf numImage;
	kxCImageBuf cardImage;

	int m_nErrAreaNumber;
	int m_nSimErrKernNumber;

	int m_nColorDiff;

	//char        m_szIdentify[tagCheckResultDef::MAX_IDENTIFY_COUNT*2][256];  //识别的可变信息特征
	//kxRect<int> m_rcIdentify[tagCheckResultDef::MAX_IDENTIFY_COUNT];       //可变信息的位置
};

#pragma pack(pop)

class kxCheckResult : public kxCImgQue
{
public:
	kxCheckResult();
	~kxCheckResult();
	
	int   Init( int  nQueSize );
	void   Release();
	
	unsigned char*GetErrMsgByIndex(int nIndex)
	{
		if (nIndex<0||nIndex>kxCImgQue::GetCapability())
		{  
			return NULL;
		}
		return  GetImgBuf()+GetBlockSize()*nIndex;
	}
	void Clear()
	{
		kxCImgQue::Clear();
	}
	int  IsFull()
	{
		return kxCImgQue::IsFull(); 
	}
	void Push( PreDealImgMsg&  ImgMsg);
	void Push()
	{
		kxCImgQue::Push();
	}
public:
    int              m_nObjID;
	static     int    sm_nObjNum;
};

