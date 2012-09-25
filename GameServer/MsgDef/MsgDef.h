#ifndef _MsgDefine_H_
#define _MsgDefine_H_
#include "..\iocp\IOCP.h"
#define NET_SESSIONMANAGE_MALLOC 0x100
#define NET_SESSIONMANAGE_FREE 0x101

typedef struct _PACKET
{
	unsigned long nLen;  //包大小
	unsigned long nID;   //包编号
	unsigned long nSequence; //当前pBuffer
//	char buf[2048];
	char buf[4096];
	CIOCPContext *lpOCPContext;
	_PACKET *pNext;
}PACKET;
typedef PACKET* LPPACKET;
#endif