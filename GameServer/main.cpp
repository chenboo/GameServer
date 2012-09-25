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


/*
case OP_WRITE:
case OP_ACCEPT:
//OP_ACCEPT事件只有在fllower/从属模型中才有用得上

  也就是在AcceptEx之前投递 OP_ACCEPT事件


OP系列的相关处理是你在投递了相关操作后才会得到处理的：
	AcceptEx对应的是OP_ACCEPT
	WSASend对应OP_WRITE
	WSARecv对应OP_READ
需要你使用这些API来投递相关操作，工作者线程才会给你响应的处理消息。
另外OP系列的操作时需要你手动设置的，比如你要投递一个AcceptEx,那么在
投递AcceptEx之前需要设置OpCode = OP_ACCEPT，然后调用AcceptEx，这样
在接受到一个客户端连接后，工作者线程会把这个OP_ACCEPT操作返回给你，
这时做连接处理即可；其他的操作都与此一样。
理解完成端口中的“完成”二字很关键


  二：提高完成端口效率的几种有效方法


1：使用AcceptEx代替accept。AcceptEx函数是微软的Winsosk 扩展函数，这个函数和accept的
区别就是：accept是阻塞的，一直要到有客户端连接上来后accept才返回，而AcceptEx是异步
的，直接就返回了，所以我们利用AcceptEx可以发出多个AcceptEx调用
等待客户端连接。另外，如果我们可以预见到客户端一连接上来后就会发送数据（比如WEBSERVER
的客户端浏览器），那么可以随着AcceptEx投递一个BUFFER进去，这样连接一建立成功，就可以
接收客户端发出的数据到BUFFER里，这样使用的话，一次AcceptEx调用相当于accpet和recv的一
次连续调用。同时，微软的几个扩展函数针对操作系统优化过，效率优于WINSOCK 的标准API函数。

2：在套接字上使用SO_RCVBUF和SO_SNDBUF选项来关闭系统缓冲区。这个方法见仁见智，详细
的介绍可以参考《WINDOWS核心编程》第9章。这里不做详细介绍，我封装的类中也没有使用这
个方法。


3：内存分配方法。因为每次为一个新建立的套接字都要动态分配一个“单IO数据”和“单句柄
数据”的数据结构，然后在套接字关闭的时候释放，这样如果有成千上万个客户频繁连接时候，
会使得程序很多开销花费在内存分配和释放上。这里我们可以使用lookaside list。开始在微软
的platform sdk里的SAMPLE里看到lookaside list，我一点不明白，MSDN里有没有。后来还是在
DDK的文档中找到了，，


*/