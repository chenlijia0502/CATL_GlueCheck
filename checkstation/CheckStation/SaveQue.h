#pragma once

#include "global.h"
#include <fstream>
#include <iostream>
#include "kxParameter.h"
using namespace std;

class CSaveQue
{
public:
	CSaveQue(void);
	~CSaveQue(void);
public:
	bool OpenFile(const char* lpszFile, int nImgWidth, int nImgHeight, int nImgPitch, int nQueSize);
	bool OpenFile_Changeable(const char* lpszFile, unsigned int nTotalLen);
	bool SaveImg(kxCImageBuf& Img, unsigned int& nOffset);
	//bool OpenFile_Changeable(const char* lpszFile, __int64 nTotalLen);
	//bool SaveImg(kxCImageBuf& Img, __int64& nOffset);
	//������������ļ�
	bool OpenVariableInfoFile(const char* lpszFile);

	//������վ���ص�ƫ��ֵ��ȡͼ��
	int ReadImgFromFileByOffset(unsigned int nOffset, kxCImageBuf& DstImg);

	void closefp();

public:
	FILE*	m_fp;
	int     m_nChangeable;
	int		m_nQueSize;
	int		m_nBlockSize;
	int		m_nImgWidth;
	int		m_nImgHeight;
	int     m_nImgPitch;
	unsigned int	m_nOffset;
	unsigned int		m_nTotalLen;
	kxCImageBuf  m_TmpImg[3];
	kxCImageBuf  m_InvertImg;
	//__int64 m_nOffset;
	//__int64 m_nTotalLen;
	
	//�ɱ���Ϣ�ļ��ļ����
	ifstream  m_Variablefp; 
};
extern CSaveQue    g_SaveImgQue;
extern CSaveQue    g_SaveImgQueExposure;
extern CSaveQue    g_SaveImgQueBuildModel;