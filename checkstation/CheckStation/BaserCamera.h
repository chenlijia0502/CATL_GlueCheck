#ifdef _PYLON

#ifndef BASER_CAMERA_H
#define BASER_CAMERA_H

#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include "CameraConfiguration.h"

class CCameraPylon; 

using namespace Pylon;
using namespace GenApi;
using namespace std;
using namespace Basler_GigECameraParams;

typedef Pylon::CBaslerGigEInstantCamera Camera_t;

class CBaserCamera: public CImageEventHandler, public CConfigurationEventHandler 
{
public:
	CBaserCamera(CCameraPylon *pCameraPoly);
	~CBaserCamera(void);
	BOOL Init();
	int Stop();
	int Start();
	void Alarm(int nTime = 500);
private:
	Camera_t m_camera;
	Pylon::CTlFactory *m_pTlFactory;
	Pylon::DeviceInfoList_t m_devices;
	CGrabResultPtr m_LastGrabbedImage;
	int m_ixCamera;
protected:
	virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult);
private: 
	CCameraPylon *m_pCameraPoly; 
};


#endif
#endif