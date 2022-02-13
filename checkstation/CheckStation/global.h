#pragma once

//�����㷨��
#include "KxAlogrithm.h"
#include "kxPrintf.h"
#include <time.h>
#include <direct.h>
#include <io.h>
#include <map>

//�������
//#define _SAPCLASS
//#define _Genie_TS
//#define _DALSA

//ȫ�ֳ���
const int	_AreaCount = 12;								//����������
const int	_Check = 1;
const char	iniPath[] = ".\\CheckStation.xml";			//�����ļ�·��
const int	_BlockImageCycle = 100000000;						//��ͼ����
const int	gc_CaptureClockSize = 1000;
const int	gc_nCameraMacAddress = 180;
const char	gc_szCameraMacAddress[gc_nCameraMacAddress][18] = 
{	
	//�ݵ������������
	"00-01-0D-C2-AB-54",//ȫϢ������
	"00-01-0D-C2-D9-B7",
	"00-01-0D-C2-D9-B4",
	"00-01-0D-C2-AA-AE",
	"00-01-0D-C2-AA-AA",

	// 202.02.14 ���������棬�������


};

//��������
//enum
//{
//	_Check_Ok = 0,
//	_Check_Err = 1,
//	_Similarity_Err = 2,
//
//};

//
//#define _OpenNetwork
//��Ϣ����
enum MessageType
{
	// 2020.02.14 ����

	/*��ȷ�����á�*/
	MSG_HANDSHAKE_SEND = 3001, // ��������ȷ��
	MSG_CHECK_RESULT = 1,	     //�����
	MSG_CHECK_RESULT_FINISH = 2, //��ǰpack������


	MSG_START_CHECK = 1001,	//��ʼ���
	MSG_STOP_CHECK = 1002,	//ֹͣ���
	MSG_SAVE_IMAGE = 1003,	//��ͼ
	MSG_SAVE_BAD_IMAGE = 1033,	//�滵ͼ
	MSG_ALL_IMG = 1005,	//���Ų��ԣ�ģ����
	MSG_START_CAMERA = 1012,  //��ʼ�ɼ�
	MSG_STOP_CAMERA = 1013,  //ֹͣ�ɼ�
	MSG_LOG = 1016,	 //��־
	MSG_CHANGE_EXPOURE_TIME = 1014,  //�����ع�ʱ��
	MSG_CAMERA_IS_READY = 1022,   //�����ʼ����ɣ�׼������
	MSG_START_CHECK_IS_READY = 1021,   //��ʼ����ʼ����ɣ��������ز�����
	MSG_SEND_REAL_TIME_IMAGE	= 1015,  //����ʵʱͼ��

	MSG_A = 2022,//��ȡA��Ϣ

	//MSG_ONE_IMG					= 1004,	//���Ų���
	//MSG_LEARN_DEFECT			= 1006, //����ȱ��ѧϰ
	//MSG_LEARN_DEFECT_COMPLETED  = 1007, //����ȱ��ѧϰ���
	//MSG_LEARN_ONE               = 1008, //����ѧϰ
	MSG_LEARN_ONE_COMPLETED		= 1009, //����ѧϰ���


	MSG_DOT_CHECK_OPEN = 4001,//�������
	MSG_DOT_CHECK_CLOSE = 4002,//�رյ��
	MSG_DOT_CHECK_RESULT = 4100,//�������

	MSG_BUILD_MODEL_IMG = 402, //��վ���ͻ����Ľ�ģͼ��

	//MSG_BUILD_MODEL_IMG_SECOND = 404, //��վ���Ͷ��ν�ģͼ��

	MSG_JUST_OPENCAMERA_BUILDMODEL = 501, //ֻ�����

	MSG_JUST_CLOSECAMERA_BUILDMODEL = 502, //ֻ�ر����

	MSG_CHANGE_CAMERA_INFO_REVERSE = 601, //�޸����ȡͼ����

	MSG_RECOVER_CAMERA_INFO_REVERSE = 602, //�������ȡͼ����

	MSG_JUST_OPENCAMERA = 603,//ֻ�Ǵ����

	MSG_CHANGE_CAPTURE_COL = 604,


	MSG_CLOSECAMERA = 1090, //�رպ������

