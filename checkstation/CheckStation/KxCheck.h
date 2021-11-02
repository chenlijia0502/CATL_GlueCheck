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
		_SOLVE_IMG_HEIGHT = 2000, //���δ���ͼ���С
		_SOLVE_IMG_OVERLAY = 50,  //����ͼ����ص�����
	};



	struct Param
	{
		CGlueCheck::SingleParam		params[_MAX_GROUPNUM];
		int				m_nROINUM;
	};

	struct PrepareToCheck
	{
		void clear()
		{
			m_ncopyrow = 0;
			m_nsolverowstart = 0;
			m_bCanCheck = false;
		}

		bool GetSolveImg(kxCImageBuf& dstimg)
		{
			//��m_BigImg�в��� _SOLVE_IMG_HEIGHT ��ͼ�����ڵ��δ���
			if (m_nsolverowstart >= m_BigImg.nHeight - 1)
			{
				return false;//�Ѿ��������һ��
			}
			else
			{
				if (m_nsolverowstart + _SOLVE_IMG_HEIGHT < m_BigImg.nHeight)
				{
					dstimg.SetImageBuf(m_BigImg.buf + m_nsolverowstart * m_BigImg.nPitch, m_BigImg.nWidth, 
						_SOLVE_IMG_HEIGHT, m_BigImg.nPitch, m_BigImg.nChannel, false);
				}
				else
				{
					dstimg.Init(m_BigImg.nWidth, _SOLVE_IMG_HEIGHT, m_BigImg.nChannel);

					IppiSize roisize = { m_BigImg.nWidth, _SOLVE_IMG_HEIGHT };

					Ipp8u zeroarray[3] = { 0, 0, 0 };

					ippiSet_8u_C3R(zeroarray, dstimg.buf, dstimg.nPitch, roisize);

					IppiSize copysize = { m_BigImg.nWidth, m_BigImg.nHeight - m_nsolverowstart - 1 };

					ippiCopy_8u_C3R(m_BigImg.buf + m_nsolverowstart * m_BigImg.nPitch, m_BigImg.nPitch, dstimg.buf, 
						dstimg.nPitch, copysize);

				}

				m_nsolverowstart += (_SOLVE_IMG_HEIGHT - _SOLVE_IMG_OVERLAY);// �����к���ǰ�ݽ���ͬʱ�����ص�·��


				
			}

		}
		
		kxCImageBuf		m_BigImg;
		int				m_ncopyrow;	//������
		int				m_nsolverowstart;//������ʼ��

		bool			m_bCanCheck;//�Ƿ���ͼ���Լ��

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
	PrepareToCheck				m_struct2check[_MAX_GROUPNUM];

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

	void JudgeWhichROI(const CKxCaptureImage& SrcCapImg);

	
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