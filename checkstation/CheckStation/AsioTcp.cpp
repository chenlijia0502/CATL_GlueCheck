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
	    \brief ��ʼ�����ջ�������ر���
		\detail ���ջ��������ݿ��ʼ��Ϊһ����Ϣͷ�Ĵ�С
	*/
	for (int i = 0; i < RECV_BUFFER_SIZE; i++) 
	{
		m_pRecvBuf[i] = (unsigned char *)calloc(sizeof(NetDataHeader), sizeof(unsigned char)); 
		m_nCurRecvBufferSize[i] = sizeof(NetDataHeader); 
		m_bRecvBufferUsed[i] = false;
	}
	/*!
	    \brief ��ʼ�����ջ�������ر���
		\detail ���ջ��������ݿ��ʼ��Ϊ0
	*/
	for (int i = 0; i < SEND_BUFFER_SIZE; i++) 
	{
		m_pSendBuf[i] = NULL; 
		m_nCurSendBufferSize[i] = 0; 
		m_nSendBufferValidSize[i] = 0; 
		m_bSendBufferUsed[i] = false; 
	}
	/*!
	    \brief ��ʼ���������������ǻ��ڽ������ӵ�ʱ�����¸�ֵ
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
			\brief ���յ���Ϣͷ֮�����Ĵ���
			\detail 
			- 1 ȷ�����յ������ݳ���Ϊһ����Ϣͷ��С
			- 2 ���Ƶ���ʱ������
			- 3 У�����ݿ����ȷ�ԣ���Ϊ��ϵ���������ݵ����ݽ��գ������У���У��һ��
			- 4 ����Ƿ��ǵ�һ�η�����������ǵĻ���Զ��վ��Ž��и�ֵ�����ûص��ӿ�֪ͨ�û������ӳɹ���
				�����ûص��ӿ�֪ͨ�û����յ�������Ϣ
			- 5 �����Ϣͷ��û�д����ݿ飬��dataHeader.nExtDataSizeΪ0������ûص��ӿ�֪ͨ�û����յ����ݣ�
				������StartReadHeader()������һ������
			- 6 ������������ݿ飬��������ݿ��С������ջ�������С��������StartReadExtData()���մ�����
		*/
		assert(bytes_transferred == sizeof(NetDataHeader)); 
		NetDataHeader dataHeader; 
		memcpy(&dataHeader, m_pRecvBuf[idx_buffer], sizeof(NetDataHeader)); 

		if (0 == dataHeader.nExtDataSize) //!< ���û�д����ݿ�����
		{
			if (m_pINetCallback != NULL)
			{
				m_pINetCallback->OnRecvData(this, m_pRecvBuf[idx_buffer], bytes_transferred);
			}

			/*!
				�����ջ�������Ӧ�ı�־λ��Ϊ���У�Ϊ��һ�����ݽ�����׼��
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
				�����Ϣͷ��Ĵ����ݳ���С��0���򱨴��˳�
			*/
			if (dataHeader.nExtDataSize < 0) 
			{
				fprintf(stderr, "Error: Extern Data Size Is Invalid, Check The Sender Data\n"); 
				return;  
			}
			/*!
				�����������СС��Ҫ���յ����ݳ��ȣ����ٷ����ڴ�
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
			�����ˣ��������
		*/
		DealError(ec); 
	}
}

