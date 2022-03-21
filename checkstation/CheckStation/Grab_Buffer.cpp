#include "stdafx.h"
#include "TestAsioTcpClient.h"
#include "Grab_Buffer.h"
#include "SaveQue.h"
#include "KxCheck.h"
#include "json.h"



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

int CkxGrabBuffer:: Load( const char* lpszFile )//��ȡ�ļ�
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
			kxPrintf(KX_INFO, "��վ���ò���ͼ������ô���");
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
		kxPrintf(KX_WARNING, "����ʱ���ɼ��������");
	m_nNowID++;

	//Sleep(150);
	clock_t end = clock();
	return;
}

void CkxGrabBuffer::Push(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel)
{
	m_nNowID++;

	if (Config::g_GetParameter().m_bIsBuildModelStatus)// ��ģ״̬��ѽ��ֱ�ӷ���ȥ
	{
		kxCImageBuf recimg;

		recimg.Init(nWidth, nHeight, nChannel);

		recimg.SetImageBuf(buf, nWidth, nHeight, nPitch, nChannel, true);

		
		Json::Value sendresult;

		sendresult["imagepath"] = Config::g_GetParameter().m_szNetBuildModelSaveImagePath;

		bool m_bOpenFileStatus = g_SaveImgQueBuildModel.OpenFile(Config::g_GetParameter().m_szNetBuildModelSaveImagePath,
			recimg.nWidth, recimg.nHeight, recimg.nPitch, 500);

		if (m_bOpenFileStatus)  //�ļ��򿪳ɹ�
		{
			unsigned int nOffset;

			g_SaveImgQueBuildModel.SaveImg(recimg, nOffset);
			sendresult["startoffset"] = nOffset;
			sendresult["imageoffsetlen"] = recimg.nHeight * recimg.nPitch + 5 * 4;//�����ֵ����g_SaveImgQue.SaveImg()���洢������ƫ��+5��int
		}
		else
		{
			char word[256];
			sprintf_s(word, 256, "��ͼ·����ʧ�ܣ�%s", Config::g_GetParameter().m_szNetBuildModelSaveImagePath);
			kxPrintf(KX_Err, word);
		}

		std::string sendstr = sendresult.toStyledString();
		if (Net::IsExistNetObj())
		{
			Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_BUILD_MODEL_IMG), int(sendstr.size()), sendstr.c_str());
		}
		return;
	}
	else
	{
		if (!m_CaptureQueue.IsFull())
		{
			KxCallStatus hCall;
			hCall.Clear();
			m_CaptureQueue.GetRearElement().m_Image.SetImageBuf(buf, nWidth, nHeight, nPitch, nChannel, true, hCall);
			//m_CaptureQueue.GetRearElement().m_Image.Init(nWidth, nHeight, nChannel);
			IppiSize roiSize = { nWidth, nHeight };
			if (hCall.nCallStatus != ippStsNoErr)
			{
				kxPrintf(KX_INFO, "��վ���ò���ͼ������ô���");
			}

			if (g_bdotcheckstatus)//���,Ҳ��������ͼ���׼�
			{
				Json::Value sendresult;

				sendresult["imagepath"] = Config::g_GetParameter().m_szNetDotCheckImgpath;

				kxCImageBuf recimg;

				recimg.SetImageBuf(m_CaptureQueue.GetRearElement().m_Image);

				bool m_bOpenFileStatus = g_SaveImgQueBuildModel.OpenFile(Config::g_GetParameter().m_szNetDotCheckImgpath,
					recimg.nWidth, recimg.nHeight, recimg.nPitch, 30);

				if (m_bOpenFileStatus)  //�ļ��򿪳ɹ�
				{
					unsigned int nOffset;

					g_SaveImgQueBuildModel.SaveImg(recimg, nOffset);
					sendresult["startoffset"] = nOffset;
					sendresult["imageoffsetlen"] = recimg.nHeight * recimg.nPitch + 5 * 4;//�����ֵ����g_SaveImgQue.SaveImg()���洢������ƫ��+5��int
				}
				else
				{
					char word[256];
					sprintf_s(word, 256, "��ͼ·����ʧ�ܣ�%s", Config::g_GetParameter().m_szNetDotCheckImgpath);
					kxPrintf(KX_Err, word);
				}

				std::string sendstr = sendresult.toStyledString();
				
				if (Net::IsExistNetObj())
				{
					Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_DOT_CHECK_RESULT), int(sendstr.size()), sendstr.c_str());
				}
			
			}
			else// ����ʱ���ͼѹ������ȥ,���˿���ʵʱͼ��
			{
				Json::Value sendresult;

				sendresult["imagepath"] = Config::g_GetParameter().m_szNetExposureSaveImagePath;

				kxCImageBuf recimg;

				recimg.SetImageBuf(m_CaptureQueue.GetRearElement().m_Image, true);

				kxCImageBuf resizeimg;

				const int scalefactor = 10;

				resizeimg.Init(recimg.nWidth / scalefactor, recimg.nHeight / scalefactor, recimg.nChannel);

				m_hBaseFun.KxResizeImage(recimg, resizeimg);


				//static int nsavenum = 0;

				//char savepath[128];

				//memset(savepath, 0, 128);

				//sprintf_s(savepath, "F:\\img\\%d.bmp", nsavenum);

				//cv::Mat imgmat = cv::Mat(resizeimg.nHeight, resizeimg.nWidth, CV_8UC(resizeimg.nChannel), resizeimg.buf);

				//bool savestatus = cv::imwrite(savepath, imgmat);

				//nsavenum++;

				//std::cout << savestatus <<  "  " << nsavenum << std::endl;



				bool m_bOpenFileStatus = g_SaveImgQueExposure.OpenFile(Config::g_GetParameter().m_szNetExposureSaveImagePath,
					resizeimg.nWidth, resizeimg.nHeight, resizeimg.nPitch, 30);

				if (m_bOpenFileStatus)  //�ļ��򿪳ɹ�
				{
					unsigned int nOffset;

					g_SaveImgQueExposure.SaveImg(resizeimg, nOffset);
					sendresult["startoffset"] = nOffset;
					sendresult["imageoffsetlen"] = resizeimg.nHeight * resizeimg.nPitch + 5 * 4;//�����ֵ����g_SaveImgQue.SaveImg()���洢������ƫ��+5��int
				}
				else
				{
					char word[256];
					sprintf_s(word, 256, "��ͼ·����ʧ�ܣ�%s", Config::g_GetParameter().m_szNetExposureSaveImagePath);
					kxPrintf(KX_Err, word);
				}

				std::string sendstr = sendresult.toStyledString();

				if (Net::IsExistNetObj())
				{
					Net::GetAsioTcpClient()->SendMsg(Config::g_GetParameter().m_nNetStationId, int(MSG_SHOW_IMG), int(sendstr.size()), sendstr.c_str());
				}
			}



			m_CaptureQueue.GetRearElement().m_ImageID = m_nNowID;
			m_CaptureQueue.GetRearElement().m_CardID = m_nNowID;
			m_CaptureQueue.GetRearElement().m_Type = nChannel;
			m_CaptureQueue.Push();



		}
		else
		{
			kxPrintf(KX_WARNING, "�ɼ��������");
			//-----------TODO һ��Ϊ���Բ��֣����Զ��������ס��ͼ����ʲô��---------------//
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
	}





	return;
}


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
			//GRBGģʽ
			Byermode = ippiBayerGRBG;
		}
		else if (_BGGR == Config::g_GetParameter().m_nBayerMode)
		{
			//BGGRģʽ
			Byermode = ippiBayerBGGR;

		}
		else if (_GBRG == Config::g_GetParameter().m_nBayerMode)
		{
			//GBRGģʽ
			Byermode = ippiBayerGBRG;
		}
		else
		{
			//RGGBģʽ
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
		//			//GRBGģʽ
		//			*(pDst + 0) = *(pBayer + 1);
		//			*(pDst + 1) = (int(*(pBayer)) + int(*(pBayer + nSrcPitch + 1))) >> 1;
		//			*(pDst + 2) = *(pBayer + nSrcPitch);
		//		}
		//		else if (_BGGR == Config::g_GetParameter().m_nBayerMode)
		//		{
		//			//BGGRģʽ
		//			*(pDst + 0) = *(pBayer + nSrcPitch + 1);
		//			*(pDst + 1) = (int(*(pBayer + 1)) + int(*(pBayer + nSrcPitch))) >> 1;
		//			*(pDst + 2) = *(pBayer);
		//		}
		//		else if (_GBRG == Config::g_GetParameter().m_nBayerMode)
		//		{
		//			//GBRGģʽ
		//			*(pDst + 0) = *(pBayer + nSrcPitch);
		//			*(pDst + 1) = (int(*(pBayer)) + int(*(pBayer + nSrcPitch + 1))) >> 1;
		//			*(pDst + 2) = *(pBayer + 1);
		//		}
		//		else
		//		{
		//			//RGGBģʽ
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
