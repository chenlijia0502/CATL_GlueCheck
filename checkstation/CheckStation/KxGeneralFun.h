#pragma once 
/*! 
	\file KxGeneralFun.h
	\brief ��������ͷ�ļ�

	|    ��������     |    �汾��     |               �����¼                |   �޸���    |
	|:--------------: | :-----------: | :-----------------------------------: | :---------: |
	|   2016/01/09    |     v1.0      |            ����һЩȫ�ֺ���           |   �ͨ    |
*/

#include <boost/system/error_code.hpp>
#include <string>

namespace KxFun 
{
	/*!
	    \brief �ж϶˿��Ƿ񱻽���ռ��
		\param[in] nPort �˿ں�
		\return �����ռ�ã��򷵻�ռ�õĽ���PID�����򷵻�0
	*/
	int IsPortBind(int nPort); 

	/*!
	    \brief ɱ������
		\param[in] nPid ���̺�
		\return ����ɹ�����true�����򷵻�false
	*/
	bool KillProcess(int nPid, char *pszResult = NULL, int nSize = 0); 

	/*!
	    \brief ��ȡ��������Ӧ���ַ�������
		\param[in] ec �������
		\return ��������Ӧ���ַ�������
	*/
	std::string GetErrorDescriptor(const boost::system::error_code &ec); 
	/*!
	    \brief ����һ�����ݵ��ײ�У����
		\param[in] addr �����׵�ַ
		\param[in] count ���ݳ���
		\return ����У����
	*/
	unsigned short csum(unsigned char *addr, int count); 


}