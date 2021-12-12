#pragma once
#include "stdafx.h"
#include "global.h"
#include "KxAlogrithm.h"
#include <time.h>

//#define _SAPCLASS
//#define _Hikvision
//#define _Genie_TS
//#define _PC30
//#define _GigE
//#define _Chromasens
//#define _JAI
//#define _PointGrey 
//#define _PYLON
//#define _Baumer

class kxCGrabCamera
{
public:
	kxCGrabCamera();
	virtual ~kxCGrabCamera();
public:
   virtual int Stop()=0     //停止采样
	{
	}
	virtual int Start()=0   //启动采样
	{
	}
	virtual int Close();//add 2020.09.05 关闭相机

    virtual int Init();
    virtual void OnGrab( const unsigned char* buf, float fTime );  //需要将buf的nSize字节 进队列

	//virtual void OnGrab(const unsigned char* buf, int nW, int nH, float fTime = 0.0f);  //需要将buf的nSize字节 进队列
	virtual void OnGrab(const unsigned char* buf, int nW, int nH, int nChannel = _Type_G8);  //2020.08.28 需要将buf的nSize字节 进队列

	enum
	{
		_Len_Path  = 500,
	};

	virtual int SoftwareTrigger();
	virtual int OpenInternalTrigger(int nStatus);
	virtual int CloseSoftwareControl();
	virtual int  SetActiveTrigger();
	virtual int  OpenSoftwareControl();
	virtual void Alarm(int nAlarmStatus, int nTime = 1000);

	virtual void ReverseScanDirection(int nStatus) {}

	struct Parameter 
	{
		void* pEnvirment;
		char CardName[_Len_Path];
		char CfgFileName[_Len_Path];
		int nCardIndex;
		int   m_nReserve[100000];
	};
    Parameter   m_Parameter;
	Parameter& GetParameter(){return m_Parameter;}
	void   SetParameter(void* pEnvirment, int nCardIndex, std::string CardName, std::string CfgFileName)
	{
		sprintf_s(m_Parameter.CardName, CardName.length()+1 ,CardName.c_str());
		sprintf_s(m_Parameter.CfgFileName, CfgFileName.length()+1 ,CfgFileName.c_str());
        m_Parameter.pEnvirment =pEnvirment;
        m_Parameter.nCardIndex=nCardIndex;
	}
	void  SetCardName(std::string CardName)
	{
		sprintf_s(m_Parameter.CardName, CardName.length()+1 ,CardName.c_str());
	}
	void  SetCfgFileName(std::string CfgFileName)
	{
		sprintf_s(m_Parameter.CfgFileName, CfgFileName.length()+1 ,CfgFileName.c_str());
	}
	void  SetnCardIndex(int nCardIndex)
	{
          m_Parameter.nCardIndex=nCardIndex;
	}

	bool                  m_bStop;
	bool IsStop() const { return m_bStop; }             //!< 判断采集是否停止
	bool                  m_bInitIsOk;
	bool IsInitIsOk() const { return m_bInitIsOk; }             //!< 判断采集是否停止

	long long  m_nGrabNum;
	long long  GetGrabNum(){return  m_nGrabNum;}
	 void      SetGrabNum(int n){m_nGrabNum = n;}
	 void      IncGrab(){m_nGrabNum++;}

    clock_t m_start,m_end;
	kxImageBuf m_TmpBuf;
};
