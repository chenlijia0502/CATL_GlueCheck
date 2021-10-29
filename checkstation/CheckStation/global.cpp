#include "stdafx.h"
#include "global.h"

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


