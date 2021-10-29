#include "stdafx.h"
#include "TestAsioTcpClient.h"
#include "Grab_Buffer.h"
#include "SaveQue.h"
#include "KxCheck.h"



int CkxGrabBuffer::Read( FILE* fp )
{
	if( fread( &m_Parameter, sizeof(Parameter), 1, fp ) != 1 )
		return FALSE;
	m_nChannel = 1;
	if (Init()==FALSE)
		return FALSE;
	return TRUE;
}

int CkxGrabBuffer::Write( FILE* fp )
{
	if( fwrite( &m_Parameter, sizeof(Parameter), 1, fp ) != 1 )
		return FALSE;
	return TRUE;
}

int CkxGrabBuffer:: Load( const char* lpszFile )//读取文件
{  
	FILE*   fp;
	if( fopen_s( &fp, lpszFile, "rb" ) != 0 )
		return FALSE;
	int b = FALSE;
	b = Read( fp );
	fclose( fp );
	return b;
}

int CkxGrabBuffer::Save( const char* lpszFile )  
{
	FILE*   fp;
	if( fopen_s( &fp, lpszFile, "wb" ) != 0 )
		return FALSE;
	int b = Write( fp );
	fclose( fp );
	return b;
}

CkxGrabBuffer::CkxGrabBuffer()
{
	m_nChannel = 1;
	testnum = 0;
}

CkxGrabBuffer::~CkxGrabBuffer()
{
}

int CkxGrabBuffer::Init(bool isOnline)
{   
	if (GetImgPitch() <0 || GetImgPitch() > 100000)
		return FALSE;
	if (GetImgHeight() <0 || GetImgHeight() > 100000)
		return FALSE;
	if (m_Parameter.m_nQueSize <0 || m_Parameter.m_nQueSize > 100000)
		return FALSE;

	for(int i = 0; i < m_CaptureQueue.GetCapability(); i++)
	{
		//if(!isOnline && Config::g_GetParameter().m_nImgType == 1)
		//	m_CaptureQueue.GetElementAt(i).m_Image.Init(GetImgWidth() / 2, GetImgHeight() / 2, m_nChannel * 3);
		//else
		//	m_CaptureQueue.GetElementAt(i).m_Image.Init(GetImgWidth(), GetImgHeight(), m_nChannel);
		m_CaptureQueue.GetElementAt(i).m_ImageID = -1;
	}
	m_nNowID = 0;

	return TRUE;
}
//unsigned char* CkxGrabBuffer::GetTopImg(  int& nWidth, int& nHeight, int& nPitch )
//{
//	nWidth = GetImgWidth();
//	nHeight = GetImgHeight();
//	nPitch = GetImgPitch();
//	return m_CaptureQueue.Top().m_Image.buf;
//}

void CkxGrabBuffer::Push(const unsigned char* buf, int nType)
{
	int nSetChannel = (Config::g_GetParameter().m_nImgType == 1) ? 3 : 1;


	int nNewID;
	if(g_bIsSimulate)
		nNewID = m_nNowID + 1;
	else
		nNewID = m_nNowID + 1 - m_nNowID / _BlockImageCycle;

	if(!m_CaptureQueue.IsFull())
	{
		KxCallStatus hCall;
		hCall.Clear();
		//m_CaptureQueue.GetRearElement().m_Image.SetImageBuf(buf, m_CaptureQueue.GetRearElement().m_Image.nWidth, m_CaptureQueue.GetRearElement().m_Image.nHeight,
			//m_CaptureQueue.GetRearElement().m_Image.nPitch, m_CaptureQueue.GetRearElement().m_Image.nChannel, true, hCall);

		m_CaptureQueue.GetRearElement().m_Image.SetImageBuf(buf, m_CaptureQueue.GetRearElement().m_Image.nWidth, m_CaptureQueue.GetRearElement().m_Image.nHeight,
			m_CaptureQueue.GetRearElement().m_Image.nPitch, m_CaptureQueue.GetRearElement().m_Image.nChannel, true, hCall);


		if (hCall.nCallStatus != ippStsNoErr)
		{
			kxPrintf(KX_INFO, "子站配置参数图宽高设置错误");
		}

		if (Config::g_GetParameter().m_bChangeExpoureTimeStatus && (Config::g_GetParameter().m_nSendImageCount++ % 8))
		{
			ConvertBayer2Color(m_CaptureQueue.GetRearElement().m_Image, m_TmpImg);
			//string  szNet = m_hBaseFun.FormatImageToString(m_TmpImg);
			//m_hBaseFun.SaveBMPImage_h("d:\\123.bmp", m_TmpImg);
			unsigned int nOffset;
			if (g_SaveImgQueExposure.m_fp == NULL)
				g_SaveImgQueExposure.OpenFile(Config::g_GetParameter().m_szNetExposureSaveImagePath, m_TmpImg.nWidth, m_TmpImg.nHeight, m_TmpImg.nPitch, 10);
			g_SaveImgQueExposure.SaveImg(m_TmpImg, nOffset);

			string  szNet = m_hBaseFun.FormatIntToString(nOffset) + m_hBaseFun.FormatIntToString(m_TmpImg.nPitch) + m_hBaseFun.FormatIntToString(m_TmpImg.nHeight);
			//g_Server->SendMsg(Config::GetGlobelParam().m_nStationID, MSG_SEND_REAL_TIME_IMAGE, int(szNet.size()), szNet.c_str());

			if (Net::IsExistNetObj())
			{
				Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_SEND_REAL_TIME_IMAGE), int(szNet.size()), szNet.c_str());
			}

		}
		else if (!Config::g_GetParameter().m_bChangeExpoureTimeStatus)
		{
			m_CaptureQueue.GetRearElement().m_ImageID = m_nNowID;
			m_CaptureQueue.GetRearElement().m_CardID = nNewID;
			m_CaptureQueue.GetRearElement().m_Type = nType;
			m_CaptureQueue.Push();

		}



	}
	else
		kxPrintf(KX_WARNING, "处理超时，采集队列溢出");
	m_nNowID++;

	//Sleep(150);
	clock_t end = clock();
	return;
}

