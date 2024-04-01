#pragma once
#include "KxAlogrithm.h"
#include "global.h"

/*!
	date:	2020.02.17
	author:	HYH
	desc:	全局参数，两部分组成，一是配置参数;二是类内结构体，表示从主站global.xml中读取的模板全局参数
*/

class CkxParameter    
{
public:
	CkxParameter();
	~CkxParameter();

#pragma pack(push, 1)
    struct Parameter
	{
		Parameter()
		{  //表示从主站读取的global.xml中的参数
			m_nAreakNum = 1;
			memset(m_nUsedArea, 0, sizeof(int)*_AreaCount);
			m_nUsedArea[0] = 1;
			m_nExpoureTime = 300;
		}  
		int		 m_nAreakNum;						//区域数，表示并行线程类的数量，同时也是一个站点多少份检测配置的体现(从global.xml文件中读取)
		int      m_nUsedArea[_AreaCount];			//一个标志位变量，跟区域数对应，表示对应区域数是否开启
		int      m_nExpoureTime;					//相机曝光时间
	};
#pragma pack(pop)

	
public:
    Parameter   m_Parameter;
	// 2020.02.17 下面这三个值在面阵相机中是无效的，但在线阵中有效（？？？？？待定，理论配置中是可以读取的，没必要）
	int		        m_nStandardWidth;						//图像宽，配置读取
	int		        m_nStandardHeight;						//图像高
	int		        m_nStandardPitch;						//图像pitch
	int             m_nImgType;								//0是单通道 3是三通道，配置读取(Color 为3， Gray 为1)

	int		        m_nQueSize;								//队列大小，配置读取
	char            m_szNetSaveImagePath[1 << 8];			//局域网存图路径,子站存图，主站读取，主要是检测的时候用，配置读取
	char            m_szNetExposureSaveImagePath[1 << 8];   //自动曝光局域网存图路径，现在主要是实时采图时存图路径，配置读取
	BayerMode       m_nBayerMode;							//贝叶斯转换模式，配置读取
	int				m_nNetStationId;						//当前子站ID号，这个很重要，与主站通信中其关键作用，配置读取(以前还有一个m_nStationID，两个是一样的)
	int				m_nSendInfoInterval;					//发送间隔，子站的处理结果多少次之后默认从主站发过去，配置读取
	int				m_nLanguageMode;						//语言模式，0为英文，1为中文，配置读取（记得哈，语言是主站、子站都要各自设置一份）
	int				m_nSaveImgTotalCounts;					//单次保存图片张数

	int             m_nCurrentUIid;							//当前交互界面的id号(？？？？？？以后要取消掉，站点就固定为0)
	bool			m_bChangeExpoureTimeStatus;				//控制相机图像送到何处，true为存到本地，数据被传向主站；false则正常进入处理队列；控制变量，会变动
	int				m_nSendImageCount;						//这个值主要是用来控制实时发送图像到界面的速度，统计当前张数，对某个值取余，正则发送图像

	int				m_nIndex;								//从配置CardIndex 读取，它可以代表选择哪个相机

	char			m_szCardName[512];
	char			m_szCfgFileName[512];
	int				m_nCardIndex;
	char			m_szCameraDeviceId[256];
	char            m_szNetBuildModelSaveImagePath[1 << 8]; //建模的存图队列
	bool			m_bIsBuildModelStatus;					//当前是否存图状态，是的话直接将图像发送到主站

	char			m_szNetDotCheckImgpath[1 << 8];//点检采集图像保存路径

public:
	Parameter & GetParameter()
	{
		return m_Parameter;
	}
	const Parameter& GetParameter() const
	{
		return m_Parameter;
	}

	
public:

	bool LoadXmlPara(const char* lpszFile);
	bool LoadXmlParainEnglish(const char* lpszFile);
	bool LoadXmlParainChinese(const char* lpszFile);
	const char* g_TranslatorChinese(const char* sz);
};
extern CkxParameter    g_Parameter;



namespace  Config
{
	
	inline CkxParameter& g_GetParameter() 
	{
		return g_Parameter;
	}

	inline  CkxParameter::Parameter & GetGlobalParam(){  return g_GetParameter().GetParameter(); }
		
	inline int & GetStandardWidth(){ return  g_Parameter.m_nStandardWidth; }
	inline int & GetStandardHeight(){ return   g_Parameter.m_nStandardHeight; }
	inline int & GetStandardPitch(){ return   g_Parameter.m_nStandardPitch; }
	
}

