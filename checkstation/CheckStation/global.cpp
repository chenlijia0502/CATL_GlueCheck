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


