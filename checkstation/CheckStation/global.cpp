#include "stdafx.h"
#include "global.h"
#include <stdarg.h>


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


void Global_SaveDebugImg(kxCImageBuf& saveimg, const char* namefmt, ...)
{
	if (1)// ����������Ҫ����Ϊһ��bool��
	{
		char szInfo[1 << 8];
		va_list argptr;									//����һ��ת�������ı���
		va_start(argptr, namefmt);							//��ʼ������
		vsnprintf(szInfo, sizeof(szInfo), namefmt, argptr);	//�����������ַ������ղ����б��ʽ����buffer��
		va_end(argptr);//���������б�,��va_start�ɶ�ʹ��

		char savepath[1 << 10];
		memset(savepath, 0, sizeof(savepath));
		sprintf_s(savepath, "d:\\img\\%s.bmp", szInfo);

		CKxBaseFunction fun;
		fun.SaveBMPImage_h(savepath, saveimg);
	}
}