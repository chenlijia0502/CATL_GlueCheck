#include "StdAfx.h"
#include "SaveQue.h"


CSaveQue::CSaveQue(void)
{
	m_fp = NULL;
	m_nImgWidth = 0;
	m_nImgHeight = 0;
	m_nImgPitch = 0;
}

CSaveQue::~CSaveQue(void)
{
	if (m_fp != NULL)
		fclose(m_fp);
	if (!m_Variablefp)
		m_Variablefp.close();
}

bool CSaveQue::OpenFile(const char* lpszFile, int nImgWidth, int nImgHeight, int nImgPitch, int nQueSize)
{
	if (nImgWidth == m_nImgWidth && nImgHeight == m_nImgHeight && nImgPitch == m_nImgPitch)
	{
		return true;
	}
	else
	{
		if (m_fp == NULL)
		{
			m_fp = _fsopen(lpszFile, "wb", _SH_DENYNO);
		}
		if (m_fp == NULL)
		{
			kxPrintf(KX_Err, "打开存图路径%s失败", lpszFile);
			return false;
		}

		m_nImgWidth = nImgWidth;
		m_nImgHeight = nImgHeight;
		m_nImgPitch = nImgPitch;
		m_nQueSize = nQueSize;
		m_nBlockSize = 5 * 4 + m_nImgPitch*m_nImgHeight;
		m_nTotalLen = __int64(m_nBlockSize)*m_nQueSize;
		m_nOffset = 0;
		m_nChangeable = 0;
	}


	 return true;
}

bool CSaveQue::OpenFile_Changeable(const char* lpszFile, unsigned int nTotalLen)
//bool CSaveQue::OpenFile_Changeable(const char* lpszFile, __int64 nTotalLen)
{
	m_fp = _fsopen(lpszFile, "wb", _SH_DENYNO);
	if (m_fp == NULL)
		return false;
	m_nTotalLen = nTotalLen;
	m_nOffset = 0;
	m_nChangeable = 1;

	return true;
}

bool CSaveQue::SaveImg(kxCImageBuf& Img, unsigned int& nOffset)
//bool CSaveQue::SaveImg(kxCImageBuf& Img, __int64& nOffset)
{
	if (m_fp == NULL)
		return false;
	if ((m_nChangeable == 0) && (Img.nWidth != m_nImgWidth || Img.nHeight != m_nImgHeight || Img.nPitch != m_nImgPitch))
	{
		kxPrintf(KX_Err, "图大小(%d, %d, %d)和子站配置(%d, %d, %d)不一致，请确认子站配置", Img.nWidth, Img.nHeight, Img.nChannel, m_nImgWidth, m_nImgHeight, m_nImgPitch / m_nImgWidth);
		return false;
	}
		
	if (m_nChangeable==1)
	{
		m_nBlockSize = Img.nPitch*Img.nHeight+5*4;
		if (m_nBlockSize >= m_nTotalLen)
			return false;
	}
	if (m_nOffset+m_nBlockSize>m_nTotalLen)
	{
		m_nOffset = 0;
		_fseeki64(m_fp,m_nOffset,SEEK_SET);
	}
	nOffset = m_nOffset;

	if (Img.nChannel == 3)
	{
		int nDstOder[3] = { 2, 1, 0 };
		IppiSize Roi = { Img.nWidth, Img.nHeight };
		ippiSwapChannels_8u_C3IR(Img.buf, Img.nPitch, Roi, nDstOder);

	}
	Img.Write(m_fp);
	fflush(m_fp);

	m_nOffset = m_nOffset+m_nBlockSize;

	return true;
}


int CSaveQue::ReadImgFromFileByOffset(unsigned int nOffset, kxCImageBuf& DstImg)
{
	FILE*	pFileFp = _fsopen(Config::g_GetParameter().m_szNetSaveImagePath, "rb", _SH_DENYNO);
	if (pFileFp == NULL)
		return false;

	_fseeki64(pFileFp, nOffset, SEEK_SET);
	DstImg.Read(pFileFp);
	return 1;
}

bool CSaveQue::OpenVariableInfoFile(const char* lpszFile)
{
	if (m_Variablefp.is_open())
		m_Variablefp.close();		
	m_Variablefp.open(lpszFile);
	
	return true;
}

CSaveQue    g_SaveImgQue;
CSaveQue    g_SaveImgQueExposure;
CSaveQue    g_SaveImgQueBuildModel;