#include "stdafx.h"
#include <boost/thread.hpp>
#include "AsioTcp.h"
#include <vector>

using namespace std; 
using namespace boost::asio;

CAsioTcp::CAsioTcp(INetCallback* pINetCallback)
: m_pINetCallback(pINetCallback)
{
	
}


CAsioTcp::~CAsioTcp(void)
{
}

void CAsioTcp::SendMsg(socket_handle socket, const char* pData, size_t nDataSize)
{
	TcpSessionPtr* ppTcpSession = reinterpret_cast<TcpSessionPtr *>(socket);
	(*ppTcpSession)->SendMsg(pData, nDataSize);
}

void CAsioTcp::Start()
{
	m_ioservice.run();
}

void CAsioTcp::Stop()
{
	m_ioservice.stop(); 
}

CTcpSession::CTcpSession(const boost::asio::io_service& ioService, INetCallback* pINetCallback)
: m_socket(const_cast<boost::asio::io_service&>(ioService))
, m_pINetCallback(pINetCallback)
{
	/*!
	    \brief 初始化接收缓冲区相关变量
		\detail 接收缓冲区数据块初始化为一个消息头的大小
	*/
	for (int i = 0; i < RECV_BUFFER_SIZE; i++) 
	{
		m_pRecvBuf[i] = (unsigned char *)calloc(sizeof(NetDataHeader), sizeof(unsigned char)); 
		m_nCurRecvBufferSize[i] = sizeof(NetDataHeader); 
		m_bRecvBufferUsed[i] = false;
	}
	/*!
	    \brief 初始化接收缓冲区相关变量
		\detail 接收缓冲区数据块初始化为0
	*/
	for (int i = 0; i < SEND_BUFFER_SIZE; i++) 
	{
		m_pSendBuf[i] = NULL; 
		m_nCurSendBufferSize[i] = 0; 
		m_nSendBufferValidSize[i] = 0; 
		m_bSendBufferUsed[i] = false; 
	}
	/*!
	    \brief 初始化其他变量，它们会在建立连接的时候被重新赋值
	*/
	m_nStationType = 0; 
	m_nServerStationID = m_nClientStationID = -1; 
	m_bConnected = false; 
	m_pRunThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CTcpSession::_SendQueueData, this)));
}


CTcpSession::~CTcpSession()
{ 
	m_pRunThread->interrupt(); 
	for (int i = 0; i < RECV_BUFFER_SIZE; i++) 
	{
		free(m_pRecvBuf[i]);
	} 
	for (int i = 0; i < SEND_BUFFER_SIZE; i++) 
	{
		free(m_pSendBuf[i]); 
	}
}