void CkxGrabBuffer::Push(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel)
{
	m_nNowID++;

	if (!m_CaptureQueue.IsFull())
	{
		KxCallStatus hCall;
		hCall.Clear();
		m_CaptureQueue.GetRearElement().m_Image.SetImageBuf(buf, nWidth, nHeight, nPitch, nChannel, true, hCall);
		//m_CaptureQueue.GetRearElement().m_Image.Init(nWidth, nHeight, nChannel);
		IppiSize roiSize = { nWidth, nHeight };
		//if (_Type_G8 == nChannel)
		//{
		//	ippiCopy_8u_C1R(buf, nPitch, m_CaptureQueue.GetRearElement().m_Image.buf, m_CaptureQueue.GetRearElement().m_Image.nPitch, roiSize);
		//}
		//else
		//{
		//	ippiCopy_8u_C3R(buf, nPitch, m_CaptureQueue.GetRearElement().m_Image.buf, m_CaptureQueue.GetRearElement().m_Image.nPitch, roiSize);
		//}
		if (hCall.nCallStatus != ippStsNoErr)
		{
			kxPrintf(KX_INFO, "子站配置参数图宽高设置错误");
		}

		if (Config::g_GetParameter().m_bChangeExpoureTimeStatus && (Config::g_GetParameter().m_nSendImageCount++ % 8))
		{
			ConvertBayer2Color(m_CaptureQueue.GetRearElement().m_Image, m_TmpImg);

			unsigned int nOffset;
			if (g_SaveImgQueExposure.m_fp == NULL)
				g_SaveImgQueExposure.OpenFile(Config::g_GetParameter().m_szNetExposureSaveImagePath, m_TmpImg.nWidth, m_TmpImg.nHeight, m_TmpImg.nPitch, 10);
			g_SaveImgQueExposure.SaveImg(m_TmpImg, nOffset);
			//auto m_fp = _fsopen("d:\\123.bmp", "wb", _SH_DENYNO);
			//m_TmpImg.Write(m_fp);
			//fclose(m_fp);

			std::ostringstream os;
			os.write(reinterpret_cast<const char *>(&Config::g_GetParameter().m_szNetExposureSaveImagePath), sizeof(char)* 256);

			if (!g_bIsSimulate)
			{
				string  szNet = m_hBaseFun.FormatIntToString(Config::g_GetParameter().m_nCurrentUIid) + m_hBaseFun.FormatIntToString(nOffset) + m_hBaseFun.FormatIntToString(m_TmpImg.nPitch) + m_hBaseFun.FormatIntToString(m_TmpImg.nHeight) + os.str();
				if (Net::IsExistNetObj())
				{
					Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_SEND_REAL_TIME_IMAGE), int(szNet.size()), szNet.c_str());
				}
			}

		}

		//if (Config::g_GetParameter().m_bChangeExpoureTimeStatus && Config::g_GetParameter().m_nGrabFlag)
		//{
		//	m_TmpImg = m_CaptureQueue.GetRearElement().m_Image;

		//	unsigned int nOffset;
		//	if (g_SaveImgQueExposure.m_fp == NULL)
		//		g_SaveImgQueExposure.OpenFile(Config::g_GetParameter().m_szNetExposureSaveImagePath, m_TmpImg.nWidth, m_TmpImg.nHeight, m_TmpImg.nPitch, 10);
		//	g_SaveImgQueExposure.SaveImg(m_TmpImg, nOffset);
		//	//auto m_fp = _fsopen("d:\\123.bmp", "wb", _SH_DENYNO);
		//	//m_TmpImg.Write(m_fp);
		//	//fclose(m_fp);

		//	std::ostringstream os;
		//	os.write(reinterpret_cast<const char *>(&Config::g_GetParameter().m_szNetExposureSaveImagePath), sizeof(char)* 256);

		//	if (!g_bIsSimulate)
		//	{
		//		string  szNet = m_hBaseFun.FormatIntToString(Config::g_GetParameter().m_nCurrentUIid) + m_hBaseFun.FormatIntToString(nOffset) + m_hBaseFun.FormatIntToString(m_TmpImg.nPitch) + m_hBaseFun.FormatIntToString(m_TmpImg.nHeight) + os.str();
		//		if (Net::IsExistNetObj())
		//		{
		//			Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_SEND_REAL_TIME_IMAGE), int(szNet.size()), szNet.c_str());
		//		}
		//	}

		//}

		//if (!Config::g_GetParameter().m_bChangeExpoureTimeStatus）//yl 2020.08.24 这里由于在开始检查进入RecMsgToStartCheck后m_bChangeExpoureTimeStatus置为false，在硬触发时未能将数据push到check，遂注释
		//if (Config::g_GetParameter().m_bChangeExpoureTimeStatus)  
		//{
		m_CaptureQueue.GetRearElement().m_ImageID = m_nNowID;
		m_CaptureQueue.GetRearElement().m_CardID = m_nNowID;
		m_CaptureQueue.GetRearElement().m_Type = nChannel;
		m_CaptureQueue.Push();
		//}
	}
	else
	{
		kxPrintf(KX_WARNING, "采集队列溢出");
		//-----------TODO 一下为测试部分，测试队列溢出卡住的图像是什么样---------------//
		CKxBaseFunction fun;
		char savedir[256];
		sprintf_s(savedir, sizeof(savedir), "d:\\wrong\\%d", Config::g_GetParameter().m_nNetStationId);
		if (_access("d:\\wrong\\", 0))
			_mkdir("d:\\wrong\\");
		if (_access(savedir, 0))
			_mkdir(savedir);
		char saveszname[256];
		sprintf_s(saveszname, sizeof(saveszname), "d:\\wrong\\%d\\%d.bmp", Config::g_GetParameter().m_nNetStationId, testnum++ % 100);
		fun.SaveBMPImage_h(saveszname, m_CaptureQueue.Top().m_Image);


	}



	return;
}

