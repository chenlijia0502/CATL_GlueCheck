#include "stdafx.h"
#include "kxParameter.h"
#include "global.h"
#include "KxReadXml2.h"

CkxParameter    g_Parameter;
CkxParameter::CkxParameter()
{
	//m_nIndex = 1;
	//m_nPre2Index = 0;
	//Insert_Check();

	m_nQueSize = 30;
	m_nStandardWidth = 1400;
	m_nStandardHeight = 900;
	m_nStandardPitch = 1400*3;
	m_szNetSaveImagePath[0] = '\0';
	m_szNetExposureSaveImagePath[0] = '\0';
	m_bChangeExpoureTimeStatus = false;
	m_nSendImageCount = 0;
	m_nSendInfoInterval = 1;
	m_nLanguageMode = 1;
	m_nQueSize = 30;
	m_nImgType = 1;
	m_nIndex=0;				
	m_bIsBuildModelStatus = false;
	m_szNetBuildModelSaveImagePath[0] = '\0';

	//m_nVariableJugeFlag = 0;
	//m_bSaveImg = false;
	//m_nSaveTotalCounts = 200;
	//m_nSaveCount = 0;
	//m_szSavePath[0] = '\0';
	//m_bSavebadImg = false;
	//m_szSavebadPath[0] = '\0';
	//m_nIsperiodicCheck = 0;
	//m_nLargeSize = 3;
	//m_nSignatureExtendLen = 10;
	//m_nSaveBayerImg = 0;
	//m_bIsUsehardware = 1;
}

CkxParameter::~CkxParameter()
{
	
}

bool CkxParameter::LoadXmlPara(const char* lpszFile)
{
	std::string szResult;
	//------------------------------����ģʽ------------------------------//
	//int nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "����ģʽ", szResult);
	//if (nSearchStatus)
	//{
	//	int nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nLanguageMode);
	//	if (!nStatus)
	//	{
	//		return false;
	//	}
	//}

	if (m_nLanguageMode == 0)
	{
		LoadXmlParainEnglish(lpszFile);
	}
	else
	{
		LoadXmlParainChinese(lpszFile);
	}
	return true;
}


bool CkxParameter::LoadXmlParainChinese(const char* lpszFile)
{
	std::string szResult;
	//int nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "����", "վ���", szResult);
	//if (nSearchStatus)
	//{
	//	int nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nStationID);
	//	if (!nStatus)
	//	{
	//		return false;
	//	}
	//}

	int nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "ȫ������", "������", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nAreakNum);
		if (!nStatus)
		{
			return false;
		}
	}
	for (int i = 0; i < m_Parameter.m_nAreakNum; i++)
	{
		m_Parameter.m_nUsedArea[i] = 1;
	}

	//nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "����", "�ع�ʱ��", szResult);
	//if (!nSearchStatus)
	//{
	//	return false;

	//}
	//int nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nExpoureTime);
	//if (!nStatus)
	//{
	//	return false;
	//}

	//nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "����", "����ʱ��", szResult);
	//if (!nSearchStatus)
	//{
	//	return false;

	//}
	//nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nAlarmTime);
	//if (!nStatus)
	//{
	//	return false;
	//}

	//nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "����", "���ͼ��", szResult);
	//if (!nSearchStatus)
	//{
	//	return false;

	//}
	//nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nSendInfoLen);
	//if (!nStatus)
	//{
	//	return false;
	//}

	//nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "����", "����Ƶ��", szResult);
	//if (!nSearchStatus)
	//{
	//	return false;

	//}
	//nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nGrabCount);
	//if (!nStatus)
	//{
	//	return false;
	//}



	return true;


}



/* --&&-----------------------------------------------------&&-- */
bool CkxParameter::LoadXmlParainEnglish(const char* lpszFile)
{
	std::string szResult;

	//int nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "Setting", "StationID", szResult);
	//if (nSearchStatus)
	//{
	//	int nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nStationID);
	//	if (!nStatus)
	//	{
	//		return false;
	//	}
	//}

	int nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "GlobalSetting", "AreaNumber", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nAreakNum);
		if (!nStatus)
		{
			return false;
		}
	}
	for (int i = 0; i < m_Parameter.m_nAreakNum; i++)
	{
		m_Parameter.m_nUsedArea[i] = 1;
	}

	//nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "Setting", "ExposureTime", szResult);
	//if (!nSearchStatus)
	//{
	//	return false;

	//}
	//int nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nExpoureTime);
	//if (!nStatus)
	//{
	//	return false;
	//}

	//nSearchStatus = KxXmlFun2::SearchXmlGetValue(lpszFile, "Setting", "AlarmTime", szResult);
	//if (!nSearchStatus)
	//{
	//	return false;
	//}
	//nStatus = KxXmlFun2::FromStringToInt(szResult, m_Parameter.m_nAlarmTime);
	//if (!nStatus)
	//{
	//	return false;
	//}

	return true;


}

