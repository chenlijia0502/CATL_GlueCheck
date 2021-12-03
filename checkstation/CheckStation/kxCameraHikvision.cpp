#include "stdafx.h"
#include "kxCameraHikvison.h"
#include "kxParameter.h"


CCamera_HK g_CameraHK;
#define _Hikvision

#ifdef _Hikvision
#pragma comment(lib,"MvCameraControl.lib")

void __stdcall XferCallback(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	if (pFrameInfo)
	{
		printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);


		// 转换图像  2020.08.28
		cv::Mat curimg;
		if (Config::g_GetParameter().m_nImgType == _Type_G24)
		{
			curimg = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
		}
		else
		{
			curimg = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1);

		}
		//cv::Mat curimg = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);

		// ch:像素格式转换 | en:Convert pixel format 
		MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };
		memset(&stConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));

		stConvertParam.nWidth = pFrameInfo->nWidth;                 //ch:图像宽 | en:image width
		stConvertParam.nHeight = pFrameInfo->nHeight;               //ch:图像高 | en:image height
		stConvertParam.pSrcData = pData;                            //ch:输入数据缓存 | en:input data buffer
		stConvertParam.nSrcDataLen = pFrameInfo->nFrameLen;         //ch:输入数据大小 | en:input data size
		stConvertParam.enSrcPixelType = pFrameInfo->enPixelType;    //ch:输入像素格式 | en:input pixel format
		//stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed; //ch:输出像素格式 | en:output pixel format
		//yl 2020.09.01 为修复在线检测输出图像颜色不对，在此修改输出像素格式
		stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed; //ch:输出像素格式 | en:output pixel format   
		stConvertParam.pDstBuffer = curimg.data;                    //ch:输出数据缓存 | en:output data buffer
		stConvertParam.nDstBufferSize = curimg.step * curimg.rows;  //ch:输出缓存大小 | en:output buffer size

		//pUser = CCamera_HK d;
		//void *handle = pUser->gethandle();
		CCamera_HK *pInfoContext = (CCamera_HK *)pUser;
		void *handle = pInfoContext->gethandle();
		int nRet = MV_CC_ConvertPixelType(handle, &stConvertParam);

		//cv::namedWindow("h", 0);
		//cv::imshow("h", curimg);
		//cv::waitKey(110);
		pInfoContext->OnGrab(curimg.data, curimg.cols, curimg.rows, curimg.channels());   //yl 2020.08.29 增加通道数
		// 把curimg塞入队列

	}
}

// 要修改配置文件路径和采集回调函数 [11/3/2015 CBPM]
CCamera_HK::CCamera_HK()
{
	m_bStop = true;
	m_camerahandle = NULL;
	//m_pSapAcq = NULL;
	//m_pSapBuffers = NULL;
	//m_pSapXfer = NULL;
	//m_pAcqDevice = NULL;
	//m_bInitIsOk = false;
	//m_bCallCompleted = true;
	//pThis = this;
}

CCamera_HK::~CCamera_HK()
{
	//DestroyObjects();
	//if (m_pSapAcq)
	//	delete m_pSapAcq;
	//if (m_pSapBuffers)
	//	delete m_pSapBuffers;
	//if (m_pSapXfer)
	//	delete m_pSapXfer;
	MV_CC_DestroyHandle(m_camerahandle);;
}



//yl 2020.09.05 因为海康相机在异常关闭的时候，下一次开启相机会出现初始化失败问题，所以添加此部分
// ch:关闭设备 | en:Close Device




int CCamera_HK::Close()
{
	if (MV_NULL == m_camerahandle)
	{
		return MV_E_HANDLE;
	}

	MV_CC_CloseDevice(m_camerahandle);

	int nRet = MV_CC_DestroyHandle(m_camerahandle);
	m_camerahandle = MV_NULL;
	return 1;
}


int CCamera_HK::Stop()
{
	if (m_bStop)
		return 0;

	int nRet = MV_CC_StopGrabbing(m_camerahandle);
	if (MV_OK == nRet || MV_OK == MV_E_CALLORDER)//临时加个调用顺序错误
	{
		m_bStop = true;
		printf("stop successed\n");
	}
	else
	{
		printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
		m_bStop = false;
	}

	return 1;
}

int CCamera_HK::Start()
{
	if (m_bStop == false || m_bInitIsOk == false)
		return 0;

	//ChangeExposureTime(Config::GetGlobelParam().m_nExpoureTime);
	//ChangeFrameRate(Config::GetGlobelParam().m_nGrabCount);
	//printf("采集张数%d\n", Config::GetGlobelParam().m_nGrabCount);

	//assert(m_pSapXfer);
	//m_pSapXfer->Grab();

	int nRet = MV_CC_StartGrabbing(m_camerahandle);
	if (MV_OK != nRet)
	{
		printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
		m_bStop = true;

	}
	else
	{
		m_bStop = false;
	}


	return 1;
}
//单位ms
void CCamera_HK::Alarm(int nAlarmStatus, int nTime)
{

}

void CCamera_HK::Snap()
{
	//if (m_pSapXfer->Snap())
	//{
	//}
}

