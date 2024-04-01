#pragma once 
/*! 
	\file KxGeneralFun.h
	\brief 公共函数头文件

	|    更新日期     |    版本号     |               变更记录                |   修改人    |
	|:--------------: | :-----------: | :-----------------------------------: | :---------: |
	|   2016/01/09    |     v1.0      |            定义一些全局函数           |   侯耿通    |
*/

#include <boost/system/error_code.hpp>
#include <string>

namespace KxFun 
{
	/*!
	    \brief 判断端口是否被进程占用
		\param[in] nPort 端口号
		\return 如果被占用，则返回占用的进程PID，否则返回0
	*/
	int IsPortBind(int nPort); 

	/*!
	    \brief 杀死进程
		\param[in] nPid 进程号
		\return 如果成功返回true，否则返回false
	*/
	bool KillProcess(int nPid, char *pszResult = NULL, int nSize = 0); 

	/*!
	    \brief 获取错误代码对应的字符串描述
		\param[in] ec 错误代码
		\return 错误代码对应的字符串描述
	*/
	std::string GetErrorDescriptor(const boost::system::error_code &ec); 
	/*!
	    \brief 计算一段数据的首部校验码
		\param[in] addr 数据首地址
		\param[in] count 数据长度
		\return 数据校验码
	*/
	unsigned short csum(unsigned char *addr, int count); 


}