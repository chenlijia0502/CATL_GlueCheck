#include "stdafx.h"
#include "global.h"
#include <stdarg.h>


//全局变量定义
char	g_szCameraDeviceId[1<<8];				//相机设备ID
char	g_szCameraConfigPath[1<<8];				//相机配置文件路径
bool	g_bIsSimulate;							//是不是模拟检测
int		g_SimulateTimeStep;						//读图时间间隔
int		g_expoureTime;							//曝光时间
int		g_trigger;								//触发模式 0:内触发,1:外触发
char	g_savePath[1 << 8];						//存图路径
int		g_grabWidth;							//采集图宽
int		g_grabHeight;							//采集图高

GrabStatus     g_Grabstatus;						//采集方向

bool g_bdotcheckstatus;//点检状态


void Global_SaveDebugImg(kxCImageBuf& saveimg, const char* namefmt, ...)
{
	if (1)// 后面这里需要设置为一个bool量
	{
		char szInfo[1 << 8];
		va_list argptr;									//声明一个转换参数的变量
		va_start(argptr, namefmt);							//初始化变量
		vsnprintf(szInfo, sizeof(szInfo), namefmt, argptr);	//将带参数的字符串按照参数列表格式化到buffer中
		va_end(argptr);//结束变量列表,和va_start成对使用

		char savepath[1 << 10];
		memset(savepath, 0, sizeof(savepath));
		sprintf_s(savepath, "d:\\img\\%s.bmp", szInfo);

		CKxBaseFunction fun;
		fun.SaveBMPImage_h(savepath, saveimg);
	}
}