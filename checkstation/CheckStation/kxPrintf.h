#pragma once

#include "KxAlogrithm.h"

enum LogLevel
{
	//2020.02.11 为了与子站一致
	KX_INFO		= 20,
	KX_WARNING  = 30,
	KX_Err	    = 40,
};

const char gc_szLogDir[] = ".\\log";
const char gc_szLogLevel[8][8] = {"info  ", "warning", "err   "};

extern bool g_bDebug;

void kxPrintf(enum LogLevel loglevel, const char *fmt, ...);
void kxPrintf(const char *fmt, ...);
void kxIppLog(IppStatus status);
