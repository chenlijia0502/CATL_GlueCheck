#pragma once
#include "kxCamera.h"

//#undef _JAI

#ifdef _JAI

#include "Jai_Factory.h"


class CCameraJAI : public kxCGrabCamera
{
public:
	CCameraJAI();
	virtual ~CCameraJAI();

	virtual int Stop();
	virtual int Start();
	virtual int Init();
	void StreamCBFunc(J_tIMAGE_INFO * pAqImageInfo);
	void CCameraJAI::ChangeExposureTime(int nExpoureTime);
	//----------------------------------------------------------------------
private:

	FACTORY_HANDLE  m_hFactory;                         // Factory Handle
	CAM_HANDLE      m_hCam;                             // Camera Handle
	THRD_HANDLE     m_hThread;                          // Stream Channel handle
	int8_t          m_sCameraId[J_CAMERA_ID_SIZE];      // Camera ID

};

#else 

class CCameraJAI : public kxCGrabCamera
{
public:
	CCameraJAI();
	virtual ~CCameraJAI();

	virtual int Stop();
	virtual int Start();
	virtual int Init();
	//----------------------------------------------------------------------

};
#endif

