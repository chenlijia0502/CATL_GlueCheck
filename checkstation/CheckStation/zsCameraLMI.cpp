#include "stdafx.h"
#include "zsCameraLMI.h"
#include "kxParameter.h"



#ifdef _LMIvision

CCamera_LMI g_CameraHK;

kStatus kCall XferCallbackLMI(void* ctx, void* sys, void* dataset)
{
	unsigned int i, j;
	DataContext *context = (DataContext *)ctx;
	k32u surfaceBufferHeight = 0;

	//printf("onData Callback:\n");
	//printf("Data message received:\n");
	//printf("Dataset message count: %u\n", (k32u)GoDataSet_Count(dataset));

	for (i = 0; i < GoDataSet_Count(dataset); ++i)
	{
		GoDataMsg dataObj = GoDataSet_At(dataset, i);
		// retrieve GoStamp message
		switch (GoDataMsg_Type(dataObj))
		{
		case GO_DATA_MESSAGE_TYPE_UNIFORM_SURFACE:
		{
			GoSurfaceMsg surfaceMsg = dataObj;
			unsigned int rowIdx, colIdx;
			ProfilePoint **surfaceBuffer = NULL;

			double XResolution = NM_TO_MM(GoSurfaceMsg_XResolution(surfaceMsg));
			double YResolution = NM_TO_MM(GoSurfaceMsg_YResolution(surfaceMsg));
			double ZResolution = NM_TO_MM(GoSurfaceMsg_ZResolution(surfaceMsg));
			//double ZResolution = GoSurfaceMsg_ZResolution(surfaceMsg);

			double XOffset = UM_TO_MM(GoSurfaceMsg_XOffset(surfaceMsg));
			double YOffset = UM_TO_MM(GoSurfaceMsg_YOffset(surfaceMsg));
			double ZOffset = UM_TO_MM(GoSurfaceMsg_ZOffset(surfaceMsg));

			printf("  Surface data width: %lu\n", (k32u)GoSurfaceMsg_Width(surfaceMsg));
			printf("  Surface data length: %lu\n", (k32u)GoSurfaceMsg_Length(surfaceMsg));

			//allocate memory if needed
			if (surfaceBuffer == NULL)
			{
				surfaceBuffer = (ProfilePoint **)malloc(GoSurfaceMsg_Length(surfaceMsg) * sizeof(ProfilePoint *));
				for (j = 0; j < GoSurfaceMsg_Length(surfaceMsg); j++)
				{
					surfaceBuffer[j] = (ProfilePoint *)malloc(GoSurfaceMsg_Width(surfaceMsg) * sizeof(ProfilePoint));
				}
				surfaceBufferHeight = (k32u)GoSurfaceMsg_Length(surfaceMsg);
			}

			cv::Mat img1 = cv::Mat(GoSurfaceMsg_Length(surfaceMsg), GoSurfaceMsg_Width(surfaceMsg), CV_32FC1, cv::Scalar(0));

			for (rowIdx = 0; rowIdx < GoSurfaceMsg_Length(surfaceMsg); rowIdx++)
			{
				k16s *data = GoSurfaceMsg_RowAt(surfaceMsg, rowIdx);

				for (colIdx = 0; colIdx < GoSurfaceMsg_Width(surfaceMsg); colIdx++)
				{
					surfaceBuffer[rowIdx][colIdx].x = XOffset + XResolution * colIdx;
					surfaceBuffer[rowIdx][colIdx].y = YOffset + YResolution * rowIdx;
					if (data[colIdx] != INVALID_RANGE_16BIT)
					{
						img1.at<float>(rowIdx, colIdx) = surfaceBuffer[rowIdx][colIdx].z;
					}
					else
					{
						surfaceBuffer[rowIdx][colIdx].z = INVALID_RANGE_DOUBLE;
					}
				}
			}
		}
		break;
		}
	}
	GoDestroy(dataset);
	return kOK;
}

// 要修改配置文件路径和采集回调函数 
CCamera_LMI::CCamera_LMI()
{
	m_hapi = kNULL;
	m_hsystem = kNULL;
	m_hsensor = kNULL;
	m_bStop = true;
	m_bInitIsOk = false;
}

CCamera_LMI::~CCamera_LMI()
{
	Close();
}

