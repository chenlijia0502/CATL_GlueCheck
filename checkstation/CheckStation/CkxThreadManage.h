#pragma once
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/atomic.hpp>
#include "global.h"

using namespace std;
using std::tr1::shared_ptr;

class CkxThreadManage
{
public:
	CkxThreadManage(void);
	~CkxThreadManage(void);

	enum 
	{
		_No_Deal = -1, 
		_Dealing = 1, 
		_No_Task =-1,
	};

	void startAllThread();
	void stopAllThread();

	void startSimulationThread(std::string strPath);
	void stopSimulationThread();

	void fun1();
	void SimulationAllPic(std::string strPath);


		
public:
  	shared_ptr<boost::thread> m_thread1; 
	shared_ptr<boost::thread> m_thread2;

	int					m_nCurrentProcessIndex[_AreaCount]; 
	static boost::atomic_int	sm_nLockFlag[_AreaCount]; 
	static boost::mutex  sm_mutex;

	shared_ptr<boost::thread> m_thread3; 
	int m_nIsRun;

	shared_ptr<boost::thread> m_threadSimulation; 
	int m_IsSimulation;

	int m_IsAlarmCompleted;

	shared_ptr<boost::thread> m_threadautoadjust;
	int m_IsAdjust;//调焦线程bool量

	shared_ptr<boost::thread> m_threadcontrolautoadjust;
	int m_IsControlAdjust;
};

extern  CkxThreadManage   g_ThreadManage;
namespace  Thread
{
	inline CkxThreadManage &g_GetThreadManage()
	{
		return  g_ThreadManage;
	}
}