//void CkxGrabBuffer::Push(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel)
//{
//	m_nNowID++;
//	if (!m_CaptureQueue.IsFull())
//	{
//		KxCallStatus hCall;
//		hCall.Clear();
//		m_CaptureQueue.GetRearElement().m_Image.SetImageBuf(buf, nWidth, nHeight, nPitch, nChannel, true, hCall);
//		//m_CaptureQueue.GetRearElement().m_Image.Init(nWidth, nHeight, nChannel);
//		IppiSize roiSize = { nWidth, nHeight };
//		//if (_Type_G8 == nChannel)
//		//{
//		//	ippiCopy_8u_C1R(buf, nPitch, m_CaptureQueue.GetRearElement().m_Image.buf, m_CaptureQueue.GetRearElement().m_Image.nPitch, roiSize);
//		//}
//		//else
//		//{
//		//	ippiCopy_8u_C3R(buf, nPitch, m_CaptureQueue.GetRearElement().m_Image.buf, m_CaptureQueue.GetRearElement().m_Image.nPitch, roiSize);
//		//}
//		if (hCall.nCallStatus != ippStsNoErr)
//		{
//			kxPrintf(KX_INFO, "子站配置参数图宽高设置错误");
//		}
//
//		if (Config::g_GetParameter().m_bChangeExpoureTimeStatus && (Config::g_GetParameter().m_nSendImageCount++ % 8))
//		{
//			ConvertBayer2Color(m_CaptureQueue.GetRearElement().m_Image, m_TmpImg);
//
//			unsigned int nOffset;
//			if (g_SaveImgQueExposure.m_fp == NULL)
//				g_SaveImgQueExposure.OpenFile(Config::g_GetParameter().m_szNetExposureSaveImagePath, m_TmpImg.nWidth, m_TmpImg.nHeight, m_TmpImg.nPitch, 10);
//			g_SaveImgQueExposure.SaveImg(m_TmpImg, nOffset); 
//			//auto m_fp = _fsopen("d:\\123.bmp", "wb", _SH_DENYNO);
//			//m_TmpImg.Write(m_fp);
//			//fclose(m_fp);
//
//			std::ostringstream os;
//			os.write(reinterpret_cast<const char *>(&Config::g_GetParameter().m_szNetExposureSaveImagePath), sizeof(char)* 256);
//
//			if (!g_bIsSimulate)
//			{
//				string  szNet = m_hBaseFun.FormatIntToString(Config::g_GetParameter().m_nCurrentUIid) + m_hBaseFun.FormatIntToString(nOffset) + m_hBaseFun.FormatIntToString(m_TmpImg.nPitch) + m_hBaseFun.FormatIntToString(m_TmpImg.nHeight) + os.str();
//				if (Net::IsExistNetObj())
//				{
//					Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_SEND_REAL_TIME_IMAGE), int(szNet.size()), szNet.c_str());
//				}
//			}
//
//		}
//
//		////yl 2020.09.08
//		//if (Config::g_GetParameter().m_bChangeExpoureTimeStatus && !g_bIsSimulate)
//		//{
//		//	m_TmpImg = m_CaptureQueue.GetRearElement().m_Image;
//
//		//	unsigned int nOffset;
//		//	if (g_SaveImgQueExposure.m_fp == NULL)
//		//		g_SaveImgQueExposure.OpenFile(Config::g_GetParameter().m_szNetExposureSaveImagePath, m_TmpImg.nWidth, m_TmpImg.nHeight, m_TmpImg.nPitch, 10);
//		//	g_SaveImgQueExposure.SaveImg(m_TmpImg, nOffset);
//		//	//auto m_fp = _fsopen("d:\\123.bmp", "wb", _SH_DENYNO);
//		//	//m_TmpImg.Write(m_fp);
//		//	//fclose(m_fp);
//
//		//	std::ostringstream os;
//		//	os.write(reinterpret_cast<const char *>(&Config::g_GetParameter().m_szNetExposureSaveImagePath), sizeof(char)* 256);
//
//		//	if (!g_bIsSimulate)
//		//	{
//		//		string  szNet = m_hBaseFun.FormatIntToString(Config::g_GetParameter().m_nCurrentUIid) + m_hBaseFun.FormatIntToString(nOffset) + m_hBaseFun.FormatIntToString(m_TmpImg.nPitch) + m_hBaseFun.FormatIntToString(m_TmpImg.nHeight) + os.str();
//		//		if (Net::IsExistNetObj())
//		//		{
//		//			Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_SEND_REAL_TIME_IMAGE), int(szNet.size()), szNet.c_str());
//		//		}
//		//	}
//		//}
//
//		else if (!Config::g_GetParameter().m_bChangeExpoureTimeStatus)
//		{
//			m_CaptureQueue.GetRearElement().m_ImageID = m_nNowID;
//			m_CaptureQueue.GetRearElement().m_CardID = m_nNowID;
//			m_CaptureQueue.GetRearElement().m_Type = nChannel;
//			m_CaptureQueue.Push();
//		}
//
//		//}
//	}
//	else
//	{
//		kxPrintf(KX_WARNING, "采集队列溢出");
//		//-----------TODO 一下为测试部分，测试队列溢出卡住的图像是什么样---------------//
//		CKxBaseFunction fun;
//		char savedir[256];
//		sprintf_s(savedir, sizeof(savedir), "d:\\wrong\\%d", Config::g_GetParameter().m_nNetStationId);
//		if (_access("d:\\wrong\\", 0))
//			_mkdir("d:\\wrong\\");
//		if (_access(savedir, 0))
//			_mkdir(savedir);
//		char saveszname[256];
//		sprintf_s(saveszname, sizeof(saveszname), "d:\\wrong\\%d\\%d.bmp", Config::g_GetParameter().m_nNetStationId, testnum++%100);
//		fun.SaveBMPImage_h(saveszname, m_CaptureQueue.Top().m_Image);
//
//
//	}
//	
//
//
//	return;
//}

