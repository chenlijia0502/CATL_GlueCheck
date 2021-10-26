#pragma once

#include "kxCamera.h"

#ifdef _PointGrey

#include "FlyCapture2.h"

using namespace FlyCapture2;

class CCameraPointGrey : public kxCGrabCamera
{
public:
	CCameraPointGrey();
	virtual ~CCameraPointGrey();

	virtual int Stop();
	virtual int Start();
	virtual int Init();
	static void OnImageGrabbed(Image* pImage, const void* pCallbackData);

	//----------------------------------------------------------------------
private:
	Camera		m_hCam;
	PGRGuid		m_hGuid;



};

#else

class CCameraPointGrey : public kxCGrabCamera
{
public:
	CCameraPointGrey();
	virtual ~CCameraPointGrey();

	virtual int Stop();
	virtual int Start();
	virtual int Init();

	//----------------------------------------------------------------------
};

#endif
