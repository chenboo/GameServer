#include "stdio.h"
#include ".\testserver.h"

CTestServer::CTestServer(void)
{
}

CTestServer::~CTestServer(void)
{
}

void CTestServer::HandleRecvMessage(PACKET* lpPacket)
{
//	printf("收到:%s 序列号:%d \n",lpPacket->buf,lpPacket->longth);
//  ReleasePacket(lpPacket);  //不必要用户手工销毁

	SendPacket(lpPacket);  //反射发送回去
}
