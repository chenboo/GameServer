// todo: will update! this version is so bad.

#include "log.h"
#include "..\utility\Utility.h"
#include <stdio.h>
#include <stdarg.h>

int WriteToLog(char* szFormat, ...)
{
	char szBuffer[1024] ;
	va_list pArgList;
	va_start (pArgList, szFormat) ;
	_vsnprintf(szBuffer, sizeof(szBuffer) / sizeof(char), szFormat, pArgList) ;
	va_end (pArgList) ;

	char szLogFilePath[260] = {0};
	GetFullPath("Error.txt", szLogFilePath);
	
	FILE* pLogFile;
	pLogFile = fopen(szLogFilePath, "a+");
	if (pLogFile == NULL) {
		return -1;
	}

	fprintf(pLogFile, "%s\n", szBuffer);
	fclose(pLogFile);
	return 0;
}

int WriteToScreen(char* szFormat, ...)
{
	char szBuffer[1024] ;
	
	va_list pArgList ;
	va_start (pArgList, szFormat) ;
	_vsnprintf( szBuffer, sizeof (szBuffer) / sizeof (char), szFormat, pArgList) ;
	va_end (pArgList) ;
	
	printf("%s\n", szBuffer);
	return 0;
}

void DumpBuffToFile(unsigned char* buf, int nSize)
{
    int i = 0;
	char fileName[256] = "";
	GetFullPath("logbuf.txt",fileName);

	FILE *fp = fopen(fileName, "a+");

	for (i=0; i<nSize; i++)
	{			
		fprintf(fp,"%02X ", buf[i]);

		if (i % 16 == 7)
		{
			fprintf(fp, "\t");
		}

		if (i % 16 == 15)
		{
			fprintf(fp, "\r\n");				
		}		

	}		

	fprintf(fp,"\r\n");


	for (i=0; i<nSize; i++)
	{			
		if (buf[i]>=32)// && buf[i]<=127)
		{
			fprintf(fp,"%c", buf[i]);						
		}
		else if (buf[i]==10)
		{
			fprintf(fp,"\\n");
		}
		else if (buf[i]==13)
		{
			fprintf(fp,"\\r");
		}
		else
		{
			fprintf(fp,".");
		}

	}	 
	fprintf(fp,"\r\n");

	fclose(fp);	
}

void DumpBuffToScreen(unsigned char* buf,int size)
{

    int i = 0;

	for ( i=0; i<size; i++)
	{			
		printf("%02X ", buf[i]);

		if (i % 16 == 7)
		{
			printf("\t");
		}

		if (i % 16 == 15)
		{
			printf("\r\n");				
		}		

	}		

	printf("\r\n");


	for ( i=0; i<size; i++)
	{			
		if (buf[i]>=32)// && buf[i]<=127)
		{
			printf("%c", buf[i]);						
		}
		else if (buf[i]==10)
		{
			printf("\\n");
		}
		else if (buf[i]==13)
		{
			printf("\\r");
		}
		else
		{
			printf(".");
		}

	}

	printf("\r\n");
}