 #pragma once
#include "windows.h"
//����ʱ�� 20090913 
#define GRAB_MAIN_VERSION 3
#define GRAB_SUB_VERSION 1

#define GRAB_VERSION ((GRAB_MAIN_VERSION << 16) + GRAB_SUB_VERSION)

enum GRAB_PARAM_INDEX_CONST
{
	GRAB_NONE = 0,
	GRAB_EXPOSURETIME,     //�����ع�ʱ��
	GRAB_GAIN,
	GRAB_OFFSET,
	GRAB_SOFT_TRIG,   //
	GRAB_TRIG_MODE,    //���ô���ģʽ

	GRAB_PARAM_COUNTER  //20090913 Eureka �����ӵ��������ͣ��������������ܺ�
};

//����ͼ�����������

/*
	GRAB_FORMAT ֪ͨ�����ߣ����ͼ���ʽ
		
*/
struct GRAB_FORMAT
{
	int width , height , bit;
	int cameraCount;
	int nNeedBayerConvert;	
	unsigned int nPixelType;
};

////20100814-1128 Eureka
////�鿴JAI���Stream��buffer�Ľṹ
//typedef struct _tagJAIStreamAndBufferInfoDef
//{
//	int nTotalMissingPackets;        //������
//	int nNumOfFramesLose;            //��֡��
//	int nNumOfBufferAnnounced;       //�ܻ�������
//	int nNumOfBuffersQueue;          //���û�������
//	int nNumOfFramesAwaitDelivered;  //����֡��
//	int nNumOfFramesDelivered;       //���ϴ�������⣬���������� ��������
//	int nNumOfFramesCorrupt;         //��֡��
//	int nPassCorruptFrameFlag;       //������֡��״̬λ
//
//	int nFrameIndex;                 //�ɼ�֡��
//	SYSTEMTIME hAbsSystemTime;       //�ɼ�ʱ�̵�ϵͳ����ʱ��
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

	//SetPara() ���ò���
	//paraIndex -- ��������
	//data ���������ָ�룬�ж�������
	//���ز�������: 0 -- ���ò����ɹ�
	//	            -1 -- ���ò������ʹ���, paraIndex���ʹ���
	//              -2 -- �������ݴ���, data���ݴ���
	//				-3 -- ���ò����ĺ�������ʧ��
	virtual int _stdcall SetPara(int paraIndex , void * para) = 0;

	virtual int _stdcall GetPara(int paraIndex , void * para) = 0;
};

#ifdef USE_GRAB_DLL
	extern "C" CCardGrab * _stdcall CreateCCardGrabInstance(int version = GRAB_VERSION);
#else
	CCardGrab * _stdcall CreateCCardGrabInstance(int version = GRAB_VERSION);
#endif

