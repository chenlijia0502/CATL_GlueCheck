#include "stdafx.h"
#include "kxCamera.h"
#include "Grab_Buffer.h"
#include "kxParameter.h"
#include "ipps.h"
#include <iostream>
#include <fstream>
using namespace std;

kxCGrabCamera::kxCGrabCamera()
{
	m_bStop = true;
	m_bInitIsOk = false;
	m_nGrabNum = 0;

	m_end = INT_MAX;
	m_start = 0;
}

kxCGrabCamera::~kxCGrabCamera()
{
}

void kxCGrabCamera::OnGrab(const unsigned char* buf, float fTime)
{//需要将buf的nSize字节 进队列
	m_start = clock();
	int nNum = 1;
	double dur = (double)(m_start - m_end);

	//Graber::g_GetGraberBuffer().Push(buf);
	Graber::g_GetGraberBuffer().Push(buf);

	IncGrab();
	m_end = m_start;
}

void kxCGrabCamera::OnGrab(const unsigned char* buf, int nW, int nH, int nChannel)
{
	Graber::g_GetGraberBuffer().Push(buf, nW, nH, nW*nChannel, nChannel);
	IncGrab();
}



int kxCGrabCamera::Init()
{
	return TRUE;
}

int kxCGrabCamera::SoftwareTrigger()
{
	return 1;

}
int kxCGrabCamera::Close()
{
	std::cout << "here" << std::endl;
	return 1;
}



int kxCGrabCamera::OpenInternalTrigger(int nStatus)
{
	return 1;
}

int kxCGrabCamera::CloseSoftwareControl()
{
	return 1;
}

int kxCGrabCamera::OpenSoftwareControl()
{
	return 1;
}


int kxCGrabCamera::SetActiveTrigger()
{
	return 1;
}

void kxCGrabCamera::Alarm(int nAlarmStatus, int nTime)
{
	
}

