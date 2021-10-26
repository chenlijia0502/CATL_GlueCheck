#ifndef _SERIALPORTHH
#define _SERIALPORTHH

/*!
使用串口，必须配置serial.lib
*/
#include "stdafx.h"
#include "serial.h"
//
//class SerailCommunity{
//public:
//
//	SerailCommunity();
//	~SerailCommunity();
//	communicate::serial::Serial serials;
//};

//communicate::serial::Serial;

extern communicate::serial::Serial g_serialport;

namespace Serials
{
	inline communicate::serial::Serial &g_GetSerialObj()
	{
		//CSerialPort g_serialport;
		return g_serialport;
	}

	inline void initSerialPort(const std::string &port, uint32_t baudrate)
	{
		g_GetSerialObj().setPort(port);
		g_GetSerialObj().setBaudrate(baudrate);
	}

	inline void WriteData(const uint8_t *data, size_t size)
	{
		g_GetSerialObj().write(data, size);
	}

	inline void ReadData(uint8_t *data, size_t size)
	{
		g_GetSerialObj().read(data, size);
	}
}
#endif
