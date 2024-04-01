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

//< 数据包头格式
typedef struct tagDataHeader 
{
	int nStationID;		// 站点ID
	int nMsgType;		// 数据包消息类型
	int nExtDataSize;	// 超大数据长度
} DataHeader, NetDataHeader;

enum MSG_TYPE_DEFINE 
{
	USER_MSG_DEFINE_START = 1, 
};

#pragma pack(pop) 

//< 网络库回调类
class INetCallback
{
public:
	virtual void OnNewConnection(socket_handle newSocket) = 0;	// 建立新连接后的回调函数
	virtual void OnRecvData(socket_handle socket, const unsigned char* pData, size_t nDataSize) = 0; // 接收到数据后的回调函数
};

// TCP会话的简单封装
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
	void SendMsg(const char* pData, size_t nDataSize); // 实际发送数据的函数
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
	void _SendQueueData(); // 线程函数
private: 
	/*!
	    \brief 获取空闲的发送缓冲区索引
		\return 索引位置
	*/
	int GetFreeSendBufferIndex(); 
	/*!
	    \brief 获取发送缓冲区对应位置的空闲标志
		\param[in] nIndex 索引位置
		\return 缓冲区位置是否被使用
	*/
	bool GetSendBufferUsedFlag(int nIndex); 
	/*!
	    \brief 设置发送缓冲区对应位置的标志
		\param[in] nIndex 索引位置
		\param[in] bFlag 是否使用
		\return 设置是否成功
	*/
	bool SetSendBufferUsedFlag(int nIndex, bool bFlag); 

	/*!
	    \brief 获取空闲的接收缓冲区索引
		\return 索引位置
	*/
	int GetFreeRecvBufferIndex(); 
	/*!
	    \brief 获取接收缓冲区对应位置的空闲标志
		\param[in] nIndex 索引位置
		\return 缓冲区位置是否被使用
	*/
	bool GetRecvBufferUsedFlag(int nIndex); 
	/*!
	    \brief 设置接收缓冲区对应位置的标志
		\param[in] nIndex 索引位置
		\param[in] bFlag 是否使用
		\return 设置是否成功
	*/
	bool SetRecvBufferUsedFlag(int nIndex, bool bFlag); 
	bool IsInvalidData(const unsigned char *pDataBuf, int nLen); 
private:
	enum 
	{
		RECV_BUFFER_SIZE = 50,		//!< 接收缓冲区个数 
		SEND_BUFFER_SIZE = 500,		//!< 发送缓冲区个数 
	};
	enum 
	{
		CLIENT_TYPE = 0, //!< 客户端会话类型
		SERVER_TYPE,	 //!< 服务端会话类型
	};

	socket_t m_socket;	//!< 会话绑定的socket							
	INetCallback *m_pINetCallback; //!< 会话绑定的用户回调接口
	boost::mutex m_mutexSend; //!< 发送函数加的锁，避免并发
	boost::shared_mutex m_mutexSendUsed; //!< 操作发送缓冲区标志时用的锁
	boost::shared_mutex m_mutexRecvUsed; //!< 操作接收缓冲区标志时用的锁
	unsigned char *m_pRecvBuf[RECV_BUFFER_SIZE]; //!< 接收缓冲区
	size_t m_nCurRecvBufferSize[RECV_BUFFER_SIZE]; //!< 接收缓冲区大小

	unsigned char *m_pSendBuf[SEND_BUFFER_SIZE]; //!< 发送缓冲区
	size_t m_nCurSendBufferSize[SEND_BUFFER_SIZE]; //!< 发送缓冲区大小
	size_t m_nSendBufferValidSize[SEND_BUFFER_SIZE]; //!< 缓冲区合法发送数据大小
	int m_nStationType; //!< 站点类型，客户端或服务端
	int m_nServerStationID; //!< 服务器端ID，当本地为客户端时，服务器端为远程站点
	int m_nClientStationID; //!< 客户端ID，当本地为服务器端时，客户端为远程站点
	double m_dlbDurTime;	//!< 发送计时用
	clock_t m_clockStart, m_clockEnd; //!< 计时用 
	bool m_bConnected;  //!< 标志连接状态

	boost::lockfree::spsc_queue<int, boost::lockfree::capacity<SEND_BUFFER_SIZE> > m_qSendingFlag; //!< 发送数据队列
	bool m_bSendBufferUsed[SEND_BUFFER_SIZE]; //!< 发送缓冲区标志数组
	bool m_bRecvBufferUsed[RECV_BUFFER_SIZE]; //!< 接收缓冲区标志数组

	boost::shared_ptr<boost::thread> m_pRunThread; 
};

//< TCP会话 + 回调接口
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

//< TCP服务端类
class CAsioTcpServer : public CAsioTcp 
{
public: 
	// 设置回调和监听端口
	CAsioTcpServer(INetCallback* pINetCallback, int nListenPort); 
	virtual ~CAsioTcpServer(); 
protected:
	void StartAccept();	// 接收客户端连接的函数，在内部调用
	void AcceptHandler(const boost::system::error_code& ec, TcpSessionPtr pTcpSession); // 接收到客户端连接后的回调函数
	void CheckConnection(); // 检测客户端是否关闭连接
public:
	// 发送数据的接口
	void SendMsg(int nStationID, const char* pData, size_t nDataSize); 
	void SendHeartBeat(int nStationID); // 发送心跳，未实现
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

//< TCP客户端类
class CAsioTcpClient : public CAsioTcp
{
public:
	//< 设置回调和服务器IP、端口
	CAsioTcpClient(INetCallback* pINetCallback, const char *c_pszAddress, int nListenPort); 
	virtual ~CAsioTcpClient(); 
protected:
	void Connect();	// 连接服务器函数。在内部调用
public:
	void SendMsg(const char* pData, size_t nDataSize); // 发送数据
protected:
	boost::asio::ip::tcp::endpoint m_connectPoint; 
	TcpSessionPtr m_pTcpSession; 
};