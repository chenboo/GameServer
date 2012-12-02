//todo:
#ifndef _LOG_H_
#define _LOG_H_

int WriteToLog(char* szFormat, ...);
int WriteToScreen(char* szFormat, ...);

void DumpBuffToFile(unsigned char* lpBuf, int nLen);
void DumpBuffToScreen(unsigned char* lpBuf, int nLen);

#endif