void CTcpSession::HandleReadExtData(const boost::system::error_code& ec, size_t bytes_transferred, int idx_buffer) 
{
	if (!ec)
	{
		/*!
			���յ�����֮��ֱ��ȥ�������е�����֪ͨ�û�
		*/
		if (m_pINetCallback != NULL)
		{
			m_pINetCallback->OnRecvData(this, m_pRecvBuf[idx_buffer], bytes_transferred + sizeof(NetDataHeader));
		}
		/*!
			�����ջ�������Ӧ�ı�־λ��Ϊ���У�Ϊ��һ�����ݽ�����׼��
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
			�ٴζ�ȡ��Ϣͷ����
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
		\brief ׼����������ͷ
		\detail 
		- 1 ���ҿ��еĽ��ջ�����
		- 2 ����ռ䲻�㣬���ٷ����ڴ�
		- 3 ����Boost���첽���ӿڶ�ȡ���ݣ������ö�ȡ���֮����õĺ����Ͳ���
	*/
	/*! 
		���ҿ��еĻ�����
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
		�����ڴ�
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
		���ýӿڣ�ָ�����������õĻ������ʹ�С��ϵͳ��ȡ���ô�С�����ݿ�֮�󣬾ͻص����úõ�handle����\n
		\code 
		async_read(
			m_socket,													//!< socket����
			boost::asio::buffer(m_pRecvBuf[nFreeRecvBuffer],			//!< ������ָ�룬���������ݻ��������
								sizeof(NetDataHeader)),					//!< ָ�����ݴ�С
			boost::bind(&CKxAsioTcpSession::HandleReadHeader,			//!< ������ָ�룬������Сһ�µ�����֮�����
						shared_from_this(),								//!< thisָ�룬Ҳ���ǵ�ǰ����
																		//!< ����������HandleReadHeader�Ĳ���
						boost::asio::placeholders::error,				//!< ����ռλ����asio���ú���ʱ����
						boost::asio::placeholders::bytes_transferred,	//!< ����ռλ����asio���ú���ʱ����
						nFreeRecvBuffer									//!< ���������������ô�����֪��Ҫȥ��ȡ����
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
		\brief �����첽���ӿڶ�ȡͷ��Ϣ��Ĵ�����
		\note ���ʱ��ͷ��Ϣ�Ѿ�����ָ���Ļ��������ˣ����һ�������С����������
		��Щ��������HandleReadHeader()�������õ�
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
		���ҿ��еķ��ͻ�����������־Ϊ����
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
		�������ݣ����ݿ���������ڷ��Ͷ����У����ȡ�����������������ⲿwhileѭ��ִֻ��һ��
		\note ���﷢�����ݺ󲻵���pop������pop�������첽д�Ĵ�������ִ�У�Ҳ����HandleWrite����
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
				�������ֵ����������pop�����У�����ʹ��whileѭ������Ϊ����һ���̰߳�ȫ���У������߳̿���Ҳ���������н��в���
				��pop�ɹ�ʱ���᷵��true�˳�ѭ��
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
		�����첽д������ָ��������ΪHandleWrite��async_write������async_read��࣬
		�ο�_StartReadHeader�е�ע��
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
			�������ݳɹ�������ôη����������õ�ʱ��
		*/
		double dlbDurTime = (double)(clock() - clock_send);
		if (sizeof(NetDataHeader) < bytes_writed) 
		{
			//fprintf(stderr, "      �����������ݰ� ��ʱ��%.2fms\n", dlbDurTime / CLOCKS_PER_SEC * 1000);
		}
	} 

	/*!
		�ӷ��Ͷ�����ȡ���ôη��͵�����������m_qSendingFlag��һ���̰߳�ȫ����
	*/
	int nSendedIndex = -1; 
	
	//boost::unique_lock<boost::mutex>(m_mutexSend); 
	while (!m_qSendingFlag.pop(nSendedIndex))
		;
	/*!
		�жϷ��Ͷ����Ƿ�Ϊ�գ������Ϊ����˵����������δ���ͣ���¼��־������
	*/
	bool bEmpty = m_qSendingFlag.empty(); 
	if (nSendedIndex != idx_buffer || !GetSendBufferUsedFlag(idx_buffer)) 
	{
		/*!
			���ȡ�������������ڷ������ݿ�����������Ӧ�ı�־λû����Ϊ��ʹ�ã�\n
			�������Boost���첽����������������������⣬��ӡ������Ϣ����������
		*/
		std::cerr << "Sending Buffer Flag Exception! Index: " << idx_buffer << std::endl; 
	}
	else  
	{
		SetSendBufferUsedFlag(idx_buffer, false); 
	}

	/*!
		������Ͷ��в�Ϊ�գ�����ú�������ȡ��һ�����ݷ���
		\note ֮����Ҫ�Ƚ��жϽ�����浽bEmpty�У�����Ϊ����Ի�������־λ���в���ʱ����
		����Ϊ����ͻ�ȴ����ڵȴ��Ĺ����������ط����ܻ����յķ��Ͷ���������ݲ���ʼ�������ݣ�
		������������ж϶����Ƿ�Ϊ�գ������Ϊ�����ﻹ��δ���͵�����
	*/
// 	if (!bEmpty) 
// 	{
// 		_SendQueuedMsg(); 
// 	}
	m_mutexSend.unlock(); 
}

int CTcpSession::GetFreeSendBufferIndex()
{
	boost::shared_lock<boost::shared_mutex> rdLock(m_mutexSendUsed); //!< ����
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
	boost::shared_lock<boost::shared_mutex> rdLock(m_mutexSendUsed);//!< ����
	return m_bSendBufferUsed[nIndex]; 
}

bool CTcpSession::SetSendBufferUsedFlag(int nIndex, bool bFlag)
{
	boost::unique_lock<boost::shared_mutex> wtLock(m_mutexSendUsed); //!< д��
	/*!
		�п��ܱ�����߳����ñ�־λ�ˣ��ͷ���false
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
	boost::shared_lock<boost::shared_mutex> rdLock(m_mutexRecvUsed); //!< ����
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
	boost::shared_lock<boost::shared_mutex> rdLock(m_mutexRecvUsed); //!< ����
	return m_bRecvBufferUsed[nIndex]; 
}

bool CTcpSession::SetRecvBufferUsedFlag(int nIndex, bool bFlag)
{
	boost::unique_lock<boost::shared_mutex> wtLock(m_mutexRecvUsed); //!< д��
	/*!
		�п��ܱ�����߳����ñ�־λ�ˣ��ͷ���false
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
		std::cerr << "�ر���������..." << std::endl; 
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

	// �������� [11/25/2015 YOYO]
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
