#include "stdafx.h"
#include "kxcameraJAI.h"
#include "kxParameter.h"

 
 #ifdef _JAI


#define NODE_NAME_WIDTH         (int8_t*)"Width"
#define NODE_NAME_HEIGHT        (int8_t*)"Height"
#define NODE_NAME_PIXELFORMAT   (int8_t*)"PixelFormat"
#define NODE_NAME_GAIN          (int8_t*)"GainRaw"
#define NODE_NAME_EXPOSURE      (int8_t*)"ExposureTime"
#define NODE_NAME_ACQSTART      (int8_t*)"AcquisitionStart"
#define NODE_NAME_ACQSTOP       (int8_t*)"AcquisitionStop"
#define NODE_NAME_EXPOSURE_MODE (int8_t*)"ExposureMode"

 CCameraJAI::CCameraJAI()
 {
 	m_bStop = true;
	m_hCam = NULL; 
	m_hFactory = NULL; 
	m_hThread = NULL; 
 }
 CCameraJAI::~CCameraJAI()
 {
	 // Has the Camera been successfully opened?
	 Stop();
	 if (m_hCam)
	 {
		 // Close camera
		 J_Camera_Close(m_hCam);
		 m_hCam = NULL;
	 }

	 // Has the Factory been sucessfully opened?
	 if (m_hFactory)
	 {
		 // Close factory
		 J_Factory_Close(m_hFactory);
		 m_hFactory = NULL;
	 }
 }
 
 int CCameraJAI::Stop()
 {
 	if( m_bStop )
 		return 0;

	J_STATUS_TYPE retval;
	if (m_hCam)
		retval = J_Camera_ExecuteCommand(m_hCam, NODE_NAME_ACQSTOP);

	if(m_hThread)
	{
		// Close stream
		retval = J_Image_CloseStream(m_hThread);
		m_hThread = NULL;
	}

 	m_bStop = true;
 	return 1;
 }
 
 int CCameraJAI::Start()
 {
 	if( m_bStop == false )
 		return 0;
 
	J_STATUS_TYPE   retval;
	int64_t int64Val;

	SIZE	ViewSize;
	RECT	Frame;

	// Open stream
	if (m_hCam == NULL)
		return 0;

	ChangeExposureTime(Config::GetGlobalParam().m_nExpoureTime);

	// Get Width from the camera
	retval = J_Camera_GetValueInt64(m_hCam, NODE_NAME_WIDTH, &int64Val);
	ViewSize.cx = (LONG)int64Val;     // Set window size cx

	// Get Height from the camera
	retval = J_Camera_GetValueInt64(m_hCam, NODE_NAME_HEIGHT, &int64Val);
	ViewSize.cy = (LONG)int64Val;     // Set window size cy

	// Get PixelFormat from the camera
	retval = J_Camera_GetValueInt64(m_hCam, NODE_NAME_PIXELFORMAT, &int64Val);
	int pixelformat = (int)int64Val;

	// Calculate number of bits (not bytes) per pixel using macro
	int bpp = J_BitsPerPixel(pixelformat);

	retval = J_Image_OpenStream(m_hCam, 0, reinterpret_cast<J_IMG_CALLBACK_OBJECT>(this), reinterpret_cast<J_IMG_CALLBACK_FUNCTION>(&CCameraJAI::StreamCBFunc), &m_hThread, (ViewSize.cx*ViewSize.cy*bpp)/8);
	if (retval != J_ST_SUCCESS) 
	{
		printf("Could not open stream!\n");
		return FALSE;
	}

	if (m_hCam)
		retval = J_Camera_ExecuteCommand(m_hCam, NODE_NAME_ACQSTART);
 
 	m_bStop = false;
 	return 1;
 }
 
 int CCameraJAI::Init()
 {
 	m_bInitIsOk = false;
 	if (IsStop() == false)
 	{
 		printf("请先停止采集\n");
 		return 0;
 	}

	J_STATUS_TYPE   retval;
	uint32_t        iSize;
	retval = J_Factory_Open((int8_t*)"" , &m_hFactory);
	if (retval != J_ST_SUCCESS)
	{
		printf("Could not open factory!\n");
		return FALSE;
	}

	iSize = (uint32_t)sizeof(m_sCameraId);
	retval = J_Factory_GetCameraIDByIndex(m_hFactory, 1, m_sCameraId, &iSize);
	if (retval != J_ST_SUCCESS)
	{
		printf("Could not get the camera ID!\n");
		return FALSE;
	}

	// Open camera
	retval = J_Camera_Open(m_hFactory, m_sCameraId, &m_hCam);
	if (retval != J_ST_SUCCESS)
	{
		printf("Could not open the camera!\n");
		return FALSE;
	}

	//ChangeExposureTime(Config::GetGlobalParam().m_nExpoureTime);

 	m_bInitIsOk = true;
 	return TRUE;
 }

 void CCameraJAI::ChangeExposureTime(int nExpoureTime)
 {
	 J_STATUS_TYPE	retval;
	 NODE_HANDLE	hExposureNode;    // Handle to "ExposureTimeRaw" node
	 NODE_HANDLE	hNode;


	 retval = J_Camera_GetNodeByName(m_hCam, (int8_t*)"ShutterMode", &hNode);

	 // Does the "ShutterMode" node exist?
	 if ((retval == J_ST_SUCCESS) && (hNode != NULL))
	 {
		 // Here we assume that this is JAI way so we do the following:
		 // ShutterMode=ProgrammableExposure
		 // Make sure that the ShutterMode selector is set to ProgrammableExposure
		 retval = J_Camera_SetValueString(m_hCam, (int8_t*)"ShutterMode", (int8_t*)"ProgrammableExposure");
	 }

	 // Get ExposureTimeRaw Node
	 retval = J_Camera_GetNodeByName(m_hCam, NODE_NAME_EXPOSURE, &hExposureNode);
	 if (retval == J_ST_SUCCESS)
	 {
		 double fValue = nExpoureTime; 
		 retval = J_Node_SetValueDouble(hExposureNode, false, fValue);
		 //std::cout << retval << std::endl; 
		 tGenICamErrorInfo info; 
		 strcpy(info.sNodeName, NODE_NAME_EXPOSURE); 
		 retval = J_Factory_GetGenICamErrorInfo(&info); 
		 //std::cout << info.sDescription << std::endl; 
	 } 

 }
 
//#include "kxTools.h"
 void CCameraJAI::StreamCBFunc(J_tIMAGE_INFO * pAqImageInfo)
 {
	OnGrab( pAqImageInfo->pImageBuffer,NULL );    //数据 进队列
//	 g_Tool.SaveBMPImage_h("d:\\JAI.BMP", pAqImageInfo->pImageBuffer, pAqImageInfo->iSizeX, pAqImageInfo->iSizeY, pAqImageInfo->iImageSize / pAqImageInfo->iSizeY, 0); 
 }



 #else
 CCameraJAI::CCameraJAI()
 {
 }
 CCameraJAI::~CCameraJAI()
 {
 }
 
 int CCameraJAI::Stop()
 {
 	return 1;
 }
 
 int CCameraJAI::Start()
 {
 	return 1;
 }
 
 int CCameraJAI::Init()
 {
 	return TRUE;
 }

 

#endif
