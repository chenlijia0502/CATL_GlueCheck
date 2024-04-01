#include "stdafx.h"
#include "TestAsioTcpClient.h"
#include "kxPrintf.h"
#include "kxParameter.h"
#include <stdarg.h>
#include <stdlib.h>

bool g_bDebug = false;

void kxPrintf(enum LogLevel loglevel, const char *fmt, ...)
{
	//if(KX_DEBUG == loglevel && false == g_bDebug)
	//	return;

	//char szTime[1<<5];
	//SYSTEMTIME wtm;
	//GetLocalTime(&wtm);
	//sprintf_s(szTime, sizeof(szTime), "%04d.%02d.%02d_%02d:%02d:%02d.%03d'", wtm.wYear, wtm.wMonth, wtm.wDay, wtm.wHour, wtm.wMinute, wtm.wSecond, wtm.wMilliseconds);

	char szInfo[1024];
	memset(szInfo, 0, 1024);
	//strcpy_s(szInfo, fmt);
	va_list argptr;									//����һ��ת�������ı���
	va_start(argptr, fmt);							//��ʼ������
	vsnprintf(szInfo, sizeof(szInfo) ,fmt, argptr);	//�����������ַ������ղ����б��ʽ����buffer��
	va_end(argptr);									//���������б�,��va_start�ɶ�ʹ��

	//char szPrint[1<<10];
	//sprintf_s(szPrint, sizeof(szPrint), "%s %s %s", szTime, gc_szLogLevel[loglevel], szInfo);

	//д���ļ�
	//if(access(gc_szLogDir, 0))
	//	mkdir(gc_szLogDir);
	//char szLogPath[1<<5];
	//sprintf_s(szLogPath, sizeof(szLogPath), "%s\\%04d%02d%02d.log", gc_szLogDir, wtm.wYear, wtm.wMonth, wtm.wDay);
	//FILE *fp = fopen(szLogPath, "a");
	//fprintf_s(fp, "%s\n", szPrint);
	//fclose(fp);
	//���������̨
	printf("%s\n", szInfo);
	//���͵���վ

	if (Net::IsExistNetObj())
	{
		std::ostringstream os;
		os.write(reinterpret_cast<const char *>(&loglevel), sizeof(int));
		//int n = wtm.wYear;
		//os.write(reinterpret_cast<const char *>(&n), sizeof(int));
		//n = wtm.wMonth;
		//os.write(reinterpret_cast<const char *>(&n), sizeof(int));
		//n = wtm.wDayOfWeek;
		//os.write(reinterpret_cast<const char *>(&n), sizeof(int));
		//n = wtm.wDay;
		//os.write(reinterpret_cast<const char *>(&n), sizeof(int));
		//n = wtm.wHour;
		//os.write(reinterpret_cast<const char *>(&n), sizeof(int));
		//n = wtm.wMinute;
		//os.write(reinterpret_cast<const char *>(&n), sizeof(int));
		//n = wtm.wSecond;
		//os.write(reinterpret_cast<const char *>(&n), sizeof(int));
		//n = wtm.wMilliseconds;
		int n = sizeof(szInfo);
		os.write(reinterpret_cast<const char *>(&n), sizeof(int));
		os.write(reinterpret_cast<const char *>(&szInfo), sizeof(szInfo));
		std::string str = os.str();
		//std::cout << str.size() << std::endl;
		Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_LOG), int(str.size()), str.c_str());
	}




	return;
}

void kxPrintf(const char *fmt, ...)
{
	char szInfo[1<<10];
	va_list argptr;									//����һ��ת�������ı���
	va_start(argptr, fmt);							//��ʼ������
	vsnprintf(szInfo, sizeof(szInfo) ,fmt, argptr);	//�����������ַ������ղ����б��ʽ����buffer��
	va_end(argptr);									//���������б�,��va_start�ɶ�ʹ��

	kxPrintf(KX_INFO, "%s", szInfo);
	return;
}

void kxIppLog(IppStatus status)
{
	if(0 == status)
		return;
	if(status < 0)
	{
		kxPrintf(KX_INFO, "�ڲ�����״̬ = %d", (int)status);
		if(g_bDebug)
		{
			int *p = NULL;
			*p = NULL;
			return;
		}
	}
	if(status > 0)
		kxPrintf(KX_WARNING, "�ڲ����棬״̬ = %d", (int)status);
}

