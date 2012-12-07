#include "userserver.h"
#include <process.h>

CUserServer::CUserServer()
{
	m_pFreePacketList = NULL;
	m_nMaxPacketBuffers = 20000;
	m_nFreePacketCount = 0;
	::InitializeCriticalSection(&m_FreePacketListLock);
}

CUserServer::~CUserServer()
{
	m_pFreePacketList = NULL;
	m_nMaxPacketBuffers = 0;
	m_nFreePacketCount = 0;

	FreePacket();
	::DeleteCriticalSection(&m_FreePacketListLock);
}

PACKET* CUserServer::AllocatePacket()
{
	::EnterCriticalSection(&m_FreePacketListLock);
	
	PACKET *pPacket = NULL;
	if(m_pFreePacketList == NULL) {
		pPacket = (PACKET *)::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PACKET));
	}
	else {
		pPacket = m_pFreePacketList;
		m_pFreePacketList = m_pFreePacketList->pNext;
		pPacket->pNext = NULL;
		m_nFreePacketCount--;
	}

	::LeaveCriticalSection(&m_FreePacketListLock);

	return pPacket;
}

void CUserServer::ReleasePacket(PACKET *pPacket)
{
	::EnterCriticalSection(&m_FreePacketListLock);

	if(m_nFreePacketCount <= m_nMaxPacketBuffers) {
		memset(pPacket, 0, sizeof(PACKET));
		pPacket->pNext = m_pFreePacketList;
		m_pFreePacketList = pPacket;
		m_nFreePacketCount++ ;
	}
	else {
		::HeapFree(::GetProcessHeap(), 0, pPacket);
	}

	::LeaveCriticalSection(&m_FreePacketListLock);
}

void CUserServer::FreePacket()
{
	::EnterCriticalSection(&m_FreePacketListLock);

	PACKET *pFreePacket = m_pFreePacketList;
	PACKET *pNextPacket = NULL;
	while(pFreePacket != NULL) {
		pNextPacket = pFreePacket->pNext;
		if(!::HeapFree(::GetProcessHeap(), 0, pFreePacket)) {
#ifdef _DEBUG
			::OutputDebugString("  FreeBuffers释放内存出错！");
#endif
			break;
		}
		else {
#ifdef _DEBUG
			OutputDebugString("  FreeBuffers释放内存！");
#endif
		}

		pFreePacket = pNextPacket;
	}

	m_pFreePacketList = NULL;
	m_nFreePacketCount = 0;

	::LeaveCriticalSection(&m_FreePacketListLock);
}

void CUserServer::IniServer()
{
	int i = 0;
    CIOCPBuffer *pBuffer[3000];
	for (i = 0; i < 3000; i++) {
		pBuffer[i] = AllocateBuffer(BUFFER_SIZE);
	}

	for (i = 0; i< 3000; i++) {
		ReleaseBuffer(pBuffer[i]);
	}
    
	SOCKET s;
    CIOCPContext *pContext[100];
    for (i = 0; i< 100; i++)
	{
		pContext[i] = AllocateContext(s);
	}

	for (i = 0; i< 100; i++)
	{
		ReleaseContext(pContext[i]);
	}

    PACKET *pPacket[3000];
	for (i = 0; i< 3000; i++)
	{
		pPacket[i] = AllocatePacket();
	}

	for (i = 0; i< 3000; i++)
	{
		ReleasePacket(pPacket[i]);
	}
}

bool CUserServer::StartupAllMsgThread()
{
	m_bRecvRun =true;
	m_hRecvThread = NULL;
	m_hRecvWaitEvent = NULL;
	m_hRecvWaitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (m_hRecvWaitEvent==NULL)
		return false;

	m_hRecvThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, NULL, (PTHREADFUN) RecvThread, this, 0, NULL)); 
	if (m_hRecvThread == NULL) 
		return false;

	m_bSendRun = true;
	m_hSendThread = NULL;
	m_hSendWaitEvent = NULL;
	m_hSendWaitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (!m_hSendWaitEvent)
		return false;

	m_hSendThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, NULL, (PTHREADFUN)SendThread, this, 0, NULL)); 
	if (m_hSendThread == NULL) 
		return false;
	
	return true;
}

void CUserServer::CloseAllMsgThread()
{
	m_bRecvRun = false;
	SetEvent(m_hRecvWaitEvent);
	if(WaitForSingleObject(m_hRecvThread,10000)!= WAIT_OBJECT_0)
		TerminateThread(m_hRecvThread, 0);
	
	CloseHandle(m_hRecvThread);
	CloseHandle(m_hRecvWaitEvent);
	
	m_bSendRun = false;
	SetEvent(m_hSendWaitEvent);
	if(WaitForSingleObject(m_hSendThread,10000)!= WAIT_OBJECT_0)
		TerminateThread(m_hSendThread, 0);
	
	CloseHandle(m_hSendThread);
	CloseHandle(m_hSendWaitEvent);
}

