#include "kxCamera.h"
#include "MvCamera.h"

#ifdef _Hikvision

class CCamera_HK : public kxCGrabCamera
{
public:
	CCamera_HK();
	virtual ~CCamera_HK();

	int Stop();
	int Start();
	int Close();
	BOOL Init();
	void Alarm(int nAlarmStatus, int nTime = 1000);
	void Snap();

	int SoftwareTrigger();
	int OpenInternalTrigger(int nStatus);
	int CloseSoftwareControl();
	int SetActiveTrigger();
	int OpenSoftwareControl();

	void* gethandle() { return m_camerahandle; }

	void ReverseScanDirection(int nStatus);


	//----------------------------------------------------------------------
private:

	//static void XferCallback(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);//�ص�����
	bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);//��ӡ����
	//	enum
	//	{
	//		SAPBUFFER_AMOUNT = 4,                  //Number of buffers
	//	};
	//
	//	static void XferCallback(SapXferCallbackInfo *pInfo);//��������Ļص�����
	//
	//
	//	SapAcquisition*    m_pSapAcq;
	//	SapBuffer*         m_pSapBuffers;
	//	SapTransfer*       m_pSapXfer;
	//	SapAcqDevice       *m_pAcqDevice;
	//	SapGio*  m_pSapGio;
	//
	//	// HYH 2019.07.17 ��ʱ�������
	//	SapAcqDevice       *m_pcamera;
	//
	//
	//
	//private:
	//	void DestroyObjects();
	//	bool CreateObjects();
	//public:
	//	bool ChangeExposureTime(double dlbTime);
	//	bool ChangeAcquisitionLineRate(int nRate);
	//	void CCamera_Genie_TS::setOutputValue();
	//	int CCamera_Genie_TS::ChangeFrameRate(int nFramRate);
	//
	//	static CCamera_Genie_TS * pThis;
	//
	//	int m_nImageW;
	//	int m_nImageH;
	//	int m_bCallCompleted;
	void* m_camerahandle;


	bool m_bStop;
	bool m_bInitIsOk;
};

extern CCamera_HK g_CameraHK;

namespace Graber
{
	inline CCamera_HK & gGetGrabCamera()
	{
		return g_CameraHK;
	}
}


#else 

class CCamera_HK : public kxCGrabCamera
{
public:
	CCamera_HK();
	virtual ~CCamera_HK();

	virtual int Close(); //2020.09.0-5
	virtual int Stop();
	virtual int Start();
	virtual int Init();

};
#endif