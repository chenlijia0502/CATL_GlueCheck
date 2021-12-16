#pragma  once
#include "KxAlogrithm.h"
#include "global.h"
#include "kxParameter.h"
#include "kxCamera.h"

class CkxGrabBuffer //存放 将采集图像 分成 R/G/B 
{
public:
	CkxGrabBuffer();
	~CkxGrabBuffer();

	struct Parameter
	{
		Parameter()
		{   //---这些只是 一些简单的设置，需要在工程中具体设置
			m_nWidth   = 1600;
			m_nHeight  = 1200;
			m_nQueSize = 30;
		}

		int   m_nWidth, m_nHeight;      //, m_nPitch;//一次进入队列的图像 宽度， 高度； 和 Buf1 设置对应
		int   m_nQueSize;
		int   m_nReserve[100];
	};
	Parameter       m_Parameter;
	void           SetParameter( const Parameter& para ) { m_Parameter = para; }
	Parameter& GetParameter() { return m_Parameter; }
	const Parameter& GetParameter() const { return m_Parameter; }
	int  GetImgWidth() const { return m_Parameter.m_nWidth; } 
	int  GetImgHeight() const { return m_Parameter.m_nHeight; } 
	int  GetImgPitch()
	{ 
		return GetImgWidth(); 
	} //--我们定义的图像队列 

	int  GetChanelCount()
	{
		if (Config::g_GetParameter().m_nImgType == 1)
			return 3;
		else
            return 1;
	}
	int Init(bool isOnline = true);
	void Clear(){ m_CaptureQueue.Clear();m_nNowID = 0; }
	void Pop(){m_CaptureQueue.Pop();}
	kxCQue<CKxCaptureImage, 80> m_CaptureQueue;
	void Push( const unsigned char* buf ,int nType = _Type_G8);   //将 buf分成 R/G/

	void Push(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel);

	//unsigned char* GetTopImg(  int& nWidth, int& nHeight, int& nPitch ); 
	CKxCaptureImage& GetTopImg(){ return m_CaptureQueue.GetElement(0); }
	int  IsEmpty() {return m_CaptureQueue.IsEmpty();}
	///////======
	virtual int Read( FILE* fp );
	virtual int Write( FILE* fp );
	int     Load( const char* lpszFile );//读取文件
	int     Save( const char* lpszFile );//保存文件
	int     ConvertBayer2Color(const kxCImageBuf& cardImg, kxCImageBuf& dstImg);

	//kxCImageBuf  m_MidImg[_RGB_COUNT];

	//转化图
	kxCImageBuf      m_TmpImg;
	int m_nChannel;

	CKxBaseFunction   m_hBaseFun;
	int m_nNowID;

	//int            m_nIsSimulateCheck;             //是模拟检查状态
	kxCQue<CKxCaptureImage, 30> m_AutoFocusQueue;


	int testnum;
};
extern   CkxGrabBuffer   g_GraberBuffer;
namespace Graber
{
	inline  CkxGrabBuffer& g_GetGraberBuffer()
	{
		return g_GraberBuffer;
	}
	int GLoad_Grabbuffer();
	int GSave_Grabbuffer();
}
