
#pragma once

#include "kxCamera.h"


#ifdef _PYLON

#include "BaserCamera.h"

class CCameraPylon : public kxCGrabCamera
{
public:
	CCameraPylon();
	virtual ~CCameraPylon();

	virtual int Start();
	virtual int Stop();
	virtual int Init();
	virtual void Alarm(int nTime = 500);

private:
	CBaserCamera m_CBaserCamera;
	bool m_bInitIsOk;
};

// extern CCameraPylon g_CameraPylon;
// 
// namespace Graber
// {
// 	inline CCameraPylon & gGetGrabCamera()
// 	{
// 		return g_CameraPylon;
// 	}
// }

#else

class CCameraPylon : public kxCGrabCamera
{
public:
	CCameraPylon();
	virtual ~CCameraPylon();

	virtual int Stop();
	virtual int Start();
	virtual int Init();
	virtual void Alarm(int nTime = 500);

	//----------------------------------------------------------------------
};


#endif