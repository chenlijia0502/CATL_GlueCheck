#ifndef _KXCARDHH
#define _KXCARDHH

#include "KxAlogrithm.h"
#include "global.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/partitioner.h"
#include "kxCheckResult.h"
//#include "Result.CheckResultInfo.pb.h"
#include "BaseCheckMethod.h"
#include "json.h"
#include "GlueCheck.h"
#include "CombineImg.h"

using namespace tbb;

/*!

	Date:	2020.02.27
	Author: HYH
	Desc:	����࣬���������е��ü����㷨��checktool��Ϊ���з�������


*/


class CKxCheck
{
public:
	CKxCheck();
	~CKxCheck();
	
	enum 
	{
		_MAX_RESULT_LEN = 5,
		_MAX_BLOB_COUNT = 128,
		_MAX_AREA_COUNT = 12,
		_MAX_SEND_DEFECTLEN = 8,
	};

	enum SaveImgStatus
	{
		_DEFAULT = 0,
		_SAVEALL = 1,
		_SAVEBAD = 2,
	};

	enum
	{
		_MAX_GROUPNUM = 32,//���32��ROI
		
		_SPLICING_IMG_SCALEFACTOR = 10,//����ͼ��߷ֱ�ѹ������

	};



	struct Param
	{
		CGlueCheck::SingleParam		params[_MAX_GROUPNUM];
		int				m_nROINUM;
		int				m_nimgscalefactor;//ͼ������ϵ��
		int				m_nscantimes;//ɨ�����
		int				m_nmaxrownum; // ɨ����������м�����ROI

	};


private:

	kxCImageBuf					m_TransferImage;
	//kxCImageBuf				    m_ImgFourCornerWarp;
	kxCImageBuf                 m_SrcImg;

	kxCImageBuf                 m_BayerImg;

	kxCImageBuf                 m_DstImg;

	CKxBaseFunction				m_hBaseFun;
	
	BaseCheckMethod*			m_hCheckTools[_MAX_AREA_COUNT];
	Json::Value					m_hCheckResult[_MAX_AREA_COUNT];//�ж����ּ�鷽�����ж����ּ����


	bool						m_bCheckStatus[_MAX_AREA_COUNT];//����㷨�Ƿ�����
	//CKxSparse                   m_hSparse[_AreaCount]; // �����ж�
	KxJudgeStandard			    m_hJudgeStandard[_AreaCount];
	int							m_bJudgeStatus[KxJudgeStandard::MAX_RULE_NUM][KxJudgeStandard::MAX_DEFECT_TYPE_COUNT];//һά���жϹ��򣬶�ά��Ӧ����ȱ��
	int							m_nimgsavenum;
	char						m_csaveimgpath[1 << 8];
	SaveImgStatus				m_estatus;
	CheckResultStatus			m_finalcheckstatus;//ÿ�εĴ����������ֵ�����ڱ��滵ͼ��
	Param						m_param;
	CCombineImg					m_hcombineimg;

	kxCImageBuf					m_ImgMaxSizeA;//��ÿ�ż��ͼ��һ��Ϊһ�����ģ��Ž�ȥ��⣬��������Ƶ�������ڴ棬���ҽ��Ҳ�÷���
	kxCImageBuf					m_ImgMaxSizeB;//��ÿ�ż��ͼ��һ��Ϊһ�����ģ��Ž�ȥ��⣬��������Ƶ�������ڴ棬���ҽ��Ҳ�÷���

	kxCImageBuf					m_Savebigimg[6];
	kxCImageBuf					m_ImgSplicing;//ƴ��ͼ��ԭͼѹ��֮��ƴ��һ��
	kxCImageBuf					m_ImgResize;

private:
	//תBayerͼ��Ϊ��ɫͼ���ģ������ת��
	int TransferImage(const CKxCaptureImage& card);
	//�������
	int AnalyseCheckResult(int nCardID, Json::Value* checkresult);
	//��ս��
	int ClearResult(int nCardId);

	//����ͼƬ
	void SaveImg(CheckResultStatus status);
	
	//��ʼ��鷽��
	void InitCheckMethod();
	void ReleaseCheckMethod();

	void JudgeCheckStaus(const Json::Value& checkresult, Json::Value& sendresult);// ͨ�����ʽ�ж������

	void DotCheckImg(const kxCImageBuf& SrcImg);

	void SaveImgToPath(const kxCImageBuf& SrcImg, Json::Value& sendresult);// ���ݼ���������ݱ��浽

	void CopyImg2SplicImg(const kxCImageBuf& SrcImg, int nrow, int ncol);
	
public:
	//���һ�ſ�Ƭ��ȫ����,����Ԥ��������
	int Check(const CKxCaptureImage& card);

	//��ȡ��鹤�߾��
	inline  BaseCheckMethod& GetCheckTools(int nIndex)
	{
		return *m_hCheckTools[nIndex];
	}

	int ClearIndex();//����ĳ�reset

	bool ReadParamXml(const char* filePath, char *ErrorInfo);

	//��ȡ�зϱ�׼
	bool ReadReadJudgeStandardParaByXml(const char* lpszFile, int nIndex);
	bool ReadReadJudgeStandardParaByXmlinChinese(const char* lpszFile, int nIndex);
	bool ReadReadJudgeStandardParaByXmlinEnglish(const char* lpszFile, int nIndex);
	

	void SetSaveStatus(SaveImgStatus status, char* savepath=NULL);//HYH 2020.02.15 ���ô�ͼ״̬��Ҳ����ȡʲôͼ��
};

extern  CKxCheck   g_CheckObj;
namespace  Check
{
	inline CKxCheck &g_GetCheckCardObj()
	{
		return  g_CheckObj;
	}
}

#endif