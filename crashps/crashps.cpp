// crashps.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>


void crash()
{
   int *p=0;
   *p=0;
}

void foo2(int x)
{
   x = 5;
   crash();
}
void foo1(int x, char* s)
{
   x = 4;
   foo2(x);
}

int _tmain(int argc, _TCHAR* argv[])
{
   Sleep(1500);
   foo1(1, "beta");
	return 0;
}

