#include "stdafx.h"
#include "kxCameraPointGrey.h"

#ifdef _PointGrey

 CCameraPointGrey::CCameraPointGrey()
 {
 	m_bStop = true;
 
 }
 CCameraPointGrey::~CCameraPointGrey()
 {
	 Stop();
 }
 
 int CCameraPointGrey::Stop()
 {
 	if( m_bStop )
 		return 0;

	// Stop capturing images
	Error error;
	error = m_hCam.StopCapture();
	if (error != PGRERROR_OK)
	{
		//PrintError( error );
		return -1;
	}      

	// Disconnect the camera
	error = m_hCam.Disconnect();
	if (error != PGRERROR_OK)
	{
		//PrintError( error );
		return -1;
	}

 	m_bStop = true;
 	return 1;
 }
 
 int CCameraPointGrey::Start()
 {
 	if( m_bStop == false )
 		return 0;

	Error error;

	// Connect to a camera
	error = m_hCam.Connect(&m_hGuid);
	if (error != PGRERROR_OK)
	{
		//PrintError( error );
		return -1;
	}

	// Get the camera information
	CameraInfo camInfo;
	error = m_hCam.GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
		//PrintError( error );
		return -1;
	}

	//PrintCameraInfo(&camInfo);        

	// Start capturing images
	error = m_hCam.StartCapture(OnImageGrabbed, this);
	if (error != PGRERROR_OK)
	{
		//PrintError( error );
		return -1;
	}

	Property frameRateProp(FRAME_RATE);
	error = m_hCam.GetProperty(&frameRateProp);
	if (error != PGRERROR_OK)
	{
		//PrintError( error );
		return -1;
	} 
 	m_bStop = false;
 	return 1;
 }
 
 int CCameraPointGrey::Init()
 {
 	m_bInitIsOk = false;
 	if (IsStop() == false)
 	{
 		printf("请先停止采集\n");
 		return 0;
 	}

	Error error;

	BusManager busMgr;
	unsigned int numCameras;
	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		//PrintError( error );
		return -1;
	}

	printf( "Number of cameras detected: %u\n", numCameras );
	if (numCameras<1)
	{
		return -1;
	}
	
	error = busMgr.GetCameraFromIndex(0, &m_hGuid);
	if (error != PGRERROR_OK)
	{
		//PrintError( error );
		return -1;
	}
	
 	m_bInitIsOk = true;
 	return TRUE;
 }
 
#include "kxTools.h"
 void CCameraPointGrey::OnImageGrabbed(Image* pImage, const void* pCallbackData)
 {
	 CCameraPointGrey *pCamera = (CCameraPointGrey *)pCallbackData; 
	 pCamera->OnGrab( pImage->GetData(),NULL );    //数据 进队列
	 ////static int a =0;
	 ////char sz[50];
	 ////sprintf(sz,"d:\\m_ImgPre0_%d.bmp",a);
	 ////g_Tool.SaveBMPImage_h(sz, pImage->GetData(), pImage->GetCols(), pImage->GetRows(),pImage->GetCols(), 0); 
	 ////a++;
 }


 #else
 CCameraPointGrey::CCameraPointGrey()
 {
 }
 CCameraPointGrey::~CCameraPointGrey()
 {
 }
 
 int CCameraPointGrey::Stop()
 {
 	return 1;
 }
 
 int CCameraPointGrey::Start()
 {
 	return 1;
 }
 
 int CCameraPointGrey::Init()
 {
 	return TRUE;
 }

 

#endif
