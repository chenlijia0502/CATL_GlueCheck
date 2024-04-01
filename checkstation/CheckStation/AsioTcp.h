#pragma once
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <sstream>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/queue.hpp>

typedef boost::asio::ip::tcp::socket socket_t;
typedef void* socket_handle;

#pragma pack(push, 1)

//#define ASYNC_WRITE 

//< ���ݰ�ͷ��ʽ
typedef struct tagDataHeader 
{
	int nStationID;		// վ��ID
	int nMsgType;		// ���ݰ���Ϣ����
	int nExtDataSize;	// �������ݳ���
} DataHeader, NetDataHeader;

enum MSG_TYPE_DEFINE 
{
	USER_MSG_DEFINE_START = 1, 
};

#pragma pack(pop) 

//< �����ص���
class INetCallback
{
public:
	virtual void OnNewConnection(socket_handle newSocket) = 0;	// ���������Ӻ�Ļص�����
	virtual void OnRecvData(socket_handle socket, const unsigned char* pData, size_t nDataSize) = 0; // ���յ����ݺ�Ļص�����
};

// TCP�Ự�ļ򵥷�װ
class CTcpSession : public boost::enable_shared_from_this<CTcpSession>
{
public:
	CTcpSession(const boost::asio::io_service& ioService, INetCallback* pINetCallback);
	~CTcpSession(); 
	void HandleReadHeader(const boost::system::error_code& ec, size_t bytes_transferred, int idx_buffer);
	void HandleReadExtData(const boost::system::error_code& ec, size_t bytes_transferred, int idx_buffer);
	void HandleWrite(const boost::system::error_code& ec, size_t bytes_writed, int idx_buffer, long clock_send);
	void StartReadHeader();
	void StartReadExtData(int nBufferIndex);
	void SendMsg(const char* pData, size_t nDataSize); // ʵ�ʷ������ݵĺ���
	void DealError(const boost::system::error_code &ec); 

	socket_t&  GetSocket() { return m_socket; }
	std::string str() 
	{
		std::ostringstream strContext; 
		strContext << m_pRecvBuf << " " << m_nCurRecvBufferSize << " " << m_socket.remote_endpoint().address().to_string(); 
		return strContext.str(); 
	}
protected:
	bool _SendQueuedMsg(); 
	void _SendMsg(const unsigned char pData[], size_t nDataSize, int nBufferIndex); 
	void _StartReadHeader();
	void _StartReadExtData(int nBufferIndex); 
	void _SendQueueData(); // �̺߳���
private: 
	/*!
	    \brief ��ȡ���еķ��ͻ���������
		\return ����λ��
	*/
	int GetFreeSendBufferIndex(); 
	/*!
	    \brief ��ȡ���ͻ�������Ӧλ�õĿ��б�־
		\param[in] nIndex ����λ��
		\return ������λ���Ƿ�ʹ��
	*/
	bool GetSendBufferUsedFlag(int nIndex); 
	/*!
	    \brief ���÷��ͻ�������Ӧλ�õı�־
		\param[in] nIndex ����λ��
		\param[in] bFlag �Ƿ�ʹ��
		\return �����Ƿ�ɹ�
	*/
	bool SetSendBufferUsedFlag(int nIndex, bool bFlag); 

	/*!
	    \brief ��ȡ���еĽ��ջ���������
		\return ����λ��
	*/
	int GetFreeRecvBufferIndex(); 
	/*!
	    \brief ��ȡ���ջ�������Ӧλ�õĿ��б�־
		\param[in] nIndex ����λ��
		\return ������λ���Ƿ�ʹ��
	*/
	bool GetRecvBufferUsedFlag(int nIndex); 
	/*!
	    \brief ���ý��ջ�������Ӧλ�õı�־
		\param[in] nIndex ����λ��
		\param[in] bFlag �Ƿ�ʹ��
		\return �����Ƿ�ɹ�
	*/
	bool SetRecvBufferUsedFlag(int nIndex, bool bFlag); 
	bool IsInvalidData(const unsigned char *pDataBuf, int nLen); 
private:
	enum 
	{
		RECV_BUFFER_SIZE = 50,		//!< ���ջ��������� 
		SEND_BUFFER_SIZE = 500,		//!< ���ͻ��������� 
	};
	enum 
	{
		CLIENT_TYPE = 0, //!< �ͻ��˻Ự����
		SERVER_TYPE,	 //!< ����˻Ự����
	};

