#include "StdAfx.h"
#include "TestAsioTcpClient.h"
#include "CkxThreadManage.h"
#include "CkxEnvironment.h"
#include "Grab_Buffer.h"
#include "kxCheckResult.h"
#include "GrabPack.h"
#include "CkxFileRead.h"
#include "kxParameter.h"
#include "SaveQue.h"
#include "KxCheck.h"
#include "KxReadXml2.h"


CkxEnvironment::CkxEnvironment(void)
{
	m_bIsSimulate = false;
}

CkxEnvironment::~CkxEnvironment(void)
{
}

int CkxEnvironment::LoadAllObjByXml(char* configPath)
{
	string strPath = configPath;
	//strPath.append("\\");
	string strTemp;
	string modelpath;


	strTemp = strPath + "global.xml";
	if (!Config::g_GetParameter().LoadXmlPara(strTemp.c_str()))
	{
		if (Config::g_GetParameter().m_nLanguageMode)
			kxPrintf(KX_Err, "全局参数载入失败，请重新载入，请保存参数");
		else
			kxPrintf(KX_Err, "Global Parameters load failure, please confirm it");
		return 0;
	}

	for (int i = 0; i < Config::GetGlobalParam().m_nAreakNum; i++)
	{
		char configname[256], templateimgname[256];
		memset(configname, 0, 256);
		memset(templateimgname, 0, 256);
		sprintf_s(configname, "check%d.xml", i + 1);
		sprintf_s(templateimgname, "KxModel%d.dat", i + 1);

		strTemp = strPath + configname;
		modelpath = strPath + templateimgname;


		char error[256];
		//if (!Check::g_GetCheckCardObj().GetCheckTools(i).ReadParamXml(strTemp.c_str(), error, modelpath.c_str()))
		//{
		//	if (Config::g_GetParameter().m_nLanguageMode)
		//	{
		//		strcat_s(error, " 检测参数载入失败，请重新载入，请保存参数");
		//		kxPrintf(KX_Err, error);
		//	}
		//	else
		//	{
		//		strcat_s(error, " Global Parameters load failure, please confirm it");
		//		kxPrintf(KX_Err, error);
		//	}
		//	return 0;
		//}

		memset(error, 0, sizeof(error));
		if (!Check::g_GetCheckCardObj().ReadParamXml(strTemp.c_str(), error))
		{
			if (Config::g_GetParameter().m_nLanguageMode)
			{
				strcat_s(error, " 检测参数载入失败，请重新载入，请保存参数");
				kxPrintf(KX_Err, error);
			}
			else
			{
				strcat_s(error, " Global Parameters load failure, please confirm it");
				kxPrintf(KX_Err, error);
			}
			return 0;
		}

	}




	return 1;
}


int CkxEnvironment::InitAllObj(int n)
{
	if (n == 1)
	{
		Graber::g_GetGrabPack().InitCamera();
		Graber::g_GetGrabPack().Init(); 
	}	

	return 1;

}

int CkxEnvironment::ClearAllObj()
{
	Graber::g_GetGraberBuffer().Clear();
	//Config::ClearIndex();
	return 1;
}

int CkxEnvironment::StartCheck()
{
	m_bIsSimulate = false;
	Thread::g_GetThreadManage().startAllThread();
	int startstatus  = Graber::g_GetGrabPack().Start();
	
	return startstatus;
}

int CkxEnvironment::StopCheck()
{
	if (m_bIsSimulate)
	{
		Thread::g_GetThreadManage().stopSimulationThread();
		Graber::g_GetGrabPack().Stop();
		ClearAllObj();
	}
	else
	{
		Thread::g_GetThreadManage().stopAllThread();
		Graber::g_GetGrabPack().Stop();
		ClearAllObj();
	}

	return 1;
}

