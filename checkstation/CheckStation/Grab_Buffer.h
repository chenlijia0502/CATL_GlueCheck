#pragma  once
#include "KxAlogrithm.h"
#include "global.h"
#include "kxParameter.h"
#include "kxCamera.h"

class CkxGrabBuffer //��� ���ɼ�ͼ�� �ֳ� R/G/B 
{
public:
	CkxGrabBuffer();
	~CkxGrabBuffer();

	struct Parameter
	{
		Parameter()
		{   //---��Щֻ�� һЩ�򵥵����ã���Ҫ�ڹ����о�������
			m_nWidth   = 1600;
			m_nHeight  = 1200;
			m_nQueSize = 30;
		}

		int   m_nWidth, m_nHeight;      //, m_nPitch;//һ�ν�����е�ͼ�� ��ȣ� �߶ȣ� �� Buf1 ���ö�Ӧ
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
	} //--���Ƕ����ͼ����� 

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
	void Push( const unsigned char* buf ,int nType = _Type_G8);   //�� buf�ֳ� R/G/

	void Push(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel);

	//unsigned char* GetTopImg(  int& nWidth, int& nHeight, int& nPitch ); 
	CKxCaptureImage& GetTopImg(){ return m_CaptureQueue.GetElement(0); }
	int  IsEmpty() {return m_CaptureQueue.IsEmpty();}
	///////======
	virtual int Read( FILE* fp );
	virtual int Write( FILE* fp );
	int     Load( const char* lpszFile );//��ȡ�ļ�
	int     Save( const char* lpszFile );//�����ļ�
	int     ConvertBayer2Color(const kxCImageBuf& cardImg, kxCImageBuf& dstImg);

	//kxCImageBuf  m_MidImg[_RGB_COUNT];

	//ת��ͼ
	kxCImageBuf      m_TmpImg;
	int m_nChannel;

	CKxBaseFunction   m_hBaseFun;
	int m_nNowID;

	//int            m_nIsSimulateCheck;             //��ģ����״̬
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
