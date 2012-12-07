//SessionManage.h
#ifndef _SessionMangage_H_
#define _SessionMangage_H_
#include "..\iocp\IOCP.h"
#include "..\MsgDef\MsgDef.h"

#define MAX_FREESESSION 3000

typedef struct _CSession
{
	CIOCPContext* lpSession;
	CRITICAL_SECTION Lock;
	_CSession *pNext;
} CSession;

class CSessionMangage 
{
public:
	CSessionMangage();
	virtual ~CSessionMangage();

public:
	virtual void OnMessage(PACKET *lpPacket);

private:
	CSession *AllocateSession();
	void ReleaseSession(CSession *pSession);
	void FreeSession();

private:
	int m_nFreeSessionCount;
	int m_nMaxFreeSessions;

	CSession* m_pFreeSessionList;
	CRITICAL_SECTION m_FreeSessionListLock;
};
#endif