int CkxEnvironment::StartSimulation(std::string strPath)
{
	m_bIsSimulate = true;
	//Thread::g_GetThreadManage().startAllThread();
	//Graber::g_GetGrabPack().Start();
	//m_threadSimulation = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CkxFileRead::readAllPic, &Graber::g_GetFileRead(), strPath))); 
	Thread::g_GetThreadManage().startSimulationThread(strPath);
	return 1;
}


//读取配置
int CkxEnvironment::ReadSystemParam()
{
	char szTemp[1<<8];
	int nTemp;

	//相机参数
	//相机类型
	kxGetPrivateProfileString("Camera", "Type", "Dalsa", szTemp, sizeof(szTemp), iniPath);
	Graber::g_GetGrabPack().m_Parameter.m_nCameraType = kxCGrabPack::_CAMERA_GENIE;
	if(0 == strcmp("Dalsa", szTemp))
		Graber::g_GetGrabPack().m_Parameter.m_nCameraType = kxCGrabPack::_CAMERA_GENIE;
	else if (0 == strcmp("Hikvision", szTemp))
		Graber::g_GetGrabPack().m_Parameter.m_nCameraType = kxCGrabPack::_CAMERA_Hikvision;
	else if (0 == strcmp("DalsaP4", szTemp))
	{
		Graber::g_GetGrabPack().m_Parameter.m_nCameraType = kxCGrabPack::_CAMERA_P4;
	}
	else
		kxPrintf(KX_INFO, "未知的相机类型 %s, 已启用默认类型CAMERA_GENIE", szTemp);

	//相机模式：彩色/黑白
	kxGetPrivateProfileString("Camera", "Mode", "Color", szTemp, sizeof(szTemp), iniPath);
	Config::g_GetParameter().m_nImgType = _Type_G24;
	if(0 == strcmp(szTemp, "Gray"))
		Config::g_GetParameter().m_nImgType = _Type_G8;
	else if(0 == strcmp(szTemp, "Color"))
		Config::g_GetParameter().m_nImgType = _Type_G24;
	else if (0 == strcmp(szTemp, "P4"))
		Config::g_GetParameter().m_nImgType = _Type_G32;
	else
		kxPrintf(KX_INFO, "未知的相机模式 %s，已启用模式Color", szTemp);

	kxGetPrivateProfileString("Camera", "CardName", "Xcelera-CL+_PX8_1", Config::g_GetParameter().m_szCardName, sizeof(Config::g_GetParameter().m_szCardName), iniPath);
	kxGetPrivateProfileString("Camera", "ConfigName", ".\\P4_Default_Default.ccf", Config::g_GetParameter().m_szCfgFileName, sizeof(Config::g_GetParameter().m_szCfgFileName), iniPath);
	Config::g_GetParameter().m_nCardIndex = kxGetPrivateProfileInt("Camera", "CardIndex", 1, iniPath);
	Config::g_GetParameter().m_nIndex = kxGetPrivateProfileInt("Camera", "CardIndex", 1, iniPath);

	//相机Bayer格式
	kxGetPrivateProfileString("Camera", "BayerMode", "GRBG", szTemp, sizeof(szTemp), iniPath);
	Config::g_GetParameter().m_nBayerMode = _GRBG;
	if(0 == strcmp(szTemp, "GRBG"))
		Config::g_GetParameter().m_nBayerMode = _GRBG;
	else if(0 == strcmp(szTemp, "BGGR"))
		Config::g_GetParameter().m_nBayerMode = _BGGR;
	else if(0 == strcmp(szTemp, "GBRG"))
		Config::g_GetParameter().m_nBayerMode = _GBRG;
	else if(0 == strcmp(szTemp, "RGGB"))
		Config::g_GetParameter().m_nBayerMode = _RGGB;
	else
		kxPrintf(KX_INFO, "未知的相机Bayer模式 %s, 已启用默认GRBG模式", szTemp);

	//载入相机设备ID
	kxGetPrivateProfileString("Camera", "DeviceId", "", szTemp, sizeof(szTemp), iniPath);
	strcpy_s(g_szCameraDeviceId, sizeof(g_szCameraDeviceId), "");
	nTemp = 0;
	for(int i = 0; i < gc_nCameraMacAddress; i++)
		if(0 == strcmp(gc_szCameraMacAddress[i], szTemp))
		{
			strcpy_s(g_szCameraDeviceId, sizeof(g_szCameraDeviceId), gc_szCameraMacAddress[i]);
			nTemp = 1;
			break;
		}
	if(0 == nTemp)
		kxPrintf(KX_INFO, "相机设备ID错误 %s", szTemp);

	//其他相机参数
	kxGetPrivateProfileString("Camera", "ConfigName", ".\\T_Genie_TS-C3500_Default_Default.ccf", g_szCameraConfigPath, sizeof(g_szCameraConfigPath), iniPath);
	g_expoureTime = kxGetPrivateProfileInt("Camera", "ExpoureTime", 5000, iniPath);
	g_trigger = kxGetPrivateProfileInt("Camera", "Trigger", 0, iniPath);
	g_grabWidth = kxGetPrivateProfileInt("Camera", "GrabImageWidth", 0, iniPath);
	g_grabHeight = kxGetPrivateProfileInt("Camera", "GrabImageHeight", 0, iniPath);

	//网络参数
	int nNetPort = kxGetPrivateProfileInt("Network", "Port", 3002, iniPath);
	char szNetIp[1 << 8];
	kxGetPrivateProfileString("Network", "IpAddress", "127.0.0.1", szNetIp, sizeof(szNetIp), iniPath);
	Config::g_GetParameter().m_nNetStationId = kxGetPrivateProfileInt("Network", "NetStationId", 0, iniPath);
	Net::SetAsioTcpClient(Config::g_GetParameter().m_nNetStationId, szNetIp, nNetPort);

	//路径参数
	kxGetPrivateProfileString("Path", "NetWorkSaveImagePath", "Z:\\ColorSaveQue.dat", Config::g_GetParameter().m_szNetSaveImagePath, sizeof(Config::g_GetParameter().m_szNetSaveImagePath), iniPath);
	kxGetPrivateProfileString("Path", "NetWorkExposureSaveImagePath", "Z:\\ColorExposureSaveQue.dat", Config::g_GetParameter().m_szNetExposureSaveImagePath, sizeof(Config::g_GetParameter().m_szNetExposureSaveImagePath), iniPath);
	kxGetPrivateProfileString("Path", "NetBuildModelSaveImagePath", "Z:\\NetBuildModelSaveImagePath.dat", Config::g_GetParameter().m_szNetBuildModelSaveImagePath, sizeof(Config::g_GetParameter().m_szNetBuildModelSaveImagePath), iniPath);
	kxGetPrivateProfileString("Path", "NetDotCheckImgpath", "D:\\NetDotCheckImgpath.dat", Config::g_GetParameter().m_szNetDotCheckImgpath, sizeof(Config::g_GetParameter().m_szNetDotCheckImgpath), iniPath);


	//全局检测参数
	//Config::g_GetParameter().m_nStandardWidth	= kxGetPrivateProfileInt("GlobalParameter", "WarpWidth", 1400, iniPath);
	//Config::g_GetParameter().m_nStandardHeight	= kxGetPrivateProfileInt("GlobalParameter", "WarpHeight", 900, iniPath);
	//Config::g_GetParameter().m_nStandardPitch	= Config::g_GetParameter().m_nStandardWidth * (Config::g_GetParameter().m_nImgType ? 3 : 1);
	//Config::g_GetParameter().m_nQueSize	= kxGetPrivateProfileInt("GlobalParameter", "QueSize", 30, iniPath);
	Graber::g_GetGraberBuffer().GetParameter().m_nQueSize = kxGetPrivateProfileInt("GlobalParameter", "QueSize", 30, iniPath);


	
	Config::g_GetParameter().m_nLanguageMode = kxGetPrivateProfileInt("GlobalParameter", "LanguageMode", 1, iniPath);
	Config::g_GetParameter().m_nSendInfoInterval = kxGetPrivateProfileInt("GlobalParameter", "SendInfoInterval", 1, iniPath);
	Config::g_GetParameter().m_nSaveImgTotalCounts = kxGetPrivateProfileInt("GlobalParameter", "SaveImgTotalCounts", 1000, iniPath);
	g_SimulateTimeStep = kxGetPrivateProfileInt("GlobalParameter", "SimulateTimeStep", 200, iniPath);



	
	return 1;
}