	MSG_PACK_ID = 701, //����pack id

	MSG_SHOW_IMG = 1100, //���ʱ���ĵ�ʵʱͼ������ʵʱ��ʾ������ʾ


};


//��������
enum CheckResultStatus
{
	_Check_Ok = 0,
	_Check_Err = 1,
	_Similarity_Err = 2,
};

//Bayerת������ipp��bayerת����һ����
enum BayerMode
{
	_GRBG,
	_BGGR,
	_GBRG,
	_RGGB,
};



//Ԥ����ͼ����Ϣ
struct PreDealImgMsg
{
	unsigned char*	pBuf;
	int				nWidth;
	int				nHeight;
	int				nPitch;
	int				nIndex;			//�����
	int				nStatus;
	int				nTemplateIndex;	//���ģ���
	int				nSideLen[4];
};


struct GrabStatus
{
	void init()
	{
		nGrabDirection = 0;

		nGrabTimes = 0;
	}

	int				nGrabDirection;		//�ɼ��������ھ������ϻ��µ�ͼ��.0Ϊ���ϣ�1Ϊ����
	int				nGrabTimes;			//��Եڼ���ROI�ɼ�
};
const  int  g_ncMAXGRABNUM = 28000; //�����ͷ�ߵ�β�����ô���У�ÿһ�вɼ���ô���� 

//ȫ�ֱ���������������global.cpp��
extern char	g_szCameraDeviceId[1 << 8];				//����豸ID
extern char	g_szCameraConfigPath[1 << 8];				//��������ļ�·��
extern bool	g_bIsSimulate;							//�ǲ���ģ����
extern int  g_SimulateTimeStep;						//��ͼʱ����
extern int	g_expoureTime;							//�ع�ʱ��
extern int	g_trigger;								//����ģʽ 0:�ڴ���,1:�ⴥ��
extern char	g_savePath[1 << 8];						//��ͼ·��
extern int	g_grabWidth;							//�ɼ�ͼ��
extern int	g_grabHeight;							//�ɼ�ͼ��

extern GrabStatus  g_Grabstatus;					//�ɼ�״̬

extern bool g_bdotcheckstatus;//���״̬


class CKxCaptureImage
{
public:
	kxCImageBuf	m_Image;
	int			m_ImageID;// �����ʼΪ0
	int			m_CardID;
	int			m_Type;

	// Ϳ����Ŀ



};

//�쳣��Ϣ��Ӣ�ķ���
struct Translator
{
	std::string szChinese;
	std::string szEnglish;
};
const int nTranslatormapNum = 1024;
const Translator g_hTranslator[nTranslatormapNum] = { { "��ʼ���",                            "Start Check" },
								                      { "ֹͣ���",                            "Stop  Check" },
													  { "��������ʧ�ܣ��޷���ʼ�������",      "Load parameters failure, and cannot start check!" },
													  { "����ģ�����",                        "Load parameters successful" },
													  { "����·����%s",                        "Parameters path is %s" },
													  { "��ͼ·����%s",                        "Save Images Path is %s" },
													  { "��ʼ��ͼ",                            "Begin starting save images" },
													  { "��ʼģ����·��������,·��Ϊ��%s",   "Load pictures failure, Because of the path isn't exist with Off-line Check, the path is %s" },
													  { "��ʼģ����_·��Ϊ��%s",             "Off-line Check, the path is %s" },
													  { "����ȱ��ѧϰ",                        "Single defect learn" },
													  { "����ȱ��ѧϰ_%s,ȱ��ѧϰʧ��",        "Single defect learn  failure, because of %s" },
													  { "����%d��%d��ȱ�ݾֲ�ѧϰ���_%s",     "Area %d Index %d Single defect learn successful %s" },
													  { "����ѧϰ_%s, ѧϰʧ��",               "Single Image learn failure, Because of %s" },
													  { "����%d�ĵ���ѧϰ���_%s",             "Area %d Single Image learn successful %s" },
													  { "��ʼ�ڴ����ɼ�",                      "Start Grab Image by internal trigger mode" },
													  { "ֹͣ�ɼ�",                            "Stop Grab" },
													  { "ѧϰ���",                            "Learning has completed" },
													  { "����%d:%s",                           "Area %d:%s" },

};