	socket_t m_socket;	//!< �Ự�󶨵�socket							
	INetCallback *m_pINetCallback; //!< �Ự�󶨵��û��ص��ӿ�
	boost::mutex m_mutexSend; //!< ���ͺ����ӵ��������Ⲣ��
	boost::shared_mutex m_mutexSendUsed; //!< �������ͻ�������־ʱ�õ���
	boost::shared_mutex m_mutexRecvUsed; //!< �������ջ�������־ʱ�õ���
	unsigned char *m_pRecvBuf[RECV_BUFFER_SIZE]; //!< ���ջ�����
	size_t m_nCurRecvBufferSize[RECV_BUFFER_SIZE]; //!< ���ջ�������С

	unsigned char *m_pSendBuf[SEND_BUFFER_SIZE]; //!< ���ͻ�����
	size_t m_nCurSendBufferSize[SEND_BUFFER_SIZE]; //!< ���ͻ�������С
	size_t m_nSendBufferValidSize[SEND_BUFFER_SIZE]; //!< �������Ϸ��������ݴ�С
	int m_nStationType; //!< վ�����ͣ��ͻ��˻�����
	int m_nServerStationID; //!< ��������ID��������Ϊ�ͻ���ʱ����������ΪԶ��վ��
	int m_nClientStationID; //!< �ͻ���ID��������Ϊ��������ʱ���ͻ���ΪԶ��վ��
	double m_dlbDurTime;	//!< ���ͼ�ʱ��
	clock_t m_clockStart, m_clockEnd; //!< ��ʱ�� 
	bool m_bConnected;  //!< ��־����״̬

	boost::lockfree::spsc_queue<int, boost::lockfree::capacity<SEND_BUFFER_SIZE> > m_qSendingFlag; //!< �������ݶ���
	bool m_bSendBufferUsed[SEND_BUFFER_SIZE]; //!< ���ͻ�������־����
	bool m_bRecvBufferUsed[RECV_BUFFER_SIZE]; //!< ���ջ�������־����

	boost::shared_ptr<boost::thread> m_pRunThread; 
};

//< TCP�Ự + �ص��ӿ�
class CAsioTcp
{
protected:
	typedef boost::shared_ptr<CTcpSession> TcpSessionPtr;
	typedef boost::asio::ip::tcp::acceptor acceptor;
public:
	CAsioTcp(INetCallback* pINetCallback);
	virtual ~CAsioTcp(void);
	void Start();
	void Stop(); 
protected:
	void SendMsg(socket_handle socket, const char* pData, size_t nDataSize);
protected:
	boost::asio::io_service m_ioservice;
	INetCallback* m_pINetCallback;
};

//< TCP�������
class CAsioTcpServer : public CAsioTcp 
{
public: 
	// ���ûص��ͼ����˿�
	CAsioTcpServer(INetCallback* pINetCallback, int nListenPort); 
	virtual ~CAsioTcpServer(); 
protected:
	void StartAccept();	// ���տͻ������ӵĺ��������ڲ�����
	void AcceptHandler(const boost::system::error_code& ec, TcpSessionPtr pTcpSession); // ���յ��ͻ������Ӻ�Ļص�����
	void CheckConnection(); // ���ͻ����Ƿ�ر�����
public:
	// �������ݵĽӿ�
	void SendMsg(int nStationID, const char* pData, size_t nDataSize); 
	void SendHeartBeat(int nStationID); // ����������δʵ��
protected:
	enum 
	{
		MIN_CHECK_TIME_SECOND = 5, 
	};
	boost::shared_ptr<acceptor> m_pAcceptor; 
	std::vector<TcpSessionPtr> m_vecSessionPtr; 
	//boost::asio::deadline_timer m_tickCheck;
	int m_nExpiresTime; 
};

//< TCP�ͻ�����
class CAsioTcpClient : public CAsioTcp
{
public:
	//< ���ûص��ͷ�����IP���˿�
	CAsioTcpClient(INetCallback* pINetCallback, const char *c_pszAddress, int nListenPort); 
	virtual ~CAsioTcpClient(); 
protected:
	void Connect();	// ���ӷ��������������ڲ�����
public:
	void SendMsg(const char* pData, size_t nDataSize); // ��������
protected:
	boost::asio::ip::tcp::endpoint m_connectPoint; 
	TcpSessionPtr m_pTcpSession; 
};