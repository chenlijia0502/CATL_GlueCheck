#include "stdafx.h"
#include "CameraPylon.h"

/*CCameraPylon g_CameraPylon;*/

#ifdef _PYLON

//#ifdef _pylon

CCameraPylon::CCameraPylon(void)
: m_CBaserCamera(this)
{
	m_bInitIsOk = false;
}

CCameraPylon::~CCameraPylon(void)
{
	;
}

int CCameraPylon::Init()
{
	m_bInitIsOk = false;
	if( m_CBaserCamera.Init() )
	{
		m_bInitIsOk = true;
	}
	return true;
}

int CCameraPylon::Start()
{
	if(m_CBaserCamera.Start())
	{
		return true;
	}
	return false;
}

int CCameraPylon::Stop()
{
	if( m_CBaserCamera.Stop() )
	{
		return true;
	}
	return false;
}

void CCameraPylon::Alarm(int nTime)
{
	m_CBaserCamera.Alarm(nTime);
}

#else

CCameraPylon::CCameraPylon()
{
	m_bInitIsOk = false;
}

CCameraPylon::~CCameraPylon()
{
	;
}

int CCameraPylon::Init()
{
	return 1;
}

int  CCameraPylon::Start()
{
	return 1;
}

int CCameraPylon::Stop()
{
	return 1;
}

void CCameraPylon::Alarm(int nTime)
{
	return ;
	
}

//#endif

#endif