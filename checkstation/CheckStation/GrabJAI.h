 #pragma once
#include "windows.h"
//更新时间 20090913 
#define GRAB_MAIN_VERSION 3
#define GRAB_SUB_VERSION 1

#define GRAB_VERSION ((GRAB_MAIN_VERSION << 16) + GRAB_SUB_VERSION)

enum GRAB_PARAM_INDEX_CONST
{
	GRAB_NONE = 0,
	GRAB_EXPOSURETIME,     //设置曝光时间
	GRAB_GAIN,
	GRAB_OFFSET,
	GRAB_SOFT_TRIG,   //
	GRAB_TRIG_MODE,    //设置触发模式

	GRAB_PARAM_COUNTER  //20090913 Eureka 新增加的命令类型，用来计算命令总和
};

//设置图像参数的类型

/*
	GRAB_FORMAT 通知调用者，相机图像格式
		
*/
struct GRAB_FORMAT
{
	int width , height , bit;
	int cameraCount;
	int nNeedBayerConvert;	
	unsigned int nPixelType;
};

////20100814-1128 Eureka
////查看JAI相机Stream和buffer的结构
//typedef struct _tagJAIStreamAndBufferInfoDef
//{
//	int nTotalMissingPackets;        //丢包数
//	int nNumOfFramesLose;            //丢帧数
//	int nNumOfBufferAnnounced;       //总缓冲区数
//	int nNumOfBuffersQueue;          //可用缓冲区数
//	int nNumOfFramesAwaitDelivered;  //待传帧数
//	int nNumOfFramesDelivered;       //从上次启动检测，到检测结束， 传递总数
//	int nNumOfFramesCorrupt;         //损坏帧数
//	int nPassCorruptFrameFlag;       //传递损坏帧数状态位
//
//	int nFrameIndex;                 //采集帧数
//	SYSTEMTIME hAbsSystemTime;       //采集时刻的系统绝对时间
//	int nAbsCheckTimeFromLastStartInMillSeconds;
//}JAIStreamAndBufferInfoDef;


class __declspec(novtable) CCardGrabHook
{
public:
	virtual int _stdcall OnFrameGrabbed(int frameIndex , BYTE * imageDataTab , float time) = 0;
	virtual void _stdcall OnGrabStop() = 0;
};

class __declspec(novtable) CCardGrab
{
public:
	virtual int _stdcall Release() = 0;
	virtual int _stdcall Init(GRAB_FORMAT * grabFormat , LPCTSTR cardName , int cardIndex , LPCTSTR cfgFileName , CCardGrabHook * hook) = 0;
	virtual void _stdcall Free() = 0;
	virtual int _stdcall Start(int grabCount = -1) = 0;
	virtual void _stdcall Stop() = 0;

	//SetPara() 设置参数
	//paraIndex -- 参数类型
	//data 传入参数的指针，有多种类型
	//返回参数类型: 0 -- 设置参数成功
	//	            -1 -- 设置参数类型错误, paraIndex类型错误
	//              -2 -- 传入数据错误, data数据错误
	//				-3 -- 设置参数的函数调用失败
	virtual int _stdcall SetPara(int paraIndex , void * para) = 0;

	virtual int _stdcall GetPara(int paraIndex , void * para) = 0;
};

#ifdef USE_GRAB_DLL
	extern "C" CCardGrab * _stdcall CreateCCardGrabInstance(int version = GRAB_VERSION);
#else
	CCardGrab * _stdcall CreateCCardGrabInstance(int version = GRAB_VERSION);
#endif

