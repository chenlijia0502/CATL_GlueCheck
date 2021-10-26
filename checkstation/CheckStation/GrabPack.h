#pragma once
#include "kxCamera.h"
#include <string.h>
class kxCGrabPack 
{
public:
	kxCGrabPack();
	~kxCGrabPack();
	enum                  //2013-9-13 ������ý������
	{
		_CAMERA_GigE        = 0,
		_CAMERA_P30         = 1,
		_CAMERA_Chromasens   =2,
		_CAMERA_JAI = 3, 
		_CAMERA_GENIE = 4, 
		_CAMERA_PointGrey = 5,
		_CAMERA_Pylon = 6, 
		_CAMERA_Baumer = 7, 
		_CAMERA_Hikvision = 8,   //2020.08.18
		_CAMERA_P4 = 9,
		_CAMERA_Count,
	};
	struct Parameter 
	{
		Parameter()
		{
			m_nCameraType=_CAMERA_PointGrey;
		}
		int  m_nCameraType;
	};
	Parameter m_Parameter;
	void  SetCameraType(int n){m_Parameter.m_nCameraType = n;}
	int   GetCameraType(){return m_Parameter.m_nCameraType;}
	/////////=======
	kxCGrabCamera*              m_pCamera;
	kxCGrabCamera::Parameter&   GetCameraParam(){ return m_pCamera->GetParameter();}
	void                        InitCamera();
	///////=========
	kxCGrabCamera*     GetCamera() {return m_pCamera;}
	//////============
	virtual bool Read( FILE* fp );
	virtual bool Write( FILE* fp );
	bool     Load( const char* lpszFile );//��ȡ�ļ�
	bool     Save( const char* lpszFile );//�����ļ�
    int Stop();     //ֹͣ����
	int Start();   //��������
	bool Init();
	int Close();
	bool IsInitIsOk()
	{
		if(m_pCamera==NULL)
			return 0;
		return m_pCamera->IsInitIsOk();
	}
	long long  GetGrabNum()
	{
		if (m_pCamera==NULL)
		{
			return 0;
		}
		return m_pCamera->GetGrabNum();
	};
	void  SetGrabNum(int n)
	{
		if (m_pCamera)
		{
			m_pCamera->SetGrabNum(n);
		}
	}

};

extern   kxCGrabPack   g_GrabPack;
namespace Graber
{
	inline  kxCGrabPack& g_GetGrabPack()
	{
		return g_GrabPack;
	}
	inline  kxCGrabCamera* g_GetCamera(){ return g_GrabPack.GetCamera();}
// 	bool GLoad_GrabCamera();
// 	bool GSave_GrabCamera();
}