int CCamera_LMI::Close()
{
	if (m_hapi != kNULL)
		GoDestroy(m_hsystem);
	if (m_hsystem = kNULL)
		GoDestroy(m_hapi);
	return 1;
}

int CCamera_LMI::Stop()
{
	if (m_bStop)
		return 0;

	int nRet = MV_CC_StopGrabbing(m_camerahandle);
	if (MV_OK == nRet || MV_OK == MV_E_CALLORDER)//临时加个调用顺序错误
	{
		m_bStop = true;
	}
	else
	{
		printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
		m_bStop = false;
	}
	kStatus status;
	// stop Gocator sensor
	if ((status = GoSystem_Stop(system)) != kOK)
	{
		m_bStop = false;
		printf("Error: GoSystem_Stop:%d\n", status);
		return 0 ;
	}
	else
	{
		m_bStop = true;
	}


	return 1;
}

int CCamera_LMI::Start()
{
	if (m_bStop == false || m_bInitIsOk == false)
		return 0;
	kStatus status;

	if ((status = GoSystem_Start(m_hsystem)) != kOK)
	{
		m_bStop = true;
		printf("Error: GoSystem_Start:%d\n", status);
		return 0;
	}
	else
	{
		m_bStop = false;
	}


	return 1;
}

void CCamera_LMI::Alarm(int nAlarmStatus, int nTime)
{

}

void CCamera_LMI::Snap()
{
}

BOOL CCamera_LMI::Init()
{
	m_bInitIsOk = false;
	

	kStatus status;
	kIpAddress ipAddress;

	DataContext contextPointer;

	// construct Gocator API Library
	if ((status = GoSdk_Construct(&m_hapi)) != kOK)
	{
		printf("Error: GoSdk_Construct:%d\n", status);
		return 0;
	}

	// construct GoSystem object
	if ((status = GoSystem_Construct(&m_hsystem, kNULL)) != kOK)
	{
		printf("Error: GoSystem_Construct:%d\n", status);
		return 0;
	}

	// Parse IP address into address data structure
	kIpAddress_Parse(&ipAddress, SENSOR_IP);

	// obtain GoSensor object by sensor IP address
	if ((status = GoSystem_FindSensorByIpAddress(system, &ipAddress, &m_hsensor)) != kOK)
	{
		printf("Error: GoSystem_FindSensor:%d\n", status);
		return 0;
	}

	// create connection to GoSystem object
	if ((status = GoSystem_Connect(system)) != kOK)
	{
		printf("Error: GoSystem_Connect:%d\n", status);
		return 0;
	}

	// enable sensor data channel
	if ((status = GoSystem_EnableData(system, kTRUE)) != kOK)
	{
		printf("Error: GoSensor_EnableData:%d\n", status);
		return 0;
	}

	// set data handler to receive data asynchronously(回调设置)
	if ((status = GoSystem_SetDataHandler(system, XferCallbackLMI, &contextPointer)) != kOK)
	{
		printf("Error: GoSystem_SetDataHandler:%d\n", status);
		return 0;
	}


	m_bInitIsOk = true;
	
	return 1;
}

int CCamera_LMI::SoftwareTrigger()
{
	return 0;

}

int CCamera_LMI::OpenInternalTrigger(int nStatus)
{
	return 0;
}

int  CCamera_LMI::CloseSoftwareControl()
{
	return 0;

}

int CCamera_LMI::SetActiveTrigger()
{
	return 0;

}

int CCamera_LMI::OpenSoftwareControl()
{
	return 0;
}

bool CCamera_LMI::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
	if (NULL == pstMVDevInfo)
	{
		printf("The Pointer of pstMVDevInfo is NULL!\n");
		return false;
	}
	if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
	{
		int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
		int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
		int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
		int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

		// ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
		//printf("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
		//printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
		printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
	}
	else
	{
		printf("Not support.\n");
	}

	return true;
}



#else
CCamera_LMI::CCamera_LMI()
{
}
CCamera_LMI::~CCamera_LMI()
{
}

int CCamera_LMI::Stop()
{
	return 1;
}

int CCamera_LMI::Start()
{
	return 1;
}

int CCamera_LMI::Init()
{
	return TRUE;
}

int CCamera_LMI::Close() //2020.09.05
{
	return 1;
}


#endif