int CkxGrabBuffer::ConvertBayer2Color(const kxCImageBuf& cardImg, kxCImageBuf& dstImg)
{
	if (Config::g_GetParameter().m_nImgType == 1)
	{
		int nTransferWidth = cardImg.nWidth;
		int nTransferHeight = cardImg.nHeight;
		dstImg.Init(nTransferWidth, nTransferHeight, 3);

		unsigned char* pBayerData = const_cast<unsigned char*>(cardImg.buf);
		unsigned char* pDstData = dstImg.buf;

		int nSrcPitch = cardImg.nWidth;
		int nDstPitch = dstImg.nPitch;

		IppiRect srcRoiQ = { 0, 0, cardImg.nWidth, cardImg.nHeight };
		IppiRect dstRoiQ = { 0, 0, cardImg.nWidth, cardImg.nHeight };
		IppiSize srcSizeQ = { cardImg.nWidth, cardImg.nHeight };

		IppiBayerGrid  Byermode;
		if (_GRBG == Config::g_GetParameter().m_nBayerMode)
		{
			//GRBG模式
			Byermode = ippiBayerGRBG;
		}
		else if (_BGGR == Config::g_GetParameter().m_nBayerMode)
		{
			//BGGR模式
			Byermode = ippiBayerBGGR;

		}
		else if (_GBRG == Config::g_GetParameter().m_nBayerMode)
		{
			//GBRG模式
			Byermode = ippiBayerGBRG;
		}
		else
		{
			//RGGB模式
			Byermode = ippiBayerRGGB;
		}
		ippiCFAToRGB_8u_C1C3R(pBayerData, srcRoiQ, srcSizeQ, cardImg.nPitch, dstImg.buf, dstImg.nPitch, Byermode, 0);


		//for (int y = 0; y < nTransferHeight; y++)
		//{
		//	unsigned char* pBayer = pBayerData + y * 2 * nSrcPitch;
		//	unsigned char* pDst = pDstData + y * nDstPitch;
		//	for (int x = 0; x < nTransferWidth; x++)
		//	{
		//		if (_GRBG == Config::g_GetParameter().m_nBayerMode)
		//		{
		//			//GRBG模式
		//			*(pDst + 0) = *(pBayer + 1);
		//			*(pDst + 1) = (int(*(pBayer)) + int(*(pBayer + nSrcPitch + 1))) >> 1;
		//			*(pDst + 2) = *(pBayer + nSrcPitch);
		//		}
		//		else if (_BGGR == Config::g_GetParameter().m_nBayerMode)
		//		{
		//			//BGGR模式
		//			*(pDst + 0) = *(pBayer + nSrcPitch + 1);
		//			*(pDst + 1) = (int(*(pBayer + 1)) + int(*(pBayer + nSrcPitch))) >> 1;
		//			*(pDst + 2) = *(pBayer);
		//		}
		//		else if (_GBRG == Config::g_GetParameter().m_nBayerMode)
		//		{
		//			//GBRG模式
		//			*(pDst + 0) = *(pBayer + nSrcPitch);
		//			*(pDst + 1) = (int(*(pBayer)) + int(*(pBayer + nSrcPitch + 1))) >> 1;
		//			*(pDst + 2) = *(pBayer + 1);
		//		}
		//		else
		//		{
		//			//RGGB模式
		//			*(pDst + 0) = *(pBayer);
		//			*(pDst + 1) = (int(*(pBayer + 1)) + int(*(pBayer + nSrcPitch))) >> 1;
		//			*(pDst + 2) = *(pBayer + nSrcPitch + 1);
		//		}

		//		pDst += 3;
		//		pBayer += 2;
		//	}
		//}
	}
	else
	{
		IppiSize roiSize = { cardImg.nWidth, cardImg.nHeight };
		dstImg.Init(cardImg.nWidth, cardImg.nHeight);
		ippiCopy_8u_C1R(cardImg.buf, cardImg.nPitch, dstImg.buf, dstImg.nPitch, roiSize);
	}

	return 1;
}

CkxGrabBuffer   g_GraberBuffer;

namespace Graber
{
	int GLoad_Grabbuffer()
	{
		// 	CString   strFile=GetTemplBasicPath()+_T("Grab.dat");      
		// 	return    g_GraberBuffer.Load( strFile );   
		return TRUE;
	}

	int GSave_Grabbuffer()
	{
		// 	CString   strFile=GetTemplBasicPath()+_T("Grab.dat");      
		// 	return    g_GraberBuffer.Save( strFile );  
		return TRUE;
	}
}