// 根据相机的deviceID来查找和初始化 [3/7/2016 KEXIN]
BOOL CCamera_HK::Init()
{
	m_bInitIsOk = false;

	//if (IsStop() == false)
	//{
	//	kxPrintf(KX_WARNING, "相机初始化失败，请先停止采集");
	//	return m_bInitIsOk;
	//}
	int nRet = MV_OK;

	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
	if (MV_OK != nRet)
	{
		printf("Enum Devices fail! nRet [0x%x]\n", nRet);
		return m_bInitIsOk;
	}

	if (stDeviceList.nDeviceNum > 0)
	{
		for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
		{
			//printf("[device %d]:\n", i);
			MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
			if (NULL == pDeviceInfo)
			{
				break;
			}
			PrintDeviceInfo(pDeviceInfo);
		}
	}
	else
	{
		printf("Find No Devices!\n");
		return m_bInitIsOk;
	}

	//printf("Please Input camera index: 0");
	unsigned int nIndex = Config::g_GetParameter().m_nIndex;  //yl 2020.09.03 根据配置中的CardIndex选择相机
	//scanf_s("%d", &nIndex);
	if (nIndex >= stDeviceList.nDeviceNum)
	{
		printf("Input error!\n");
		return m_bInitIsOk;
	}

	// ch:选择设备并创建句柄 | Select device and create handle
	nRet = MV_CC_CreateHandle(&m_camerahandle, stDeviceList.pDeviceInfo[nIndex]);
	if (MV_OK != nRet)
	{
		printf("Create Handle fail! nRet [0x%x]\n", nRet);
		return m_bInitIsOk;
	}

	// ch:打开设备 | Open device
	nRet = MV_CC_OpenDevice(m_camerahandle);
	if (MV_OK != nRet)
	{
		printf("Open Device fail! nRet [0x%x]\n", nRet);
		return m_bInitIsOk;
	}

	// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
	if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
	{
		int nPacketSize = MV_CC_GetOptimalPacketSize(m_camerahandle);
		if (nPacketSize > 0)
		{
			nRet = MV_CC_SetIntValue(m_camerahandle, "GevSCPSPacketSize", nPacketSize);
			if (nRet != MV_OK)
			{
				printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
			}
		}
		else
		{
			printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
		}
	}

	// ch:设置触发模式为off | eb:Set trigger mode as off
	nRet = MV_CC_SetEnumValue(m_camerahandle, "TriggerMode", MV_TRIGGER_MODE_ON);//HYH 修改为外触发，由硬件控制触发
	if (MV_OK != nRet)
	{
		printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
		return m_bInitIsOk;
	}

	// ch:注册抓图回调 | en:Register image callback
	nRet = MV_CC_RegisterImageCallBackEx(m_camerahandle, XferCallback, this);
	if (MV_OK != nRet)
	{
		printf("Register Image CallBack fail! nRet [0x%x]\n", nRet);
		return m_bInitIsOk;
	}

	m_bInitIsOk = true;
	return m_bInitIsOk;
}

int CCamera_HK::SoftwareTrigger()
{
	int nRet = MV_CC_SetEnumValue(m_camerahandle, "TriggerMode", MV_TRIGGER_MODE_OFF);//HYH 修改为内触发，由硬件控制触发
	if (MV_OK != nRet)
	{
		printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
		return m_bInitIsOk;
	}
	return 0;

}

int CCamera_HK::OpenInternalTrigger(int nStatus)
{
	//yl 2020.08.19 add
	int nRet = MV_CC_SetEnumValue(m_camerahandle, "TriggerMode", nStatus);
	if (MV_OK != nRet)
	{
		printf("Open Internal Trigger fail! nRet [0x%x]\n", nRet);
		return m_bInitIsOk;
	}
	return 0;
}

int  CCamera_HK::CloseSoftwareControl()
{
	return 0;

}

int CCamera_HK::SetActiveTrigger()
{
	return 0;

}

int CCamera_HK::OpenSoftwareControl()
{
	return 0;
}



bool CCamera_HK::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
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



//void CCamera_HK::XferCallback(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
//{
//	if (pFrameInfo)
//	{
//		printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
//			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
//
//
//		//// 转换图像
//		//cv::Mat curimg;
//		//if (Config::g_GetParameter().m_nImgType == 0)
//		//{
//		//	curimg = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1);
//		//}
//		//else
//		//{
//		//	curimg = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
//
//		//}
//		cv::Mat curimg = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3);
//
//		// ch:像素格式转换 | en:Convert pixel format 
//		MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };
//		memset(&stConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
//
//		stConvertParam.nWidth = pFrameInfo->nWidth;                 //ch:图像宽 | en:image width
//		stConvertParam.nHeight = pFrameInfo->nHeight;               //ch:图像高 | en:image height
//		stConvertParam.pSrcData = pData;                            //ch:输入数据缓存 | en:input data buffer
//		stConvertParam.nSrcDataLen = pFrameInfo->nFrameLen;         //ch:输入数据大小 | en:input data size
//		stConvertParam.enSrcPixelType = pFrameInfo->enPixelType;    //ch:输入像素格式 | en:input pixel format
//		stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed; //ch:输出像素格式 | en:output pixel format
//		stConvertParam.pDstBuffer = curimg.data;                    //ch:输出数据缓存 | en:output data buffer
//		stConvertParam.nDstBufferSize = curimg.step * curimg.rows;            //ch:输出缓存大小 | en:output buffer size
//		int nRet = MV_CC_ConvertPixelType(pUser, &stConvertParam);
//
//		//OnGrab(curimg.data, curimg.cols, curimg.rows);
//		// 把curimg塞入队列
//
//	}
//}




#else
CCamera_HK::CCamera_HK()
{
}
CCamera_HK::~CCamera_HK()
{
}

int CCamera_HK::Stop()
{
	return 1;
}

int CCamera_HK::Start()
{
	return 1;
}

int CCamera_HK::Init()
{
	return TRUE;
}

int CCamera_HK::Close() //2020.09.05
{
	return 1;
}


#endif