void CkxEnvironment::kxGetPrivateProfileString(char *lpAppName, char *lpKeyName, char *lpDefault, char *lpReturnedString, int nSize, const char *lpFileName)
{
	//const char szDef[] = "Load failed";
	//char szTemp[1<<8];
	//GetPrivateProfileString(lpAppName, lpKeyName, szDef, szTemp, sizeof(szTemp), lpFileName);
	//if(0 == strcmp(szDef, szTemp))
	//{
	//	WritePrivateProfileString(lpAppName, lpKeyName, lpDefault, lpFileName);
	//	kxPrintf(KX_WARNING, "%s\\%s参数读取失败；已应用默认值 %s", lpAppName, lpKeyName, lpDefault);
	//	strcpy_s(lpReturnedString, nSize, lpDefault);
	//	return;
	//}
	//else
	//{
	//	strcpy_s(lpReturnedString, nSize, szTemp);
	//	return;
	//}

	const char szDef[] = "Load failed";
	std::string result;
	//int nstatus = ZCTinyxml2::ReadConfigXml(lpFileName, lpAppName, lpKeyName, result);
	int nstatus = KxXmlFun2::ReadConfigXml(lpFileName, lpAppName, lpKeyName, result);

	if (0 == nstatus)
	{
		//WritePrivateProfileString(lpAppName, lpKeyName, lpDefault, lpFileName);
		kxPrintf(KX_WARNING, "%s\\%s参数读取失败；已应用默认值 %s", lpAppName, lpKeyName, lpDefault);
		strcpy_s(lpReturnedString, nSize, lpDefault);
		return;
	}
	else
	{
		strcpy_s(lpReturnedString, nSize, result.c_str());
		return;
	}
}

