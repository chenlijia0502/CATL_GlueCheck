#pragma once

//科信算法库
#include "KxAlogrithm.h"
#include "kxPrintf.h"
#include <time.h>
#include <direct.h>
#include <io.h>
#include <map>

//相机开关
//#define _SAPCLASS
//#define _Genie_TS
//#define _DALSA

//全局常量
const int	_AreaCount = 12;								//最大区域个数
const int	_Check = 1;
const char	iniPath[] = ".\\CheckStation.xml";			//配置文件路径
const int	_BlockImageCycle = 100000000;						//黑图周期
const int	gc_CaptureClockSize = 1000;
const int	gc_nCameraMacAddress = 180;
const char	gc_szCameraMacAddress[gc_nCameraMacAddress][18] = 
{	
	//捷德贴标智能相机
	"00-01-0D-C2-AB-54",//全息标白相机
	"00-01-0D-C2-D9-B7",
	"00-01-0D-C2-D9-B4",
	"00-01-0D-C2-AA-AE",
	"00-01-0D-C2-AA-AA",

	// 202.02.14 类似于上面，进行添加


};

//错误类型
//enum
//{
//	_Check_Ok = 0,
//	_Check_Err = 1,
//	_Similarity_Err = 2,
//
//};

//
//#define _OpenNetwork
//消息类型
enum MessageType
{
	// 2020.02.14 整理

	/*—确认有用—*/
	MSG_HANDSHAKE_SEND = 3001, // 子主连接确认
	MSG_CHECK_RESULT = 1,	     //检测结果

	MSG_START_CHECK = 1001,	//开始检测
	MSG_STOP_CHECK = 1002,	//停止检测
	MSG_SAVE_IMAGE = 1003,	//存图
	MSG_SAVE_BAD_IMAGE = 1033,	//存坏图
	MSG_ALL_IMG = 1005,	//多张测试，模拟跑
	MSG_START_CAMERA = 1012,  //开始采集
	MSG_STOP_CAMERA = 1013,  //停止采集
	MSG_LOG = 1016,	 //日志
	MSG_CHANGE_EXPOURE_TIME = 1014,  //调整曝光时间
	MSG_CAMERA_IS_READY = 1022,   //相机初始化完成，准备就绪
	MSG_START_CHECK_IS_READY = 1021,   //开始检测初始化完成，包括加载参数等
	MSG_SEND_REAL_TIME_IMAGE	= 1015,  //发送实时图像

	MSG_A = 2022,//获取A消息

	//MSG_ONE_IMG					= 1004,	//单张测试
	//MSG_LEARN_DEFECT			= 1006, //单个缺陷学习
	//MSG_LEARN_DEFECT_COMPLETED  = 1007, //单张缺陷学习完成
	//MSG_LEARN_ONE               = 1008, //单张学习
	MSG_LEARN_ONE_COMPLETED		= 1009, //单张学习完成


	//MSG_SET_RESIIDUE            = 1010,	//设置残差图和偏移值参数
	//MSG_GET_RESIIDUE            = 1011,

	
	//MSG_PARAM_TemplateImg       = 1017,  //模板学习完成

	//MSG_TRIGGLE_CAMERA          = 1018,  //给相机触发
	//
	//MSG_GET_LEARN_PAGES         = 1019,   //主站问子站要学习张数
	//MSG_SEND_LEARN_PAGES        = 1020,   //子站发主站学习张数

	//

	//MSG_ADJUST_CAMERA			= 1023,  //相机自动调焦，获取关键信息值
	//MSG_ADJUST_CAMERA_RESULT	= 1024,  //相机自动调焦，获取关键信息值
	//MSG_ADJUST_CAMERA_DONE		= 1043,  //相机自动调焦终止

	//MSG_CONTROL_SLIDEMOTOR		=1025,   //硬件通信改为主站走UDP
	//MSG_CANNOT_FOCUS			= 1027,  //无法聚焦

	//MSG_CHANGE_ADJUST_MOTOR_STATUS = 1044, //改变切换调焦状态，也即自动还是手动

};


//错误类型
enum CheckResultStatus
{
	_Check_Ok = 0,
	_Check_Err = 1,
	_Similarity_Err = 2,
};

//Bayer转化，跟ipp的bayer转换是一样的
enum BayerMode
{
	_GRBG,
	_BGGR,
	_GBRG,
	_RGGB,
};



//预处理图像信息
struct PreDealImgMsg
{
	unsigned char*	pBuf;
	int				nWidth;
	int				nHeight;
	int				nPitch;
	int				nIndex;			//总序号
	int				nStatus;
	int				nTemplateIndex;	//检测模版号
	int				nSideLen[4];
};



//全局变量申明，定义在global.cpp中
extern char	g_szCameraDeviceId[1 << 8];				//相机设备ID
extern char	g_szCameraConfigPath[1 << 8];				//相机配置文件路径
extern bool	g_bIsSimulate;							//是不是模拟检测
extern int  g_SimulateTimeStep;						//读图时间间隔
extern int	g_expoureTime;							//曝光时间
extern int	g_trigger;								//触发模式 0:内触发,1:外触发
extern char	g_savePath[1 << 8];						//存图路径
extern int	g_grabWidth;							//采集图宽
extern int	g_grabHeight;							//采集图高


class CKxCaptureImage
{
public:
	kxCImageBuf	m_Image;
	int			m_ImageID;
	int			m_CardID;
	int			m_Type;
};

//异常信息中英文翻译
struct Translator
{
	std::string szChinese;
	std::string szEnglish;
};
const int nTranslatormapNum = 1024;
const Translator g_hTranslator[nTranslatormapNum] = { { "开始检测",                            "Start Check" },
								                      { "停止检测",                            "Stop  Check" },
													  { "参数载入失败，无法开始正常检查",      "Load parameters failure, and cannot start check!" },
													  { "载入模板完成",                        "Load parameters successful" },
													  { "参数路径：%s",                        "Parameters path is %s" },
													  { "存图路径：%s",                        "Save Images Path is %s" },
													  { "开始存图",                            "Begin starting save images" },
													  { "开始模拟检查路径不存在,路径为：%s",   "Load pictures failure, Because of the path isn't exist with Off-line Check, the path is %s" },
													  { "开始模拟检查_路径为：%s",             "Off-line Check, the path is %s" },
													  { "单个缺陷学习",                        "Single defect learn" },
													  { "单个缺陷学习_%s,缺陷学习失败",        "Single defect learn  failure, because of %s" },
													  { "区域%d的%d号缺陷局部学习完成_%s",     "Area %d Index %d Single defect learn successful %s" },
													  { "单张学习_%s, 学习失败",               "Single Image learn failure, Because of %s" },
													  { "区域%d的单张学习完成_%s",             "Area %d Single Image learn successful %s" },
													  { "开始内触发采集",                      "Start Grab Image by internal trigger mode" },
													  { "停止采集",                            "Stop Grab" },
													  { "学习完成",                            "Learning has completed" },
													  { "区域%d:%s",                           "Area %d:%s" },

};