void CUserServer::SendPacketToIOCP(PACKET* lpPacket)
{
	SendText(lpPacket->lpOCPContext, lpPacket->buf, lpPacket->nLen);
}

//用户层将游戏数据包发送到发送队列
bool CUserServer::SendPacket(PACKET* lpPacket)
{
  //复制包,然后才加入发送队列,避免包还没发送出去,用户就销毁	
	PACKET *lpP = AllocatePacket();
    if(lpP) {
		memcpy(lpP, lpPacket, sizeof(PACKET));
        AddPacketToSendlist(lpP);
        return true ;
    }

    return false;
}

DWORD WINAPI CUserServer ::RecvThread(LPVOID lpParameter)
{
	CUserServer *lpUserServer = (CUserServer*)lpParameter;

	while (lpUserServer->m_bRecvRun)
	{
		PACKET* lpPacket = NULL;
		if(!lpUserServer->m_listRecvMsg.Empty())
		{
			lpPacket = lpUserServer->PopPacketFromRecvList();
			if(lpPacket !=NULL)
			{
				lpUserServer->HandleRecvMessage(lpPacket);
				lpUserServer->ReleasePacket(lpPacket);
			}
		}
		//接收列表中有包时受信
		else
		{
  		    WaitForSingleObject(lpUserServer->m_hRecvWaitEvent,1);
		}
	}

	return 0;
}

DWORD WINAPI CUserServer::SendThread(LPVOID lpParameter)
{
	CUserServer *lpUserServer = (CUserServer*)lpParameter;
	while (lpUserServer->m_bSendRun)
	{
		PACKET* lpPacket = NULL;
		if(!lpUserServer->m_listSendMsg.Empty())
		{
			lpPacket = lpUserServer->PopPacketFromSendList();
			if(lpPacket !=NULL)
			{
				lpUserServer->SendPacketToIOCP(lpPacket);
				lpUserServer->ReleasePacket(lpPacket);
			}
		}
		//发送列表中有包时受信
		else
		{
		    WaitForSingleObject(lpUserServer->m_hSendWaitEvent,1);
		}
	}

	return 0;
}

void CUserServer::OnConnectionEstablished(CIOCPContext *pContext, CIOCPBuffer *pBuffer)
{
	printf(" 接收到一个新的连接（%d）： %s \n", 
		GetCurrentConnection(), ::inet_ntoa(pContext->addrRemote.sin_addr));

	SendText(pContext, pBuffer->buff, pBuffer->nLen);
}

void CUserServer::OnConnectionClosing(CIOCPContext *pContext, CIOCPBuffer *pBuffer)
{
	printf(" 一个连接关闭！ \n" );
}

void CUserServer::OnConnectionError(CIOCPContext *pContext, CIOCPBuffer *pBuffer, int nError)
{
	printf(" 一个连接发生错误： %d \n ", nError);
}

void CUserServer::OnReadCompleted(CIOCPContext *pContext, CIOCPBuffer *pBuffer)
{
//	printf("%d | %d | %d | %d \n", pBuffer->nSequenceNumber,m_nFreeBufferCount,m_nFreeContextCount, GetIOCPBufferCount()/*,pBuffer->buff*/);
//	printf("%d | %d | %d | AllocateCount %d | ReleaseCount %d | %d \n", pBuffer->nSequenceNumber,m_nFreeBufferCount,m_nFreeContextCount, AllocateCount, ReleaseCount ,GetIOCPBufferCount()/*,pBuffer->buff*/);

	//拼包处理
	SplitPacket(pContext, pBuffer);
//	SendText(pContext, pBuffer->buff, pBuffer->nLen);
}

void CUserServer::OnWriteCompleted(CIOCPContext *pContext, CIOCPBuffer *pBuffer)
{
//	printf(" 数据发送成功！\n ");
}

