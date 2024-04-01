#pragma once
#include "KxAlogrithm.h"
#include "global.h"

/*!
	date:	2020.02.17
	author:	HYH
	desc:	ȫ�ֲ�������������ɣ�һ�����ò���;�������ڽṹ�壬��ʾ����վglobal.xml�ж�ȡ��ģ��ȫ�ֲ���
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
		{  //��ʾ����վ��ȡ��global.xml�еĲ���
			m_nAreakNum = 1;
			memset(m_nUsedArea, 0, sizeof(int)*_AreaCount);
			m_nUsedArea[0] = 1;
			m_nExpoureTime = 300;
		}  
		int		 m_nAreakNum;						//����������ʾ�����߳����������ͬʱҲ��һ��վ����ٷݼ�����õ�����(��global.xml�ļ��ж�ȡ)
		int      m_nUsedArea[_AreaCount];			//һ����־λ����������������Ӧ����ʾ��Ӧ�������Ƿ���
		int      m_nExpoureTime;					//����ع�ʱ��
	};
#pragma pack(pop)

	
public:
    Parameter   m_Parameter;
	// 2020.02.17 ����������ֵ���������������Ч�ģ�������������Ч�����������������������������ǿ��Զ�ȡ�ģ�û��Ҫ��
	int		        m_nStandardWidth;						//ͼ������ö�ȡ
	int		        m_nStandardHeight;						//ͼ���
	int		        m_nStandardPitch;						//ͼ��pitch
	int             m_nImgType;								//0�ǵ�ͨ�� 3����ͨ�������ö�ȡ(Color Ϊ3�� Gray Ϊ1)

	int		        m_nQueSize;								//���д�С�����ö�ȡ
	char            m_szNetSaveImagePath[1 << 8];			//��������ͼ·��,��վ��ͼ����վ��ȡ����Ҫ�Ǽ���ʱ���ã����ö�ȡ
	char            m_szNetExposureSaveImagePath[1 << 8];   //�Զ��ع��������ͼ·����������Ҫ��ʵʱ��ͼʱ��ͼ·�������ö�ȡ
	BayerMode       m_nBayerMode;							//��Ҷ˹ת��ģʽ�����ö�ȡ
	int				m_nNetStationId;						//��ǰ��վID�ţ��������Ҫ������վͨ������ؼ����ã����ö�ȡ(��ǰ����һ��m_nStationID��������һ����)
	int				m_nSendInfoInterval;					//���ͼ������վ�Ĵ��������ٴ�֮��Ĭ�ϴ���վ����ȥ�����ö�ȡ
	int				m_nLanguageMode;						//����ģʽ��0ΪӢ�ģ�1Ϊ���ģ����ö�ȡ���ǵù�����������վ����վ��Ҫ��������һ�ݣ�
	int				m_nSaveImgTotalCounts;					//���α���ͼƬ����

	int             m_nCurrentUIid;							//��ǰ���������id��(�������������Ժ�Ҫȡ������վ��͹̶�Ϊ0)
	bool			m_bChangeExpoureTimeStatus;				//�������ͼ���͵��δ���trueΪ�浽���أ����ݱ�������վ��false���������봦����У����Ʊ�������䶯
	int				m_nSendImageCount;						//���ֵ��Ҫ����������ʵʱ����ͼ�񵽽�����ٶȣ�ͳ�Ƶ�ǰ��������ĳ��ֵȡ�࣬������ͼ��

	int				m_nIndex;								//������CardIndex ��ȡ�������Դ���ѡ���ĸ����

	char			m_szCardName[512];
	char			m_szCfgFileName[512];
	int				m_nCardIndex;
	char			m_szCameraDeviceId[256];
	char            m_szNetBuildModelSaveImagePath[1 << 8]; //��ģ�Ĵ�ͼ����
	bool			m_bIsBuildModelStatus;					//��ǰ�Ƿ��ͼ״̬���ǵĻ�ֱ�ӽ�ͼ���͵���վ

	char			m_szNetDotCheckImgpath[1 << 8];//���ɼ�ͼ�񱣴�·��

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

