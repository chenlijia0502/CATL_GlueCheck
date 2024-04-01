#pragma once

#include "stdafx.h"
#include "kxCamera.h" 

#ifdef _Baumer
#include "AcquireRoot.h" 



#include "GigEBaumerCameraDelegate.h" 

class CKxBaumerCamera : public kxCGrabCamera, public CGrabImageHook
{
public:
	CKxBaumerCamera(int nCameraId = 0);
	~CKxBaumerCamera(void);

	virtual int Stop();
	virtual BOOL Init();
	virtual int Start();
	virtual int OnFrameGrabbed( INT64 iFrameIndex , BYTE** pImageDataTab,float fTime );

	int GetCameraId() { dynamic_cast<CGigEBaumerCameraDelegate *>(m_pAcquireRoot)->GetCameraIndex(); }
private: 
	CAcquireRoot *m_pAcquireRoot; 
	ACQUIRE_FORMAT m_hAcquireFormat; 
	int m_nCameraId; 
};

#else 
class CKxBaumerCamera : public kxCGrabCamera
{
public:
	CKxBaumerCamera(int nCameraId = 0);
	~CKxBaumerCamera(void);

	virtual int Stop();
	virtual BOOL Init();
	virtual int Start();
	//virtual int OnFrameGrabbed( INT64 iFrameIndex , BYTE** pImageDataTab,float fTime );

	int GetCameraId() { return 0; }
};
#endif

