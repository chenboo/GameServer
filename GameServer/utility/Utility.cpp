//todo:
#include "Utility.h"
#include <stdio.h>
#include <windows.h>
#include <assert.h>

int GetFullPath(const char* szFileName,char* szFullPath)
{
	char moduleName[256] = {0};
    GetModuleFileName(NULL, moduleName, 256);
    int longth = (int)lstrlen(moduleName);
   
	assert(longth > 3);

	int i;
    for(i = longth-1;i >= 0;i--)
    {
	char ch = moduleName[i];
	if(ch == '\\')
	    break;
    }
    if(i>3)
    {
	moduleName[i+1] = 0;
	lstrcat(szFullPath,moduleName);
	lstrcat(szFullPath,szFileName);
    }
    else
	return -1;
    return 0;

}