const char* CkxParameter::g_TranslatorChinese(const char* sz)
{
	if (m_nLanguageMode)  //����ģʽ
	{
		return sz;
	}
	else
	{
		int nIndex = 0;
		bool bFind = false;
		for (int i = 0; i < 1024; i++)
		{
			if (strcmp(sz, g_hTranslator[i].szChinese.c_str()) == 0)
			{
				bFind = true;
				nIndex = i;
				break;
			}
		}

		if (bFind)
		{
			return g_hTranslator[nIndex].szEnglish.c_str();
		}
		else
		{
			return g_hTranslator[nIndex].szChinese.c_str();
		}
	}
}
namespace  Config
{

}


//int CkxParameter::Load( const char* lpszFile )
//{
//	FILE*   fp;
//	if( fopen_s( &fp, lpszFile, "rb" ) != 0 )
//		return FALSE;
//	int b = Read( fp );	
//	fclose( fp );
//	return b;
//}
//
//int CkxParameter::Save( const char* lpszFile )
//{
//	FILE*   fp;
//	if( fopen_s( &fp, lpszFile, "wb" ) != 0 )
//	{
//		return FALSE;
//	}
//	int b = Write( fp );
//	fclose( fp );
//	return b;
//}
//
//bool CkxParameter::ReadVesion1Para( FILE* fp)    //��ȡ�汾1����
//{
//	if (fread(&m_Parameter.m_nAreakNum, sizeof(int), 1, fp) != 1)		
//	{
//		return false;
//	}
//	if (fread(&m_Parameter.m_nP2ThreadNum, sizeof(int), 1, fp) != 1)		
//	{
//		return false;
//	}
//	if (fread(&m_Parameter.m_nStationID, sizeof(int), 1, fp) != 1)		
//	{
//		return false;
//	}
//	if (fread(&m_Parameter.m_nUsedArea, sizeof(int), _AreaCount, fp) != _AreaCount)		
//	{
//		return false;
//	}
//	if (fread(&m_Parameter.m_nExpoureTime, sizeof(int), 1, fp) != 1)		
//	{
//		return false;
//	}
//	if (fread(&m_Parameter.m_nLearn, sizeof(int), 1, fp) != 1)
//	{
//		return false;
//	}
//	
//	if (fread(&m_Parameter.m_nRunMode, sizeof(int), 1, fp) != 1)
//	{
//		return false;
//	}
//
//
//	return true;
//}
//
//
//
//bool CkxParameter::Read( FILE* fp )
//{
//	if (fread(m_Parameter.m_szVersion, sizeof(m_Parameter.m_szVersion), 1, fp) != 1)
//	{
//		return false;
//	}
//	if (strcmp(m_Parameter.m_szVersion, "GlobalParam1.0") == 0)   
//	{
//		return ReadVesion1Para(fp);
//	}
//	//else if (strcmp(m_hParameter.m_szVersion, "GlobalParam1.0") == 0)
//	//{
//	//  ............
//	//}
//	else
//	{
//		return false;
//	}
//}
//
//
//bool CkxParameter::WriteVesion1Para( FILE* fp)    //��ȡ�汾1����
//{
//	if (fwrite(m_Parameter.m_szVersion, sizeof(m_Parameter.m_szVersion), 1, fp) != 1)
//	{
//		return false;
//	}
//	if (fwrite(&m_Parameter.m_nAreakNum, sizeof(int), 1, fp) != 1)		
//	{
//		return false;
//	}
//	if (fwrite(&m_Parameter.m_nP2ThreadNum, sizeof(int), 1, fp) != 1)		
//	{
//		return false;
//	}
//	if (fwrite(&m_Parameter.m_nStationID, sizeof(int), 1, fp) != 1)		
//	{
//		return false;
//	}
//	if (fwrite(&m_Parameter.m_nUsedArea, sizeof(int), _AreaCount, fp) != _AreaCount)		
//	{
//		return false;
//	}
//	if (fwrite(&m_Parameter.m_nExpoureTime, sizeof(int), 1, fp) != 1)		
//	{
//		return false;
//	}
//	if (fwrite(&m_Parameter.m_nLearn, sizeof(int), 1, fp) != 1)		
//	{
//		return false;
//	}
//	if (fwrite(&m_Parameter.m_nRunMode, sizeof(int), 1, fp) != 1)
//	{
//		return false;
//	}
//
//	return true;
//}
//
//
//bool CkxParameter::Write( FILE* fp )
//{
//	if (strcmp(m_Parameter.m_szVersion, "GlobalParam1.0") == 0)   
//	{
//		return ReadVesion1Para(fp);
//	}
//	//else if (strcmp(m_hParameter.m_szVersion, "GlobalParam1.0") == 0)
//	//{
//	//  ............
//	//}
//	else
//	{
//		return false;
//	}
//}
//
//int CkxParameter::Read( unsigned char*& point )
//{
//	memcpy(&m_Parameter, point, sizeof(Parameter));
//	point+=sizeof(Parameter);
//	//...............................................
//	return TRUE;
//}
//



