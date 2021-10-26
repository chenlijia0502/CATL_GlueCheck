#pragma once
#include <cstring>
#include <vector>
#include "KxBaseFunction.h"
using namespace std;

class CkxFileRead
{
public:
	CkxFileRead(void);
	~CkxFileRead(void);

	void InitSimuaStatus();
	//void readOnePic( string file );
	void readAllPic( string path );
	static int comp(string& a, string& b);
	void getFiles( string path, vector<string>& files );

public:
	int         m_nTotalFrame;
	int         m_nCurFrame;
	int        m_bSimuIsChecking;
	int        m_bIsSimuReady;
	int         m_nWidth;
	int         m_nPitch;
	int          m_nHeight;
	int          m_nType;
	unsigned char*  m_pData;

	vector<string> m_vsFiles;

	CKxBaseFunction  m_hBaseFun;
	kxCImageBuf      m_readImg;
	
};
extern CkxFileRead  g_FileRead;
namespace Graber
{
	inline  CkxFileRead& g_GetFileRead()
	{
		return g_FileRead;
	}
}