//GPTUnhandledExceptionFilter

//Code highlighting produced by Actipro CodeHighlighter (freeware)http://www.CodeHighlighter.com/-->
#include "stdafx.h"
#include "minidmp.h"

typedef void (*StationFinallyHandle)(); 
static StationFinallyHandle s_pStationFinallyHandle = NULL; 

LONG WINAPI GPTUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	//得到当前时间
	SYSTEMTIME st;
	::GetLocalTime(&st);
	//得到程序所在文件夹
	TCHAR exeFullPath[256]; // MAX_PATH
	GetModuleFileName(NULL,exeFullPath,256);//得到程序模块名称，全路径 
	char strPath[1 << 8];
	DWORD nLoc;
	sprintf(strPath, "%s", (char*)exeFullPath);
	for (nLoc = strlen(strPath) - 1; strPath[nLoc] != '\\'; nLoc--);
	strPath[nLoc + 1] = '\0';
	//eprintf("程序异常终止\n"); 

	CHAR szFileName[256];
	wsprintf(szFileName, TEXT("%sERLOG_%04d%02d%02d%02d%02d%02d%02d%02d.dmp"),strPath, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, rand()%100);
	CreateMiniDump(pExceptionInfo, szFileName);
	fprintf(stderr, "未知错误：%d\n", pExceptionInfo->ExceptionRecord->ExceptionCode); 

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
	return EXCEPTION_EXECUTE_HANDLER;    // 程序停止运行
}

void _stdcall SetStationFinallyHandle(void (*pDealHandle)())
{
	s_pStationFinallyHandle = pDealHandle; 
}