void CTcpSession::HandleReadHeader(const boost::system::error_code& ec, size_t bytes_transferred, int idx_buffer) 
{
	if (!ec)
	{
		/*!
			\brief 接收到消息头之后做的处理
			\detail 
			- 1 确定接收到的数据长度为一个消息头大小
			- 2 复制到临时变量中
			- 3 校验数据块的正确性，因为关系到后面数据的数据接收，最好用校验和校验一下
			- 4 检查是否是第一次发心跳，如果是的话对远程站点号进行赋值，调用回调接口通知用户已连接成功，
				并调用回调接口通知用户接收到心跳消息
			- 5 如果消息头后没有大数据块，即dataHeader.nExtDataSize为0，则调用回调接口通知用户接收到数据，
				并调用StartReadHeader()接收下一个数据
			- 6 如果后面有数据块，则根据数据块大小分配接收缓冲区大小，并调用StartReadExtData()接收大数据
		*/
		assert(bytes_transferred == sizeof(NetDataHeader)); 
		NetDataHeader dataHeader; 
		memcpy(&dataHeader, m_pRecvBuf[idx_buffer], sizeof(NetDataHeader)); 

		if (0 == dataHeader.nExtDataSize) //!< 如果没有大数据块的情况
		{
			if (m_pINetCallback != NULL)
			{
				m_pINetCallback->OnRecvData(this, m_pRecvBuf[idx_buffer], bytes_transferred);
			}

			/*!
				将接收缓冲区对应的标志位置为空闲，为下一次数据接收做准备
			*/
			if (!GetRecvBufferUsedFlag(idx_buffer)) 
			{
				fprintf(stderr, "Warning: Recv Buffer Index Flag Exception: %d\n", idx_buffer); 
			}
			else 
			{
				SetRecvBufferUsedFlag(idx_buffer, false); 
			}
			StartReadHeader();
		}
		else 
		{
			/*!
				如果消息头里的大数据长度小于0，则报错退出
			*/
			if (dataHeader.nExtDataSize < 0) 
			{
				fprintf(stderr, "Error: Extern Data Size Is Invalid, Check The Sender Data\n"); 
				return;  
			}
			/*!
				如果缓冲区大小小于要接收的数据长度，则再分配内存
			*/
			if (m_nCurRecvBufferSize[idx_buffer] < dataHeader.nExtDataSize + sizeof(NetDataHeader)) 
			{
				m_nCurRecvBufferSize[idx_buffer] = dataHeader.nExtDataSize + sizeof(NetDataHeader); 
				m_pRecvBuf[idx_buffer] = (unsigned char *)realloc(m_pRecvBuf[idx_buffer], m_nCurRecvBufferSize[idx_buffer]); 
				assert(NULL != m_pRecvBuf[idx_buffer]); 
				memcpy(m_pRecvBuf[idx_buffer], &dataHeader, sizeof(NetDataHeader)); 
			}
			StartReadExtData(idx_buffer); 
		}
		
	}
	else 
	{
		/*!
			出错了，处理错误
		*/
		DealError(ec); 
	}
}

void CTcpSession::HandleReadExtData(const boost::system::error_code& ec, size_t bytes_transferred, int idx_buffer) 
{
	if (!ec)
	{
		/*!
			接收到数据之后直接去缓冲区中的数据通知用户
		*/
		if (m_pINetCallback != NULL)
		{
			m_pINetCallback->OnRecvData(this, m_pRecvBuf[idx_buffer], bytes_transferred + sizeof(NetDataHeader));
		}
		/*!
			将接收缓冲区对应的标志位置为空闲，为下一次数据接收做准备
		*/
		if (!GetRecvBufferUsedFlag(idx_buffer)) 
		{
			fprintf(stderr, "Warning: Recv Buffer Index Flag Exception: %d\n", idx_buffer); 
		}
		else 
		{
			SetRecvBufferUsedFlag(idx_buffer, false); 
		}
		/*!
			再次读取消息头数据
		*/
		StartReadHeader();
	}
	else 
	{
		DealError(ec);
	}
}

void CTcpSession::StartReadHeader() 
{
	_StartReadHeader(); 
}

void CTcpSession::StartReadExtData(int nBufferIndex) 
{
	_StartReadExtData(nBufferIndex); 
}

