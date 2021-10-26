#include "kxCamera.h"
#include "MvCamera.h"

//#define _LMIvision

#ifdef _LMIvision
#include <GoSdk/GoSdk.h>

#define SENSOR_IP           "192.168.1.10"
#define NM_TO_MM(VALUE) (((k64f)(VALUE))/1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE))/1000.0)
#define DOUBLE_MAX              ((k64f)1.7976931348623157e+308) // 64-bit double - largest positive value.  
#define INVALID_RANGE_16BIT     ((signed short)0x8000)          // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range data. 
#define INVALID_RANGE_DOUBLE    ((k64f)-DOUBLE_MAX)             // floating point value to represent invalid range data.

typedef struct DataContext
{
	k32u count;
} DataContext;

typedef struct ProfilePoint
{
	double x;   // x-coordinate in engineering units (mm) - position along laser line
	double y;   // y-coordinate in engineering units (mm) - position along the direction of travel
	double z;   // z-coordinate in engineering units (mm) - height (at the given x position)
	unsigned char intensity;
} ProfilePoint;



class CCamera_LMI : public kxCGrabCamera
{
public:
	CCamera_LMI();
	virtual ~CCamera_LMI();

	int Stop();
	int Start();
	int Close(); //yl 2020.09.05
	BOOL Init();
	void Alarm(int nAlarmStatus, int nTime = 1000);
	void Snap();

	int SoftwareTrigger();
	int OpenInternalTrigger(int nStatus);
	int CloseSoftwareControl();
	int SetActiveTrigger();
	int OpenSoftwareControl();

	void* gethandle() { return m_camerahandle; }








	//----------------------------------------------------------------------
private:

	//static void XferCallback(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);//回调函数
	bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);//打印数据
	
	void* m_camerahandle;


	bool m_bStop;// 停止状态，true为停止
	bool m_bInitIsOk;


	kAssembly	m_hapi;
	GoSystem	m_hsystem;
	GoSensor	m_hsensor;

};

extern CCamera_LMI g_CameraHK;

namespace Graber
{
	inline CCamera_LMI & gGetGrabCamera()
	{
		return g_CameraHK;
	}
}


#else 

class CCamera_LMI : public kxCGrabCamera
{
public:
	CCamera_LMI();
	virtual ~CCamera_LMI();

	virtual int Close(); //2020.09.0-5
	virtual int Stop();
	virtual int Start();
	virtual int Init();

};
#endif