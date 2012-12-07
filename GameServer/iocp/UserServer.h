#ifndef _ServerSocket_H_
#define _ServerSocket_H_

#include "IOCP.h"
#include "..\log\log.h"
#include "..\MsgDef\MsgDef.h"

#include <list>
using namespace std;

template <class T>
class CQueue : public list<T>
{
public:
	CQueue(void) 
	{
		::InitializeCriticalSection(&m_ListLock);
	}

	virtual ~CQueue(void) 
	{
		::DeleteCriticalSection(&m_ListLock);
	}

	unsigned long GetSize()
	{
		
		::EnterCriticalSection(&m_ListLock);

		unsigned long m_size = 0;
		m_size = size();
		
		::LeaveCriticalSection(&m_ListLock);
		
		return m_size;
	}

	void Push(T lpData)
	{
		::EnterCriticalSection(&m_ListLock);

		push_back(lpData);
		
		::LeaveCriticalSection(&m_ListLock);
	}

	T Pop()
	{
		::EnterCriticalSection(&m_ListLock);

		T lpData = NULL;
		if(size())
		{
			lpData = front();
			pop_front(); 
		}

		::LeaveCriticalSection(&m_ListLock);

		return lpData;
	}

	bool Empty()
	{
	
		::EnterCriticalSection(&m_ListLock);

		bool bRet = false;
		bRet = empty();

		::LeaveCriticalSection(&m_ListLock);

		return bRet;
	}

private::
	CRITICAL_SECTION m_ListLock;
};

typedef unsigned (WINAPI *PTHREADFUN)(LPVOID lpParameter); 
class CUserServer : public CIOCPServer
{
public:
	CUserServer();
	virtual ~CUserServer();

public:
	void IniServer();
	bool StartupAllMsgThread();

	bool SendPacket(PACKET* lpPacket);
	PACKET* AllocatePacket();
	void ReleasePacket(PACKET *pPacket);

private:
	void AddPacketToRecvlist(PACKET* lpPacket)
	{
		m_listRecvMsg.Push(lpPacket);
	}

	PACKET* PopPacketFromRecvList()
	{
		return m_listRecvMsg.Pop();
	}

	void AddPacketToSendlist(PACKET* lpPacket)
	{
		m_listSendMsg.Push(lpPacket);
	}

	PACKET* PopPacketFromSendList()
	{
		return m_listSendMsg.Pop();
	}

	void SendPacketToIOCP(PACKET* lpPacket);

	static DWORD WINAPI RecvThread(LPVOID lpParameter);
	static DWORD WINAPI SendThread(LPVOID lpParameter);
	void CloseAllMsgThread();

	virtual void HandleRecvMessage(PACKET* lpPacket) = 0;

	void OnConnectionEstablished(CIOCPContext *pContext, CIOCPBuffer *pBuffer);
	void OnConnectionClosing(CIOCPContext *pContext, CIOCPBuffer *pBuffer);
	void OnConnectionError(CIOCPContext *pContext, CIOCPBuffer *pBuffer, int nError);
	void OnReadCompleted(CIOCPContext *pContext, CIOCPBuffer *pBuffer);
	void OnWriteCompleted(CIOCPContext *pContext, CIOCPBuffer *pBuffer);
	bool SplitPacket(CIOCPContext *pContext, CIOCPBuffer *pBuffer);
	void OnPacketError(){ WriteToLog("²ð°ü´íÎó!\r\n");}

protected:
	void FreePacket();

private:
	typedef CQueue<PACKET*> CPacketArray;
	CPacketArray m_listRecvMsg;
	CPacketArray m_listSendMsg;

	HANDLE m_hRecvWaitEvent, m_hSendWaitEvent;
	HANDLE m_hRecvThread, m_hSendThread;
	bool m_bRecvRun, m_bSendRun;

	int m_nFreePacketCount;	
	int m_nMaxPacketBuffers;
	PACKET *m_pFreePacketList;
	CRITICAL_SECTION m_FreePacketListLock;
};
#endif