void CTcpSession::_StartReadHeader()
{
	/*!
		\brief 准备接收数据头
		\detail 
		- 1 查找空闲的接收缓冲区
		- 2 如果空间不足，则再分配内存
		- 3 调用Boost的异步读接口读取数据，并设置读取完成之后调用的函数和参数
	*/
	/*! 
		查找空闲的缓冲区
	*/
	int nFreeRecvBuffer = -1; 
	while ((nFreeRecvBuffer = GetFreeRecvBufferIndex()) >= 0) 
	{
		if (SetRecvBufferUsedFlag(nFreeRecvBuffer, true)) 
		{
			break; 
		}
	}
	if (-1 == nFreeRecvBuffer) 
	{
		fprintf(stderr, "Receive Buffer Is Too Small\n"); 
		return; 
	}
	/*! 
		分配内存
	*/
	const int c_nHeaderSize = sizeof(NetDataHeader); 
	if (NULL == m_pRecvBuf[nFreeRecvBuffer] || m_nCurRecvBufferSize[nFreeRecvBuffer] < c_nHeaderSize) 
	{
		m_pRecvBuf[nFreeRecvBuffer] = (unsigned char *)realloc(m_pRecvBuf[nFreeRecvBuffer], c_nHeaderSize); 
		if (NULL == m_pRecvBuf[nFreeRecvBuffer]) 
		{
			fprintf(stderr, "recv buffer oom\n"); 
			m_nCurRecvBufferSize[nFreeRecvBuffer] = 0; 
			return; 
		}
		else 
		{
			m_nCurRecvBufferSize[nFreeRecvBuffer] = c_nHeaderSize;
		}
	}

	/*! 
		调用接口，指定接收数据用的缓冲区和大小，系统读取到该大小的数据块之后，就回调设置好的handle函数\n
		\code 
		async_read(
			m_socket,													//!< socket对象
			boost::asio::buffer(m_pRecvBuf[nFreeRecvBuffer],			//!< 缓冲区指针，读到的数据会往这里放
								sizeof(NetDataHeader)),					//!< 指定数据大小
			boost::bind(&CKxAsioTcpSession::HandleReadHeader,			//!< 处理函数指针，读满大小一致的数据之后调用
						shared_from_this(),								//!< this指针，也就是当前对象
																		//!< 下面三个是HandleReadHeader的参数
						boost::asio::placeholders::error,				//!< 错误占位符，asio调用函数时再填
						boost::asio::placeholders::bytes_transferred,	//!< 长度占位符，asio调用函数时再填
						nFreeRecvBuffer									//!< 缓冲区的索引，让处理函数知道要去哪取数据
		));
		\endcode
	*/
	async_read(m_socket, boost::asio::buffer(m_pRecvBuf[nFreeRecvBuffer], sizeof(NetDataHeader)),
		boost::bind(&CTcpSession::HandleReadHeader, shared_from_this(), boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred, nFreeRecvBuffer));
}

void CTcpSession::_StartReadExtData(int nBufferIndex)
{
	/*!
		\brief 调用异步读接口读取头消息后的大数据
		\note 这个时候头消息已经放在指定的缓冲区中了，并且缓冲区大小已做调整，
		这些操作是在HandleReadHeader()里面做好的
	*/
	NetDataHeader dataHeader; 
	memcpy(&dataHeader, m_pRecvBuf[nBufferIndex], sizeof(NetDataHeader)); 
	async_read(m_socket, boost::asio::buffer(m_pRecvBuf[nBufferIndex] + sizeof(NetDataHeader), dataHeader.nExtDataSize),
		boost::bind(&CTcpSession::HandleReadExtData, shared_from_this(), boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred, nBufferIndex));
}

void CTcpSession::SendMsg(const char* pData, size_t nDataSize)
{
	if (!GetSocket().is_open()) 
	{
		return; 
	}
   	/*!
		查找空闲的发送缓冲区，并标志为已用
	*/
	int nReadySendBufferIndex = -1; 
	while ((nReadySendBufferIndex = GetFreeSendBufferIndex()) >= 0) 
	{
		if (SetSendBufferUsedFlag(nReadySendBufferIndex, true)) 
		{
			break;
		}
	}

	if (-1 == nReadySendBufferIndex)
	{
		fprintf(stderr, "Session Buffer is Too Small!\n"); 
		return; 
	}

   	if (m_nCurSendBufferSize[nReadySendBufferIndex] < nDataSize) 
   	{
   		m_pSendBuf[nReadySendBufferIndex] = (unsigned char *)realloc(m_pSendBuf[nReadySendBufferIndex], nDataSize);
   		if (NULL == m_pSendBuf[nReadySendBufferIndex]) 
   		{
   			fprintf(stderr, "Realloc Memory Fail!\n"); 
   			return; 
   		}
   		m_nCurSendBufferSize[nReadySendBufferIndex] = nDataSize; 
   	}
   	m_nSendBufferValidSize[nReadySendBufferIndex] = nDataSize; 
   	memcpy(m_pSendBuf[nReadySendBufferIndex], pData, m_nSendBufferValidSize[nReadySendBufferIndex]); 
   
	//boost::unique_lock<boost::mutex>(m_mutexSend); 
	//bool bStartSend = m_qSendingFlag.empty(); 
	while (!m_qSendingFlag.push(nReadySendBufferIndex))
		; 
// 	if (bStartSend) 
// 	{
// 		_SendQueuedMsg(); 
// 	}
}

