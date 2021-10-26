//GPTUnhandledExceptionFilter

//Code highlighting produced by Actipro CodeHighlighter (freeware)http://www.CodeHighlighter.com/-->
#include "stdafx.h"
#include "minidmp.h"

typedef void (*StationFinallyHandle)(); 
static StationFinallyHandle s_pStationFinallyHandle = NULL; 

LONG WINAPI GPTUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	//�õ���ǰʱ��
	SYSTEMTIME st;
	::GetLocalTime(&st);
	//�õ����������ļ���
	TCHAR exeFullPath[256]; // MAX_PATH
	GetModuleFileName(NULL,exeFullPath,256);//�õ�����ģ�����ƣ�ȫ·�� 
	char strPath[1 << 8];
	DWORD nLoc;
	sprintf(strPath, "%s", (char*)exeFullPath);
	for (nLoc = strlen(strPath) - 1; strPath[nLoc] != '\\'; nLoc--);
	strPath[nLoc + 1] = '\0';
	//eprintf("�����쳣��ֹ\n"); 

	CHAR szFileName[256];
	wsprintf(szFileName, TEXT("%sERLOG_%04d%02d%02d%02d%02d%02d%02d%02d.dmp"),strPath, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, rand()%100);
	CreateMiniDump(pExceptionInfo, szFileName);
	fprintf(stderr, "δ֪����%d\n", pExceptionInfo->ExceptionRecord->ExceptionCode); 

	if (NULL != s_pStationFinallyHandle) 
	{
		(*s_pStationFinallyHandle)(); 
	}
	try 
	{
		exit(pExceptionInfo->ExceptionRecord->ExceptionCode);
	}
	catch (...) 
	{

	}
	return EXCEPTION_EXECUTE_HANDLER;    // ����ֹͣ����
}

void _stdcall SetStationFinallyHandle(void (*pDealHandle)())
{
	s_pStationFinallyHandle = pDealHandle; 
}
