#include "stdafx.h"
#include "global.h"

//ȫ�ֱ�������
char	g_szCameraDeviceId[1<<8];				//����豸ID
char	g_szCameraConfigPath[1<<8];				//��������ļ�·��
bool	g_bIsSimulate;							//�ǲ���ģ����
int		g_SimulateTimeStep;						//��ͼʱ����
int		g_expoureTime;							//�ع�ʱ��
int		g_trigger;								//����ģʽ 0:�ڴ���,1:�ⴥ��
char	g_savePath[1 << 8];						//��ͼ·��
int		g_grabWidth;							//�ɼ�ͼ��
int		g_grabHeight;							//�ɼ�ͼ��

GrabStatus     g_Grabstatus;						//�ɼ�����

bool g_bdotcheckstatus;//���״̬


void Global_SaveDebugImg(const char* name, kxCImageBuf& saveimg)
{
	if (1)// ����������Ҫ����Ϊһ��bool��
	{
		CKxBaseFunction fun;
		char savepath[128];
		memset(savepath, 0, sizeof(savepath));
		sprintf_s(savepath, "d:\\img\\%s.bmp", name);
		fun.SaveBMPImage_h(savepath, saveimg);
	}
}