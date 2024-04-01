#include "stdafx.h"
#include "TestAsioTcpClient.h"
#include "kxCheckResult.h"
#include "kxParameter.h"

kxCheckError::kxCheckError()
{
}

kxCheckError::~kxCheckError() 
{
}  

void kxCheckError::Clear( unsigned char* buf, int pitch, int nIndex, int nStatus)
{
	m_nIndex = nIndex; 

	m_nErrCount = 0;             ///<实际错误数
	m_nStatus = nStatus;

	m_pBuf = buf;    ////////图像信息
	m_nPitch = pitch;    ////////图像信息

	for (int i = 0; i < _ERR_COUNT; i++) 
	{
		m_checkErr[i].m_ErrImg.Release(); 
		m_checkErr[i].m_ErrImg.buf = NULL;
	}

	m_nErrAreaNumber = 0;
	m_nSimErrKernNumber = 0;

	
	numImage.Release();
	dataImage.Release();
	barImage.Release();

	numImage.buf = NULL;
	dataImage.buf = NULL;
	barImage.buf = NULL;
}

void kxCheckError::Deal()
{
}

int kxCheckResult::sm_nObjNum=0;

kxCheckResult::kxCheckResult()
{
	m_nObjID = sm_nObjNum;
	sm_nObjNum++;
}

kxCheckResult::~kxCheckResult()
{
    Release();
}

int kxCheckResult::Init( int  nQueSize )
{   
	Release();

	kxCImgQue::Init( sizeof(kxCheckError),nQueSize,0 );
	MemInit(0); 
	return TRUE;
}

void kxCheckResult::Release()
{
}

void kxCheckResult::Push( PreDealImgMsg&  ImgMsg)
{
	//if (IsFull())
	//{
	//	kxPrintf(KX_WARNING, "结果队列溢出");
	//	return;
	//	Pop();
	//}
	//kxCheckError* pError = (kxCheckError*)GetRearBuf();
	//pError->Clear(ImgMsg.pBuf, ImgMsg.nPitch, ImgMsg.nIndex, ImgMsg.nStatus);
	//if (ImgMsg.nStatus != _SearchEdge_Err)
	//{
	//	clock_t start,end;
	//	start = clock();
	//	Check::g_GetCheckTools(m_nObjID).Check(ImgMsg, pError);
	//	end = clock();
	//	double dur = (double)(end - start)/CLOCKS_PER_SEC;
	//}
	//Push();
}

