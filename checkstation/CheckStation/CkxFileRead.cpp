#include "stdafx.h"
#include <boost/thread.hpp>
#include "CkxFileRead.h"
#include <io.h>
#include <algorithm>
#include "Grab_Buffer.h"

#include "opencv2/opencv.hpp"

CkxFileRead  g_FileRead;
CkxFileRead::CkxFileRead(void)
{
}

CkxFileRead::~CkxFileRead(void)
{
}

void CkxFileRead::InitSimuaStatus()
{
	m_nTotalFrame =0;
	m_nCurFrame = 0;
	m_bSimuIsChecking = false;
	m_bIsSimuReady = false;
	m_nWidth = 0;
	m_nPitch = 0;
	m_nHeight = 0;
	m_nType = 0;
	m_pData =NULL;
	m_vsFiles.clear();
}

//void CkxFileRead::readOnePic( string filepath )
//{
//	MV_IMAGE* tempImg;
//	tempImg = m_hBaseFun.LoadBMPImage_h(filepath.c_str());
//
//
//	int nChannel;
//	if (Config::g_GetParameter().m_nImgType)
//	{
//		nChannel = _Type_G24;
//	}
//	else
//	{
//		nChannel = _Type_G8;
//	}
//
//	if (nChannel != tempImg->type)
//	{
//		return ;
//	}
//	else
//	{
//		Graber::g_GetGraberBuffer().Push((unsigned char*)tempImg->data, tempImg->type);
//		//delete tempImg;
//		m_hBaseFun.FreeImage_h(tempImg);
//	}
//
//}

void CkxFileRead::readAllPic( string path )
{

	m_vsFiles.clear();
	getFiles( path, m_vsFiles );
	vector<string>::iterator iter;
	for (iter = m_vsFiles.begin(); iter != m_vsFiles.end(); iter++)
	{
		string strFileName = *iter;
		int nDotIndex = int(strFileName.find_last_of('.'));
		bool bIsBmp = false;
		if (0 <= nDotIndex && nDotIndex < strFileName.length())
		{
			string strPostfix = strFileName.substr(nDotIndex + 1, strFileName.length() - nDotIndex - 1);
			if (strPostfix == "bmp")
			{
				bIsBmp = true;
			}
		}
		if (false == bIsBmp)
		{
			continue;
		}

		//cv::Mat img = cv::imread(iter->c_str(), 1);
		//m_readImg.Init(img.cols, img.rows, img.channels());
		//m_readImg.SetImageBuf(img.data, img.cols, img.rows, img.step, img.channels(), false);
		//std::cout << iter->c_str() << std::endl;

		m_hBaseFun.LoadBMPImage_h(iter->c_str(), m_readImg);
		if (m_readImg.nWidth == 0 || m_readImg.nHeight == 0)
		{
			continue;
		}

		//if(((m_readImg.nChannel == _Type_G24) && (m_readImg.nWidth != Graber::g_GetGraberBuffer().GetParameter().m_nWidth / 2 || m_readImg.nHeight != Graber::g_GetGraberBuffer().GetParameter().m_nHeight / 2))
		//	|| ((m_readImg.nChannel == _Type_G8) && (m_readImg.nWidth != Graber::g_GetGraberBuffer().GetParameter().m_nWidth|| m_readImg.nHeight != Graber::g_GetGraberBuffer().GetParameter().m_nHeight)))
		//{
		//	kxPrintf(KX_EMERG, "子站配置参数图宽图高设置错误");
		//	return;
		//}

		//Graber::g_GetGraberBuffer().Push((unsigned char*)m_readImg.buf, m_readImg.nChannel);
		Graber::g_GetGraberBuffer().Push(m_readImg.buf, m_readImg.nWidth, m_readImg.nHeight, m_readImg.nPitch, m_readImg.nChannel);

		boost::thread::sleep(boost::get_system_time() + boost::posix_time::millisec(g_SimulateTimeStep));
	}


}

int CkxFileRead::comp(string& a, string& b)
{
	if (a.length() == b.length())
	{
		return a<b; 
	}
	if (a.length() < b.length())
	{
		return 1;
	}else
	{
		return 0;
	}
}

void CkxFileRead::getFiles( string path, vector<string>& files )  
{  
	//文件句柄  
	long long   hFile   =   0;  
	//文件信息  
	struct _finddata_t fileinfo;  
	string p;  
	if((hFile = long long(_findfirst(p.assign(path).append("\\*").c_str(),&fileinfo))) !=  -1)  
	{  
		do  
		{  
			//如果是目录,迭代之  
			//如果不是,加入列表  

			if((fileinfo.attrib & _A_SUBDIR))  
			{  
// 				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)  
// 					getFiles( p.assign(path).append("\\").append(fileinfo.name), files );  
			}  
			else  
			{  
				files.push_back(p.assign(path).append("\\").append(fileinfo.name) );  
			}  
		}while(_findnext(hFile, &fileinfo)  == 0);  
		_findclose(hFile);  
	} 
	std::sort(files.begin(),files.end(), &CkxFileRead::comp);
}  