int CkxEnvironment::kxGetPrivateProfileInt(char *lpAppName, char *lpKeyName, int nDefault, const char *lpFileName)
{
	//const int nDef = -539462008;
	//int nTemp;
	//nTemp = GetPrivateProfileInt(lpAppName, lpKeyName, nDef, lpFileName);
	//if(nDef == nTemp)
	//{
	//	char szTemp[1<<8];
	//	sprintf_s(szTemp, sizeof(szTemp), "%d", nDefault);
	//	WritePrivateProfileString(lpAppName, lpKeyName, szTemp, lpFileName);
	//	kxPrintf(KX_WARNING, "%s\\%s参数读取失败；已应用默认值 %s", lpAppName, lpKeyName, szTemp);
	//	return nDefault;
	//}
	//else
	//	return nTemp;

	std::string result;
	//int nstatus = ZCTinyxml2::ReadConfigXml(lpFileName, lpAppName, lpKeyName, result);
	int nstatus = KxXmlFun2::ReadConfigXml(lpFileName, lpAppName, lpKeyName, result);
	if (nstatus = 0)
	{
		char szTemp[1 << 8];
		sprintf_s(szTemp, sizeof(szTemp), "%d", nDefault);
		kxPrintf(KX_WARNING, "%s\\%s参数读取失败；已应用默认值 %s", lpAppName, lpKeyName, szTemp);
		return nDefault;
	}
	else
	{
		int readresult = 0;
		KxXmlFun2::FromStringToInt(result, readresult);
		return readresult;
	}
}

CkxEnvironment   g_Environment;
