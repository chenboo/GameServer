#include "SessionMgr.h"

CSessionMangage::CSessionMangage()
{
	m_pFreeSessionList = NULL;
	m_nFreeSessionCount = 0;
	m_nMaxFreeSessions = MAX_FREESESSION;

	::InitializeCriticalSection(&m_FreeSessionListLock);
}

CSessionMangage::~CSessionMangage()
{
	::DeleteCriticalSection(&m_FreeSessionListLock);
}

CSession* CSessionMangage::AllocateSession()
{
	CSession *pSession = NULL;

	::EnterCriticalSection(&m_FreeSessionListLock);
	
	if(!m_pFreeSessionList) {
		pSession = (CSession *)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CSession)); 
		::InitializeCriticalSection(&pSession->Lock);
	}
	else {
		pSession = m_pFreeSessionList;
		m_pFreeSessionList = m_pFreeSessionList->pNext;
		pSession->pNext = NULL;

		m_nFreeSessionCount --;
	}

	::LeaveCriticalSection(&m_FreeSessionListLock);
	return pSession;
}

void CSessionMangage::ReleaseSession(CSession *pSession)
{
	do {
		if(pSession == NULL)
			break;

		::EnterCriticalSection(&m_FreeSessionListLock);

		if(m_nFreeSessionCount <= m_nMaxFreeSessions) {
			CRITICAL_SECTION csTmp = pSession->Lock;
			memset(pSession, 0, sizeof(CSession));

			pSession->Lock = csTmp;
			pSession->pNext = m_pFreeSessionList;
			m_pFreeSessionList = pSession;

			m_nFreeSessionCount++;
		}
		else {
			::DeleteCriticalSection(&pSession->Lock);
			::HeapFree(::GetProcessHeap(), 0, pSession);
		}

		::LeaveCriticalSection(&m_FreeSessionListLock);
	} while (FALSE);
}

void CSessionMangage::FreeSession()
{
	::EnterCriticalSection(&m_FreeSessionListLock);
	
	CSession *pFreeSession = m_pFreeSessionList;
	CSession *pNextSession = NULL;
	while(pFreeSession) {
		pNextSession = pFreeSession->pNext;
		
		::DeleteCriticalSection(&pFreeSession->Lock);
		if(!::HeapFree(::GetProcessHeap(), 0, pFreeSession)) {
#ifdef _DEBUG
			::OutputDebugString("  FreeContexts释放内存出错！");
#endif
			break;
		}
		else {
#ifdef _DEBUG
			OutputDebugString("  FreeContexts释放内存！\r\n");
#endif
		}
		pFreeSession = pNextSession;
	}

	m_pFreeSessionList = NULL;
	m_nFreeSessionCount = 0;

	::LeaveCriticalSection(&m_FreeSessionListLock);
}

void CSessionMangage::OnMessage(PACKET* lpPacket)
{

}