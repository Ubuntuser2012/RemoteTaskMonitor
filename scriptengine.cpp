// scriptengine.cpp : Defines the entry point for the console application.
//http://www.codeproject.com/Articles/17038/COM-in-plain-C-part-8

#include "stdafx.h"
#define INITGUID

#include <windows.h>
#include <activscp.h>
#include <stdio.h>

HMODULE dll[1000];

typedef void (*FOO)();

// this example proc will be called and passed text from script
int WINAPI test( char* text, char* text2, int i ) 
{
   printf("my app proc called from script with param \"%s\" \"%s\" %i\n",text,text2, i);
   // and this return variable in turn passed to script
   return atol(text+strlen(text)-1)+1;
   // return L" test result "   you can return text to script too
}
int WINAPI test2(char* foo) 
{
   printf("my app proc called from script with param \"%s\"\n", foo);
   // and this return variable in turn passed to script
   return 1;
}

struct ScriptInterface : public IDispatch {
   DWORD cdcl;
   long  WINAPI QueryInterface( REFIID riid,void ** object) { *object=IsEqualIID(riid, IID_IDispatch)?this:0;         return *object?0:E_NOINTERFACE; }         
   DWORD WINAPI AddRef ()                                                                { return 0; }         
   DWORD WINAPI Release()                                                                { return 0; }
   long  WINAPI GetTypeInfoCount( UINT *)                                                { return 0; }
   long  WINAPI GetTypeInfo( UINT, LCID, ITypeInfo **)                                   { return 0; }
   // This is where we register procs (or vars)
   long  WINAPI GetIDsOfNames( REFIID riid, WCHAR** name, UINT cnt ,LCID lcid, DISPID *id) 
   { 
      for(DWORD j=0; j<cnt;j++) {
         char buf[1000]; 
         DWORD k; 
         WideCharToMultiByte(0,0,name[j],-1,buf,sizeof(buf),0,0);
         // two loops. one for sdcall second for cdecl ones with prefix _ added in script
         for(k=0; k<2;k++) 
         {
            // first check our app procs (test) 
            if(strcmp(buf+k,"test")==0) 
            { 
               id[j]=(DISPID)test; 
               break ; 
            } 
            else
            if(strcmp(buf+k,"test2")==0) 
            { 
               id[j]=(DISPID)test2; 
               break ; 
            } 
            else
            {
               // then win32 api in known dlls
               for(int i=0; (dll[i]||dll[i-1]);i++) 
               { 
                  if((id[j]=(DISPID)GetProcAddress(dll[i],buf+k))) 
                     break; 
               } 
            }
            if(id[j]) 
               break;
         }
         cdcl=k;

         if(!id[j]) 
            return E_FAIL;
      }
      return 0; 
   }
   // And this is where they are called from script
   long  WINAPI Invoke( DISPID id, REFIID riid, LCID lcid, WORD flags, DISPPARAMS *arg, VARIANT *ret, EXCEPINFO *excp, UINT *err) { 
      if(id) {
         // we support stdcall and cdecl conventions for now
         int i= cdcl?arg->cArgs:-1, result=0, stack=arg->cArgs*4; char* args[100]={0};
         while((cdcl?(--i>-1):(++i<(int)arg->cArgs))) {
            DWORD  param=arg->rgvarg[i].ulVal; 
            // we convert unicode string params to ansi since most apis are ansi
            if(arg->rgvarg[i].vt==VT_BSTR) { 
               WCHAR* w=arg->rgvarg[i].bstrVal; 
               WideCharToMultiByte(0,0,w,-1,args[i]=new char[wcslen(w)+1],wcslen(w)+1,0,0); param=(DWORD)args[i]; 
            }
            _asm push param; 
         }
         // for cdecl we push params in reverse order and cleanup the stack after call
         _asm call id
            _asm mov result, eax
            if (cdcl) _asm add esp, stack
               i=-1; while(++i<(int)arg->cArgs) if(args[i]) delete args[i];
         // return value to script (in this case we support just unsigned integer and unicode string types )
         if(ret) ret->ullVal=result; char*c=(char*)result;
         if(ret) ret->vt=VT_UI4; __try { if(!c[1]&&(*c<'0'||*c>'9')) ret->vt=VT_BSTR; ret->bstrVal=SysAllocString((WCHAR*)c); } __except(EXCEPTION_EXECUTE_HANDLER){}
         return 0; 
      }
      return E_FAIL;
   }
}; 

