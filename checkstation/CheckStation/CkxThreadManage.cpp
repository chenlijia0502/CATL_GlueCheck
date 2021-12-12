#include "stdafx.h"
#include "CkxThreadManage.h"

#include "Grab_Buffer.h"

#include "kxCheckResult.h"
#include "CkxFileRead.h"
#include "kxParameter.h"
#include "KxCheck.h"
#include "GrabPack.h"

#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "zcudpclient.h"
#include "TestAsioTcpClient.h"


using namespace tbb;

CkxThreadManage   g_ThreadManage;
boost::atomic_int	CkxThreadManage::sm_nLockFlag[_AreaCount];   
boost::mutex  CkxThreadManage::sm_mutex;

CkxThreadManage::CkxThreadManage(void)
{
	m_nIsRun = 0;
	m_IsSimulation = 0;
	for (int i=0;i<_AreaCount;i++)
	{
		m_nCurrentProcessIndex[i] = 0; 
		sm_nLockFlag[i] = _No_Deal; 
	}	
	m_IsAlarmCompleted = 0;
	m_IsAdjust = 0;
	m_IsControlAdjust = 0;
}

CkxThreadManage::~CkxThreadManage(void)
{
}

void CkxThreadManage::startAllThread()
{
	if (m_nIsRun != 1)
	{
		m_thread1.reset(new boost::thread(boost::bind(&CkxThreadManage::fun1, this)));
		m_nIsRun = 1;
	}

}

void CkxThreadManage::stopAllThread()
{
	if (m_nIsRun != 0)
	{
		m_thread1->interrupt();
		m_thread1->join();
		m_nIsRun = 0;
	}

	if (m_IsAdjust != 0)
	{
		m_threadautoadjust->interrupt();
		m_threadautoadjust->join();
		m_IsAdjust = 0;
	}

	if (m_IsSimulation != 0)
	{
		m_threadSimulation->interrupt();
		m_threadSimulation->join();
		m_IsSimulation = 0;
	}

}

void CkxThreadManage::startSimulationThread(std::string strPath)
{
	if (m_IsSimulation != 1)
	{
		m_threadSimulation.reset(new boost::thread(boost::bind(&CkxThreadManage::SimulationAllPic, this, strPath)));
		m_IsSimulation = 1;
	}

	if (m_nIsRun != 1)
	{
		m_thread1.reset(new boost::thread(boost::bind(&CkxThreadManage::fun1, this)));
		m_nIsRun = 1;
	}
}

void CkxThreadManage::stopSimulationThread()
{
	if (m_IsSimulation != 0)
	{
		m_threadSimulation->interrupt();
		m_threadSimulation->join();
		m_IsSimulation = 0;
	}
	if (m_nIsRun != 0)
	{
		m_thread1->interrupt();
		m_thread1->join();
		m_nIsRun = 0;
	}
}

void CkxThreadManage::fun1()
{
	//task_scheduler_init init;
	try
	{
		while (1)
		{
			if (!Graber::g_GetGraberBuffer().IsEmpty())
			{
				Check::g_GetCheckCardObj().Check(Graber::g_GetGraberBuffer().GetTopImg());
				Graber::g_GetGraberBuffer().Pop();
			}
            else
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(10));// 这里也是中断点
			}
			boost::this_thread::interruption_point();// 这是个中断点	
		}
	}
	catch (boost::thread_interrupted& /*e*/)
	{
		int a = 0;
		a++;
		printf("      线程1中断\n");
	}
}

void CkxThreadManage::SimulationAllPic(std::string strPath)
{
	try
	{
		Graber::g_GetFileRead().readAllPic(strPath);
	}
	catch (boost::thread_interrupted& /*e*/)
	{
		int a = 0;
		a++;
		printf("      模拟采集中断\n");
	}
}