//拼包处理
bool CUserServer::SplitPacket(CIOCPContext *pContext, CIOCPBuffer *pBuffer)
{
	PACKET *lpPacket = AllocatePacket();
    if(lpPacket != NULL)
    {
		lpPacket->nLen = pBuffer->nLen;
		lpPacket->nSequence = pBuffer->nSequenceNumber;
		lpPacket->nID = pContext->s;
		lpPacket->lpOCPContext = pContext;
		memcpy(lpPacket->buf,pBuffer->buff, pBuffer->nLen);
		AddPacketToRecvlist(lpPacket);
        return true;
    }
    return false;

/*
	CIOCPContext* lpSession = pContext;

	// 如果缓冲区太小,就将拼包缓冲区发送到接收队列
    if (2048 <= (lpSession->lpBufEnd - lpSession->lpBufBegin + pBuffer->nLen))
    {
		PACKET *lpPacket = AllocatePacket();
	    if(lpPacket == NULL)
		   return false;

		// 将拼包缓冲区发送到接收队列
		lpPacket->nLen = lpSession->lpBufEnd - lpSession->lpBufBegin;
		lpPacket->nSequence = pBuffer->nSequenceNumber;
		lpPacket->nID = lpSession->s;
		lpPacket->lpOCPContext = pContext;
		memcpy(lpPacket->buf,lpSession->arrayDataBuf, lpSession->lpBufEnd - lpSession->lpBufBegin);
		AddPacketToRecvlist(lpPacket);
		//清空拼包缓冲区
  		lpSession->lpBufBegin = lpSession->arrayDataBuf;
		lpSession->lpBufEnd   = lpSession->arrayDataBuf;
	}
	//拼接到拼包缓冲区末尾
	memcpy(lpSession->lpBufEnd,pBuffer->buff,pBuffer->nLen);
	lpSession->lpBufEnd = lpSession->lpBufEnd + pBuffer->nLen;
	return true;
*/

/*
	CIOCPContext* lpSession = pContext;
	//原始数据

	DWORD dwDataLen = (DWORD)(lpSession->lpBufEnd - lpSession->lpBufBegin);
	//收到数据
	DWORD dwByteCount = pBuffer->nLen;


	//如果缓冲区不够了，就做数据前移
	if(USE_DATA_LONGTH - (lpSession->lpBufEnd - lpSession->arrayDataBuf) < (int)dwByteCount)
	{
		//拼包缓冲区数据前移
		//如果缓冲区仍然不够，就将之前的数据全部丢掉
		if(USE_DATA_LONGTH - dwDataLen < dwByteCount)
		{
			dwDataLen = 0;
			lpSession->lpBufBegin = lpSession->lpBufEnd = lpSession->arrayDataBuf;
			OnPacketError();
			return false;
		}
		else
		{
			memcpy(lpSession->arrayDataBuf, lpSession->lpBufBegin, dwDataLen);//移动缓存
			lpSession->lpBufBegin = lpSession->arrayDataBuf;
			lpSession->lpBufEnd = lpSession->lpBufBegin+dwDataLen;//缓存尾指针
		}

	}

	//copy数据到缓冲区尾部
	memcpy(lpSession->lpBufEnd, pBuffer->buff, dwByteCount);
	lpSession->lpBufEnd += dwByteCount;//更新缓存尾指针
	dwDataLen = (DWORD)(lpSession->lpBufEnd - lpSession->lpBufBegin);//更新缓存长度
	while(dwDataLen)
	{
		BYTE Mask = lpSession->lpBufBegin[0];

		if(Mask != 128)
		{
			lpSession->lpBufBegin = lpSession->lpBufEnd = lpSession->arrayDataBuf;
			OnPacketError();
			return false;

		}
		if (dwDataLen <=3)//没有收到数据包的长度 // byte 128 WORD longth; 
			break;
		short int longth = *(short int*)(lpSession->lpBufBegin+1);
		//数据长度超过合法长度
		if(longth > NET_DATA_LONGTH || longth < 3)
		{
			lpSession->lpBufBegin = lpSession->lpBufEnd = lpSession->arrayDataBuf;
			OnPacketError();
			return false;
		}
		if(longth + 3 > (long)dwDataLen)//没有形成完整的数据包
			break;
		//if(*(long*)(lpSession->m_lpBufBegin+3) != NET_MESSAGE_CHECK_NET)
		//{
		//	LPGAMEMSG lpGameMsg = m_Msg_Pool.MemPoolAlloc();
		//	lpGameMsg->length = longth;
		//	memset(lpGameMsg->arrayDataBuf,0,USE_DATA_LONGTH);
		//	*(long*)lpGameMsg->arrayDataBuf = longth;
		//	memcpy(lpGameMsg->arrayDataBuf+sizeof(long),lpSession->m_lpBufBegin+3,longth);
		//	lpGameMsg->lpSession = lpSession;
		//	m_Msg_Queue.Push(lpGameMsg);
		//	lpGameMsg = NULL;
		//}
		////更新缓存头指针
		unsigned char arraybuffer[USE_DATA_LONGTH];
		ZeroMemory(arraybuffer,sizeof(arraybuffer));
		*(long*)arraybuffer = longth;
		memcpy(arraybuffer+sizeof(long),lpSession->lpBufBegin+3,longth);

		DumpBuffToScreen(arraybuffer,longth+4);

		lpSession->lpBufBegin += longth + 3;
		dwDataLen = (WORD)(lpSession->lpBufEnd - lpSession->lpBufBegin);

	}
	return true;
*/

}