bool CTcpSession::_SendQueuedMsg()
{
	/*!
		发送数据，数据块的索引放在发送队列中，如果取出的索引正常，则外部while循环只执行一次
		\note 这里发送数据后不调用pop操作，pop操作在异步写的处理函数里执行，也就是HandleWrite函数
	*/
	while (!m_qSendingFlag.empty()) 
	{
		while (!m_qSendingFlag.read_available()); 
		int nReadySendBufferIndex = m_qSendingFlag.front(); 
		if (0 <= nReadySendBufferIndex && nReadySendBufferIndex < SEND_BUFFER_SIZE) 
		{
			if (m_mutexSend.try_lock()) 
			{
				_SendMsg(m_pSendBuf[nReadySendBufferIndex], m_nSendBufferValidSize[nReadySendBufferIndex], nReadySendBufferIndex); 
				return true; 
			}
			return false; 
		}
		else 
		{
			/*!
				如果索引值不正常，则pop出队列，这里使用while循环是因为这是一个线程安全队列，其他线程可能也会对这个队列进行操作
				当pop成功时，会返回true退出循环
			*/
			while (!m_qSendingFlag.pop()); 
		}
	}
	return false; 
}

void CTcpSession::_SendMsg(const unsigned char pData[], size_t nDataSize, int nBufferIndex)
{
	if (!IsInvalidData(pData, int(nDataSize)))
	{
		std::vector<int> ivec; 
		int nIndex; 
		std::cerr << "Que: ";  
		while (!m_qSendingFlag.empty()) 
		{
			if (m_qSendingFlag.pop(nIndex)) 
			{
				ivec.push_back(nIndex); 
			}
		}
		DataHeader *pDataHeader = (DataHeader *)pData; 
		assert(pDataHeader->nStationID >= 0 && pDataHeader->nExtDataSize == nDataSize - sizeof(DataHeader)); 
	}
#ifdef ASYNC_WRITE
	/*!
		调用异步写操作，指定处理函数为HandleWrite，async_write参数与async_read差不多，
		参考_StartReadHeader中的注释
	*/
	boost::asio::async_write(m_socket,
		boost::asio::buffer(pData, nDataSize),
		boost::bind(&CTcpSession::HandleWrite, shared_from_this(),
		boost::asio::placeholders::error, nDataSize, nBufferIndex, clock()));
#else 
 	clock_t start = clock();
	boost::system::error_code ec; 
	try 
	{
		nDataSize = write(m_socket, boost::asio::buffer(pData, nDataSize));
	}
	catch (boost::system::system_error e)
	{
		ec = e.code(); 
	}
	HandleWrite(ec, nDataSize, nBufferIndex, start); 
#endif 
}


