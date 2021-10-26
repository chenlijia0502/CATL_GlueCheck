#include "stdafx.h"
#include "kxParameter.h"

#ifdef _PYLON


#include "BaserCamera.h"
#include "Grab_Buffer.h"
#include "CameraPylon.h" 

CBaserCamera::CBaserCamera(CCameraPylon *pCameraPoly)
{
	m_camera.RegisterConfiguration( new CAcquireContinuousConfiguration, RegistrationMode_ReplaceAll, Ownership_TakeOwnership);
	m_camera.RegisterConfiguration( new CameraConfiguration, RegistrationMode_Append, Ownership_TakeOwnership);
	m_camera.RegisterConfiguration( this, RegistrationMode_Append, Ownership_ExternalOwnership);
	m_camera.RegisterImageEventHandler( this, RegistrationMode_Append, Ownership_ExternalOwnership);	
	m_pTlFactory = NULL;
	m_pCameraPoly = pCameraPoly; 
}

CBaserCamera::~CBaserCamera(void)
{
	m_camera.DestroyDevice();
	m_LastGrabbedImage.Release();
}

void CBaserCamera::Alarm(int nTime )
{
	m_camera.UserOutputSelector.SetValue(UserOutputSelector_UserOutput1);
	m_camera.UserOutputValue.SetValue(true);
	Sleep(nTime);
	m_camera.UserOutputValue.SetValue(false);
}

BOOL CBaserCamera::Init()
{
	m_pTlFactory = &CTlFactory::GetInstance();
	if ( m_pTlFactory == NULL )
	{
		return FALSE;
	}

	m_LastGrabbedImage.Release();
	m_devices.clear();
	m_pTlFactory->EnumerateDevices(m_devices);
	
	if ( m_devices.empty() )
	{
	  // err "No camera present!";
	  return FALSE;
	}

	m_camera.DestroyDevice();
	m_ixCamera = 0;
	m_camera.Attach( m_pTlFactory->CreateDevice(m_devices[m_ixCamera]) );		
	m_camera.Open();
	m_camera.MaxNumBuffer = 50;
	m_camera.GetTLParams().HeartbeatTimeout.SetValue( 1000 );
	m_camera.UserSetSelector.SetValue( UserSetSelector_UserSet1 );
	m_camera.UserSetLoad.Execute(true); 

	//Alarm(20);//by cm
// 	m_camera.PixelFormat.SetValue(PixelFormat_Mono8);
// 	m_camera.Width.SetValue( 1600 );
// 	m_camera.Height.SetValue( 1200 );
// 
//  	m_camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
// 	m_camera.TriggerDelayAbs.SetValue(450.0);
//  	m_camera.TriggerMode.SetValue(TriggerMode_On);
//  	m_camera.TriggerSource.SetValue(TriggerSource_Line1);
// 	/*m_camera.ExposureTimeAbs.SetValue(50.0);*/
// 	m_camera.ExposureTimeRaw.SetValue(150);
// // 	m_camera.AcquisitionFrameRateEnable.SetValue(true);
// // 	m_camera.AcquisitionFrameRateAbs.SetValue(30.0003);

//	m_camera.Close();//  [3/12/2015 caomao]
	return TRUE;
}

int CBaserCamera::Start()
{
	if (m_camera.IsPylonDeviceAttached()) 
	{
		m_camera.ExposureTimeRaw.SetValue(Config::GetGlobalParam().m_nExpoureTime);

		m_camera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
		return 1;
	}
	return 0;
}

int CBaserCamera::Stop()
{
	if (m_camera.IsPylonDeviceAttached()) 
	{
		m_camera.StopGrabbing();
		return 1;
	}
	return 0;
}

void CBaserCamera::OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult) 
{
	if (ptrGrabResult->GrabSucceeded())
	{
		m_LastGrabbedImage = ptrGrabResult;

		//Push Buffer
		//Graber::g_GetGraberBuffer().Push((unsigned char *)m_LastGrabbedImage->GetBuffer(), 0);
		m_pCameraPoly->OnGrab((unsigned char *)m_LastGrabbedImage->GetBuffer(), 0); 
		m_LastGrabbedImage.Release();
	}
	else
	{
		if ( !camera.IsCameraDeviceRemoved())
		{
			// err
		}
	}
}

#endif