struct ScriptHost : public IActiveScriptSite  { 
   ScriptInterface Interface;
   long  WINAPI QueryInterface( REFIID riid,void ** object) { *object=(IsEqualIID(riid, IID_IActiveScriptSite))?this:0; return *object?0:E_NOINTERFACE;  }                                                                                 
   DWORD WINAPI AddRef ()                                                                { return 0; }         
   DWORD WINAPI Release()                                                                { return 0; }
   long  WINAPI GetLCID( DWORD *lcid )           {  *lcid = LOCALE_USER_DEFAULT;           return 0; }
   long  WINAPI GetDocVersionString( BSTR* ver ) {  *ver  = 0;                             return 0; }
   long  WINAPI OnScriptTerminate(const VARIANT *,const EXCEPINFO *)                     { return 0; }
   long  WINAPI OnStateChange( SCRIPTSTATE state)                                        { return 0; }
   long  WINAPI OnEnterScript()                                                          { return 0; }
   long  WINAPI OnLeaveScript()                                                          { return 0; }
   long  WINAPI GetItemInfo(const WCHAR *name,DWORD req, IUnknown ** obj, ITypeInfo ** type)  
   { 
      if(req&SCRIPTINFO_IUNKNOWN) *obj=&Interface; if(req&SCRIPTINFO_ITYPEINFO) *type=0;  return 0; 
   }
   long  WINAPI OnScriptError( IActiveScriptError *err )                                 
   { 
      EXCEPINFO e; err->GetExceptionInfo(&e); MessageBoxW(0,e.bstrDescription,e.bstrSource,0);
      return 0; 
   }
}; 

void main()
{
   HRESULT hr;  
   CoInitialize(0); 

   // In this sample we can call all nonunicode apis (for sample it's enough I suppose) from following dlls. Add more if you wish
   char* name[]={"ntdll","msvcrt","kernel32","user32","advapi32","shell32","wsock32","wininet","<",">",0}; 
   for(int i=0; name[i];i++) 
      dll[i]=LoadLibraryA(name[i]); 

   // Here we can chose between installed  script engines. Default engines shipped with windows are JavaScript and VbScript.
   GUID guid; 
   CLSIDFromProgID( L"JavaScript" , &guid );

   IActiveScript      *script; 
   hr = CoCreateInstance(guid, 0, CLSCTX_ALL, IID_IActiveScript,(void **)&script);  
   if(!script) 
      return;
   
   IActiveScriptParse  *parse;
   hr = script->QueryInterface(IID_IActiveScriptParse, (void **)&parse);

   ScriptHost host;
   script->SetScriptSite((IActiveScriptSite *)&host);
   script->AddNamedItem(L"app",  SCRIPTITEM_ISVISIBLE|SCRIPTITEM_NOCODE );

   // sample JavaScript demonstrating regular expression, call of our own proc (+ exchanging params) and external dll api call (win32 api subset in this case)
//   WCHAR* source = L" number = 'text 1'.match(/\\d/); result = app.test('hello'+' world '+number); "
//      L" app.MessageBoxA(0,'WIN'+32+' or any dll api called from JavaScript without need to install anything','hello world '+result,0)";

   WCHAR * source = L"app.test2('foo'); function app.foo(){}";
   hr = parse->InitNew(); 
   hr = parse->ParseScriptText( source ,0,0,0,0,0,0,0,0);
   script->SetScriptState( SCRIPTSTATE_CONNECTED);
   script->Close();
}; 