void CTcpSession::HandleWrite(const boost::system::error_code& ec, size_t bytes_writed, int idx_buffer, long clock_send)
{
	if (ec) 
	{
		DealError(ec);
	}
	else 
	{
		/*!
			发送数据成功，计算该次发送数据所用的时间
		*/
		double dlbDurTime = (double)(clock() - clock_send);
		if (sizeof(NetDataHeader) < bytes_writed) 
		{
			//fprintf(stderr, "      发送网络数据包 用时：%.2fms\n", dlbDurTime / CLOCKS_PER_SEC * 1000);
		}
	} 

	/*!
		从发送队列里取出该次发送的数据索引，m_qSendingFlag是一个线程安全队列
	*/
	int nSendedIndex = -1; 
	
	//boost::unique_lock<boost::mutex>(m_mutexSend); 
	while (!m_qSendingFlag.pop(nSendedIndex))
		;
	/*!
		判断发送队列是否为空，如果不为空则说明还有数据未发送，记录标志待发送
	*/
	bool bEmpty = m_qSendingFlag.empty(); 
	if (nSendedIndex != idx_buffer || !GetSendBufferUsedFlag(idx_buffer)) 
	{
		/*!
			如果取出的索引不等于发送数据块的索引，或对应的标志位没有置为已使用，\n
			则可能是Boost库异步操作出现问题或库代码有问题，打印出错信息，继续运行
		*/
		std::cerr << "Sending Buffer Flag Exception! Index: " << idx_buffer << std::endl; 
	}
	else  
	{
		SetSendBufferUsedFlag(idx_buffer, false); 
	}

	/*!
		如果发送队列不为空，则调用函数继续取下一条数据发送
		\note 之所以要先将判断结果保存到bEmpty中，是因为上面对缓冲区标志位进行操作时可能
		会因为锁冲突等待，在等待的过程中其他地方可能会往空的发送队列里放数据并开始发送数据，
		如果在这里再判断队列是否为空，则会认为队列里还有未发送的数据
	*/
// 	if (!bEmpty) 
// 	{
// 		_SendQueuedMsg(); 
// 	}
	m_mutexSend.unlock(); 
}

int CTcpSession::GetFreeSendBufferIndex()
{
	boost::shared_lock<boost::shared_mutex> rdLock(m_mutexSendUsed); //!< 读锁
	for (int i = 0; i < SEND_BUFFER_SIZE; i++) 
	{
		if (!m_bSendBufferUsed[i]) 
		{
			return i; 
		}
	}
	return -1; 
}

bool CTcpSession::GetSendBufferUsedFlag(int nIndex)
{
	boost::shared_lock<boost::shared_mutex> rdLock(m_mutexSendUsed);//!< 读锁
	return m_bSendBufferUsed[nIndex]; 
}

bool CTcpSession::SetSendBufferUsedFlag(int nIndex, bool bFlag)
{
	boost::unique_lock<boost::shared_mutex> wtLock(m_mutexSendUsed); //!< 写锁
	/*!
		有可能被别的线程先置标志位了，就返回false
	*/
	if (bFlag == m_bSendBufferUsed[nIndex]) 
	{
		return false; 
	}
	m_bSendBufferUsed[nIndex] = bFlag; 
	return true; 
}

int CTcpSession::GetFreeRecvBufferIndex()
{
	boost::shared_lock<boost::shared_mutex> rdLock(m_mutexRecvUsed); //!< 读锁
	for (int i = 0; i < RECV_BUFFER_SIZE; i++) 
	{
		if (!m_bRecvBufferUsed[i]) 
		{
			return i; 
		}
	}
	return -1; 
}

bool CTcpSession::GetRecvBufferUsedFlag(int nIndex)
{
	boost::shared_lock<boost::shared_mutex> rdLock(m_mutexRecvUsed); //!< 读锁
	return m_bRecvBufferUsed[nIndex]; 
}

bool CTcpSession::SetRecvBufferUsedFlag(int nIndex, bool bFlag)
{
	boost::unique_lock<boost::shared_mutex> wtLock(m_mutexRecvUsed); //!< 写锁
	/*!
		有可能被别的线程先置标志位了，就返回false
	*/
	if (bFlag == m_bRecvBufferUsed[nIndex]) 
	{
		return false; 
	}
	m_bRecvBufferUsed[nIndex] = bFlag; 
	return true; 
}

void CTcpSession::DealError(const boost::system::error_code &ec)
{
	//std::cerr << ec.message() << endl;  
	switch(ec.value()) 
	{
	case boost::asio::error::connection_aborted:
	case boost::asio::error::connection_reset:
	case boost::asio::error::connection_refused:  
		GetSocket().close();
		std::cerr << "关闭网络连接..." << std::endl; 
		return;
	} 
	//throw std::runtime_error(ec.message()); 
}

void CTcpSession::_SendQueueData()
{
	while(true) 
	{
		if (false == _SendQueuedMsg())
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		}
	}
}

