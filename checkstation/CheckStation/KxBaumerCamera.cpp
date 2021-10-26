#include "stdafx.h"
#include "KxBaumerCamera.h"
#include <string> 

#ifdef _Baumer
CKxBaumerCamera::CKxBaumerCamera(int nCameraId/* = 0*/)
: m_nCameraId(nCameraId)
{
	m_pAcquireRoot = CAcquireRoot::CreateObject(std::string("CGigEBaumerCameraDelegate")); 
	memset(&m_hAcquireFormat, 0, sizeof(ACQUIRE_FORMAT)); 
}

CKxBaumerCamera::~CKxBaumerCamera(void)
{
	SAFE_DELETE(m_pAcquireRoot); 
}

int CKxBaumerCamera::Stop()
{
	return m_pAcquireRoot->StopAcquire(); 
}

BOOL CKxBaumerCamera::Init()
{
	return m_pAcquireRoot->Init(&m_hAcquireFormat, "", "", m_nCameraId, this); 
}

int CKxBaumerCamera::Start()
{
	return m_pAcquireRoot->StartAcquire(); 
}

int _stdcall CKxBaumerCamera::OnFrameGrabbed(INT64 iFrameIndex , BYTE** pImageDataTab,float fTime)
{
	kxCGrabCamera::OnGrab(*pImageDataTab, fTime); 
	printf("Grabing\n"); 
	return 0; 
}
#else
CKxBaumerCamera::CKxBaumerCamera(int nCameraId)
{

}
CKxBaumerCamera::~CKxBaumerCamera()
{
}

int CKxBaumerCamera::Stop()
{
	return 1;
}

int CKxBaumerCamera::Start()
{
	return 1;
}

int CKxBaumerCamera::Init()
{
	return 1;
}
int _stdcall CKxBaumerCamera::OnFrameGrabbed(INT64 iFrameIndex , BYTE** pImageDataTab,float fTime)
{
	return 1;
}
#endif