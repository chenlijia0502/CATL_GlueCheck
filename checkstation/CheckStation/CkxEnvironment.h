#pragma once
#include <boost/thread.hpp>
#include <boost/bind.hpp>

class CkxEnvironment
{
public:
	CkxEnvironment(void);
	~CkxEnvironment(void);


	int InitAllObj(int n = 0); //0是没有相机
	int ClearAllObj();
	int StartCheck();
	int StopCheck();
	int StartSimulation(std::string strPath);


	bool		m_bIsSimulate;

public:
	boost::shared_ptr<boost::thread> m_threadSimulation;
	int ReadSystemParam();
	void kxGetPrivateProfileString(char *lpAppName, char *lpKeyName, char *lpDefault, char *lpReturnedString, int nSize, const char *lpFileName);
	int kxGetPrivateProfileInt(char *lpAppName, char *lpKeyName, int nDefault, const char *lpFileName);
	int LoadAllObjByXml(char* configPath);
};

extern  CkxEnvironment   g_Environment;

inline CkxEnvironment &g_GetEnvironment()
{
	return  g_Environment;
}