bool CTcpSession::IsInvalidData(const unsigned char *pDataBuf, int nLen)
{
	DataHeader *pDataHeader = (DataHeader *)pDataBuf; 
	return pDataHeader->nStationID >= 0 && pDataHeader->nExtDataSize == nLen - sizeof(DataHeader); 
}

CAsioTcpServer::CAsioTcpServer(INetCallback* pINetCallback, int nListenPort)
: CAsioTcp(pINetCallback), /*m_tickCheck(m_ioservice), */m_nExpiresTime(MIN_CHECK_TIME_SECOND)
{
	m_pAcceptor = boost::shared_ptr<acceptor>(new acceptor(m_ioservice, ip::tcp::endpoint(ip::tcp::v4(), nListenPort)));
	StartAccept();
	//CheckConnection(); 
}

CAsioTcpServer::~CAsioTcpServer()
{

}

void CAsioTcpServer::StartAccept()
{
	TcpSessionPtr pTcpSession = boost::make_shared<CTcpSession>(m_ioservice, m_pINetCallback);
	m_pAcceptor->async_accept(pTcpSession->GetSocket(), boost::bind(&CAsioTcpServer::AcceptHandler, this, boost::asio::placeholders::error, pTcpSession));
}

void CAsioTcpServer::AcceptHandler(const boost::system::error_code& ec, TcpSessionPtr pTcpSession)
{
	if (ec)
	{
		return;
	}

	m_vecSessionPtr.push_back(pTcpSession); 
	if (m_pINetCallback != NULL)
	{
		m_pINetCallback->OnNewConnection(&pTcpSession);
	}

	StartAccept();
	pTcpSession->StartReadHeader();
}

void CAsioTcpServer::SendMsg(int nStationID, const char* pData, size_t nDataSize)
{
	//CheckConnection(); 
	//if (-1 == nStationID) 
	{
		for (int i = 0; i < m_vecSessionPtr.size(); ++i) 
		{
			m_vecSessionPtr[i]->SendMsg(pData, nDataSize); 
		}
	}
}

void CAsioTcpServer::CheckConnection()
{
	vector<TcpSessionPtr>::const_iterator iter = m_vecSessionPtr.begin(); 
	while (iter != m_vecSessionPtr.end()) 
	{
		if (false == (*iter)->GetSocket().is_open()) 
		{
			iter = m_vecSessionPtr.erase(iter); 
		}
		else 
		{
			++iter; 
		}
	}

	// 加入心跳 [11/25/2015 YOYO]
// 	if (m_vecSessionPtr.size() > 0) 
// 	{
// 		SendHeartBeat(-1); 
// 	}
	
// 	m_tickCheck.expires_from_now(boost::posix_time::seconds(m_nExpiresTime));
// 	m_tickCheck.async_wait(boost::bind(&CAsioTcpServer::CheckConnection, this));
}

void CAsioTcpServer::SendHeartBeat(int nStationID)
{
	DataHeader dataHeader; 
	memset(&dataHeader, 0, sizeof(dataHeader)); 
	dataHeader.nMsgType = 0; 
	SendMsg(nStationID, (const char *)&dataHeader, sizeof(dataHeader)); 
}

CAsioTcpClient::CAsioTcpClient(INetCallback* pINetCallback, const char *c_pszAddress, int nListenPort)
: CAsioTcp(pINetCallback)
{
	m_pTcpSession = boost::make_shared<CTcpSession>(m_ioservice, m_pINetCallback); 
	m_connectPoint = ip::tcp::endpoint(ip::address::from_string(c_pszAddress), nListenPort); 
	Connect(); 
}

CAsioTcpClient::~CAsioTcpClient()
{
	
}

void CAsioTcpClient::Connect()
{
	m_pTcpSession->GetSocket().connect(m_connectPoint); 
	m_pTcpSession->StartReadHeader(); 
}

void CAsioTcpClient::SendMsg(const char* pData, size_t nDataSize)
{
	CAsioTcp::SendMsg(&m_pTcpSession, pData, nDataSize); 
}
