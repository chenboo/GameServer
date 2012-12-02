#include "stdio.h"
//#include "iocp\iocp.h"
#include "iocp\TestServer.h"

void main()
{
	CTestServer *pServer = new CTestServer;
	pServer->IniServer();

	// 开启服务,   启动发送与接收队列及线程
	if(pServer->Start() && pServer->StartupAllMsgThread())
	{
		printf(" 服务器开启成功... \n");
	}
	else
	{
		printf(" 服务器开启失败！\n");
		return;
	}

/*    
    getchar();
	CIOCPContext *m_pCCon;
	m_pCCon = pServer->GetConnectionList();
    while(m_pCCon != NULL)
	{
		while(m_pCCon != NULL)
		{   
			pServer->SendText(m_pCCon,"123456",6);

			m_pCCon = m_pCCon->pNext;
		}
    	m_pCCon = pServer->GetConnectionList();
        Sleep(500);
	}
*/
/*    
    getchar();

	PACKET *lpPacket = pServer->AllocatePacket();
    char *cNum = "54630";
	memcpy(lpPacket->buf,cNum,strlen(cNum));
	lpPacket->lpOCPContext = pServer->GetConnectionList();

	while(lpPacket->lpOCPContext != NULL)
	{
		while(lpPacket->lpOCPContext != NULL)
		{   
			pServer->SendPacket(lpPacket);

			lpPacket->lpOCPContext = lpPacket->lpOCPContext->pNext;
		}
    	lpPacket->lpOCPContext = pServer->GetConnectionList();
        Sleep(500);
	}
	pServer->ReleasePacket(lpPacket);
*/
	// 创建事件对象，让ServerShutdown程序能够关闭自己
	HANDLE hEvent = ::CreateEvent(NULL, FALSE, FALSE, "ShutdownEvent");
	::WaitForSingleObject(hEvent, INFINITE);
	::CloseHandle(hEvent);

	// 关闭服务
	pServer->Shutdown();
	delete pServer;

	printf(" 服务器关闭 \n ");
}