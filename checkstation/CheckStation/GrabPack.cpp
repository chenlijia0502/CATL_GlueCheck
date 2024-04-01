#include "stdafx.h"
#include "GrabPack.h"
//#include "Camera_GigE.h"
//#include "kxCamera_PC30.h"
//#include "kxChromasens.h"
#include "CameraPylon.h"
#include "kxCameraJAI.h"
//#include "kxCamera_Genie_TS.h"
#include "kxCameraPointGrey.h"
#include "kxCameraHikvison.h"   //yl 2020.08.18
//#include "kxCamera_P4.h"



kxCGrabPack::kxCGrabPack()
{
	m_pCamera = NULL;
}
kxCGrabPack::~kxCGrabPack()
{
   if(m_pCamera)
	   delete m_pCamera;
}
void kxCGrabPack::InitCamera()
{
	if (m_pCamera)
     {
		delete  m_pCamera;
     }
	switch (GetCameraType())
     {
#ifdef _SAPCLASS
     case _CAMERA_GigE:
        m_pCamera =new CSaperaXcelera_GigE;
     	break;
	 case _CAMERA_P30:
		 m_pCamera =new CCameraPc30;
		 break;
	 case _CAMERA_Chromasens:
		 //m_pCamera =new CChromasens;
		 break;
	 case _CAMERA_JAI:
		 m_pCamera = new CCameraJAI; 
		 break;
	 case _CAMERA_GENIE:
		 m_pCamera = new CCamera_Genie_TS; 
		 break;
	 case _CAMERA_PointGrey:
		 m_pCamera = new CCameraPointGrey; 
		 break;
	 case _CAMERA_Pylon:
		 m_pCamera = new CCameraPylon; 
		 break; 
	 //case _CAMERA_Baumer:
		// m_pCamera = new CKxBaumerCamera(0); 
		// break;

	 case _CAMERA_P4:
		 m_pCamera = new CCameraP4;
		 break;
#endif
	 case _CAMERA_Hikvision:      //yl 2020.08.18
		 m_pCamera = new CCamera_HK;
		 break;
	 default :
		 //m_pCamera =new CCameraPc30;
		 m_pCamera = NULL;
		 break;
     }
  
}

bool kxCGrabPack::Read( FILE* fp )
{
	
	if( fread( &m_Parameter, sizeof(Parameter), 1, fp ) != 1 )
		return FALSE;
	InitCamera();
	if(m_pCamera)
	if( fread( &m_pCamera->GetParameter(), sizeof(kxCGrabCamera::Parameter), 1, fp ) != 1 )
		return FALSE;

	return TRUE;
}
bool kxCGrabPack::Write( FILE* fp )
{
	if( fwrite( &m_Parameter, sizeof(Parameter), 1, fp ) != 1 )
		return FALSE;
	if (m_pCamera)
	{
		if( fwrite( &m_pCamera->GetParameter(), sizeof(kxCGrabCamera::Parameter), 1, fp ) != 1 )
			return FALSE;
	}
	return TRUE;
}

bool kxCGrabPack:: Load( const char* lpszFile )//读取文件
{  
	
	//InitCamera();
	FILE*   fp;
	if( fopen_s( &fp, lpszFile, "rb" ) != 0 )
	{   
		return FALSE;
	}
	bool b = FALSE;
	b = Read( fp );
	fclose( fp );
	return b;
}

int kxCGrabPack::Stop()    //停止采样
{
	if(m_pCamera == NULL)
		return 0;
    m_pCamera->Stop();
	 return 1;

}
int kxCGrabPack::Start()
{
	if(m_pCamera == NULL)
		return 0;
	return m_pCamera->Start();
	
}
//yl 2020.09.05 增加关闭相机
int kxCGrabPack::Close()
{
	if (m_pCamera == NULL)
		return 0;
	m_pCamera->Close();
	return 1;
}
bool kxCGrabPack::Init()
{
	if(m_pCamera == NULL)
		return 0;
	int nStatus = m_pCamera->Init();
	return nStatus > 0 ? true:false;
}
bool kxCGrabPack::Save( const char* lpszFile )  
{
	FILE*   fp;
	if( fopen_s( &fp, lpszFile, "wb" ) != 0 )
	{	
		return FALSE;
	}
	bool b = Write( fp );
	fclose( fp );
	return b;
}

kxCGrabPack   g_GrabPack;
//#include "..\KxEnvironmentManager.h"
// namespace Graber
// {
// 	bool GLoad_GrabCamera()
// 	{
// 		CString   strFile=GetTemplBasicPath()+_T("GrabCamera.dat");      
// 		return   g_GetGrabPack().Load( strFile );   
// 
// 	}
// 	bool GSave_GrabCamera()
// 	{
// 		CString   strFile=GetTemplBasicPath()+_T("GrabCamera.dat");      
// 		return    g_GetGrabPack().Save( strFile );  
// 	}
// }