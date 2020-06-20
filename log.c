#include "comm.h"

#include <stdio.h>
#include <stdarg.h>

static wchar_t glog[256];

void logf(const wchar_t* pFormat, ...)
{
   va_list variableArgumentList;
   va_start(variableArgumentList, pFormat);
#ifdef _WIN32_WCE
   //vswprintf(glog, pFormat, variableArgumentList);
   StringCbVPrintfW(glog, sizeof(glog), pFormat, variableArgumentList);
#else
   vswprintf(glog, sizeof(glog), pFormat, variableArgumentList);
#endif
   OutputDebugString(glog);
   wprintf(glog);
   va_end(variableArgumentList);

}

