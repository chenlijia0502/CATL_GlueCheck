/*! 
	\file KxGeneralFun.cpp
	\brief ������������

	|    ��������     |    �汾��     |               �����¼                |   �޸���    |
	|:--------------: | :-----------: | :-----------------------------------: | :---------: |
	|   2016/01/09    |     v1.0      |             ȫ�ֺ�������              |   �ͨ    |
*/
#include "stdafx.h"
#include "KxGeneralFun.h"
#include <sstream> 
#include <string> 

using namespace std;

/*!
	ִ�������в���ȡ���
	\param[in]	pszCommand �������ַ���
	\param[out] szResult   ������ִ�н��
	\param[in]	nLen       �������������
	\retval 
	+ true ����ִ�гɹ�
	+ false ����ִ��ʧ��
*/
static bool ExecCmd(const char *pszCommand, char szResult[], int nLen); 

std::string KxFun::GetErrorDescriptor(const boost::system::error_code &ec)
{
	ostringstream os; 
	os << "Code: " << ec.value() << " " << ec.message(); 
	return os.str();
}


unsigned short KxFun::csum(unsigned char *addr, int count)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	register long sum = 0;
	while (count > 1) 
	{
		/* This is the inner loop */
		sum += *(unsigned short *)addr;
		addr += 2; 
		count -= 2;
	}
	/* Add left-over byte, if any */
	if( count > 0 )
		sum += *(unsigned char *)addr;
		/* Fold 32-bit sum to 16 bits */
	while (sum>>16)
		sum = (sum & 0xffff) + (sum >> 16);
	return ~sum;
}


// ������һЩ������ͬƽ̨�µ�ʵ��
#if (defined WIN32 || defined WIN64)
#ifdef	UNICODE 
#define RECOVERY_UNICODE 
#undef	UNICODE 
#endif 
#include <stdio.h>
#include <string.h>
#include <Windows.h> 

int KxFun::IsPortBind(int nPort)
{
	char szCommnd[1024] = {0}; 
	char szResult[4096] = {0}; 
	sprintf(szCommnd, "netstat -ano | findstr 0.0.0.0:%d", nPort); 
	if (ExecCmd(szCommnd, szResult, sizeof(szResult))) 
	{
		if (strlen(szResult) > 0) 
		{
			char szText[128]; 
			char szState[32]; 
			int nPid = 0; 
			sscanf(szResult, "%s%s%s%s%d", szText, szText, szText, szState, &nPid); 
			nPid = (nPid > 0 ? nPid : -1); 
			if (strcmp(szState, "LISTENING") != 0) 
			{
				nPid = -1; 
			} 
			return nPid; 
		}
	}
	return false; 
}

bool KxFun::KillProcess(int nPid, char *pszResult/* = NULL*/, int nSize/* = 0*/)
{
	char szCommnd[1024] = {0}; 
	sprintf(szCommnd, "taskkill /f /pid %d", nPid); 
	return ExecCmd(szCommnd, pszResult, nSize); 
}

static bool ExecCmd(const char *pszCommand, char szResult[], int nLen) 
{
	memset(szResult, 0, sizeof(char) * nLen); 

	SECURITY_ATTRIBUTES sa; 
	HANDLE hRead, hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
	sa.lpSecurityDescriptor = NULL; 
	sa.bInheritHandle = TRUE; 
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) 
	{ 
		return FALSE; 
	} 

	char command[1024];    
	strcpy(command, "Cmd.exe /C "); 
	strcat(command, pszCommand); 
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
	si.cb = sizeof(STARTUPINFO); 
	GetStartupInfo(&si); 
	si.hStdError = hWrite;            //!< �Ѵ������̵ı�׼��������ض��򵽹ܵ����� 
	si.hStdOutput = hWrite;           //!< �Ѵ������̵ı�׼����ض��򵽹ܵ����� 
	si.wShowWindow = SW_HIDE; 
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; 
	// �ؼ����裬CreateProcess�����������������MSDN 
	if (!CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) 
	{ 
		CloseHandle(hWrite); 
		CloseHandle(hRead); 
		return FALSE; 
	} 
	CloseHandle(hWrite);

	char buffer[4096] = {0};          //!< ��4K�Ŀռ����洢��������ݣ�ֻҪ������ʾ�ļ����ݣ�һ��������ǹ����ˡ�
	DWORD bytesRead; 
	while (true) 
	{ 
		if (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) == NULL) 
			break; 
	} 
	if ((int)bytesRead >= nLen) 
	{
		strncpy(szResult, buffer, nLen - 1); 
	}
	else 
	{
		strcpy(szResult, buffer); 
	}

	CloseHandle(hRead); 
	return TRUE; 
}

#ifdef	RECOVERY_UNICODE
#define UNICODE
#endif

/*!
	linuxƽ̨�µĺ����汾��δʵ��
*/
#elif defined __linux___
#pragma error("UnImplement")

int KxFun::IsPortBind(int nPort)
{
	return 0; 
}

static bool ExecCmd(const char *pszCommand, char szResult[], int nLen) 
{
	return true; 
}

#endif 