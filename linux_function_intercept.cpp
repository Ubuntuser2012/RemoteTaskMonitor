//https://gist.github.com/mooware/1174572
//To build, open console and type
//
//gcc -g -Wall -shared -fPIC -ldl -lrt main.c -o overlord.so
//LD_PRELOAD=/home/os75115/1/MetaFutex/overlord.so /home/os75115/gsd/qtcreator-build/bin/JDDisplay

#include <stdio.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
// necessary for RTLD_NEXT in dlfcn.h
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>
#include <stdarg.h>
#include <malloc.h>
#include <time.h>
#include <execinfo.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <iostream>
#include <map>
#include <vector>

#define THREAD_NAME_LEN 16 //must be 16
#define LOG_PATH "/home/os75115"
/*
typedef int (*PTHREAD_MUTEX_LOCK)(pthread_mutex_t *mutex);
static PTHREAD_MUTEX_LOCK _pthread_mutex_lock = 0;

typedef int (*PTHREAD_MUTEX_UNLOCK)(pthread_mutex_t *mutex);
static PTHREAD_MUTEX_UNLOCK _pthread_mutex_unlock = 0;
*/
typedef int (*SYSCALL)(int number, ...);
static SYSCALL _syscall = 0;
#define MySyscall(param1, ...) (*_syscall)(param1, ##__VA_ARGS__)

const int BACKTRACE_BUFFER_SIZE = 1024;
const int NUM_LOG_LINES = 2000;
const int NUM_CHARS_IN_LINE = 512;
const unsigned int MAX_CALLSTACKS_PER_MUTEX = 10;

class CallStack
{
    int ID;
    int Count;
    void* Buffer[BACKTRACE_BUFFER_SIZE];
    char ThreadName[THREAD_NAME_LEN];
    char TimeStamp [128];
public:
    CallStack(int id, char* threadName, char* timeStamp)
        : ID(id)
        , Count(0)
    {
        strcpy(ThreadName, threadName);
        strcpy(TimeStamp, timeStamp);
    }
    void Get()
    {
        Count = backtrace(Buffer, BACKTRACE_BUFFER_SIZE);
    }
    void Print(FILE* fp)
    {
        if(Count)
        {
            char** functions = backtrace_symbols(Buffer, Count);

            if(functions != NULL)
            {
               fprintf(fp, "%s - Thread: %s ==========================\n", TimeStamp, ThreadName[0] ? ThreadName : "");
               for(int j = 0; j < Count; j++)
               {
                  fprintf(fp, "%s\n", functions[j]);
               }
               free(functions);
            }
        }
    }
};

class Interceptor
{
    char LogBuffer[NUM_LOG_LINES][NUM_CHARS_IN_LINE];
    int LogLineNumber;
    bool LoggerReady;
    pthread_mutex_t Mutex;
    std::map<int, std::vector<CallStack*> > CallStacks;

    void DumpCallStacks()
    {
        std::map<int, std::vector<CallStack*> >::iterator it;
        for (it = CallStacks.begin(); it != CallStacks.end(); ++it)
        {
            char fileName[128];
            sprintf(fileName, LOG_PATH"/callstack-0x%x.txt", it->first);
            FILE* fp=fopen(fileName, "wt");
            if(fp)
            {
                std::vector<CallStack*>::iterator it2;
                for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    (*it2)->Print(fp);
                    delete *it2;
                }
                it->second.clear();
                fclose(fp);
            }
        }
        CallStacks.clear();
    }

    void GetFunctionAddress(const char* methodName, void** methodAddress)
    {
        void *tmpPtr = dlsym(RTLD_NEXT, methodName);
        memcpy(methodAddress, &tmpPtr, sizeof(&tmpPtr));
    }

    pthread_t GetThreadName(char *threadName)
    {
        pthread_t tid = pthread_self();
        memset(threadName, 0, THREAD_NAME_LEN);
        prctl(PR_GET_NAME, threadName, 0, 0, 0);

        return tid;
    }

    int GetTimeStamp(char *tbuffer)
    {
        time_t t = time(0);
        struct tm * timeinfo = localtime ( &t );
        struct timespec now = GetClockTime();
        int ms = now.tv_sec*1000 + now.tv_nsec/1000000;
        strftime (tbuffer, 80,"%y-%m-%d %H:%M:%S", timeinfo);
        char _ms[32];
        sprintf(_ms, ":%i", ms);
        strcat(tbuffer, _ms);
        return strlen(tbuffer);
    }

public:
    Interceptor()
        : LogLineNumber(0)
        , LoggerReady(false)
    {
        GetFunctionAddress("syscall", (void**)&_syscall);

        pthread_mutex_init(&Mutex, NULL);
        LoggerReady = true;
    }
    ~Interceptor()
    {
        pthread_mutex_unlock(&Mutex);
        pthread_mutex_destroy(&Mutex);
    }

    void FlushLog()
    {
        if(LoggerReady)
        {
            FILE *fp = NULL;

            pthread_mutex_lock(&Mutex);
            fp = fopen(LOG_PATH"/log.txt", "wt");
            if(fp)
            {
                fprintf(fp, "==============================\n");
                for(int i = LogLineNumber % NUM_LOG_LINES; i<NUM_LOG_LINES && LogBuffer[i][0]; i++)
                {
                    fprintf(fp, "%s", LogBuffer[i]);
                }
                for(int i = 0; i<LogLineNumber && LogBuffer[i][0]; i++)
                {
                    fprintf(fp, "%s", LogBuffer[i]);
                }
                fclose(fp);
                LogLineNumber = 0;
            }
            DumpCallStacks();
            pthread_mutex_unlock(&Mutex);
        }
    }

    void Log(const char *fmt, ...)
    {
        if(LoggerReady)
        {
            char threadName[THREAD_NAME_LEN];
            GetThreadName(threadName);

            char timeStamp [128];
            GetTimeStamp(timeStamp);

            va_list args;
            va_start(args, fmt);

            pthread_mutex_lock(&Mutex);
            memset(LogBuffer[LogLineNumber], 0, NUM_CHARS_IN_LINE);
            sprintf(LogBuffer[LogLineNumber], "%s - Thread:%s - ", timeStamp, threadName[0] ? threadName : "");
            vsprintf (&LogBuffer[LogLineNumber][strlen(LogBuffer[LogLineNumber])], fmt, args);
            LogBuffer[LogLineNumber][strlen(LogBuffer[LogLineNumber])]='\n';
            LogLineNumber = (LogLineNumber+1) % NUM_LOG_LINES;
            pthread_mutex_unlock(&Mutex);

            va_end(args);
        }
    }

    void AddCallStack(int obj)
    {
        char threadName[THREAD_NAME_LEN];
        GetThreadName(threadName);

        char timeStamp [128];
        GetTimeStamp(timeStamp);

        CallStack *stack = new CallStack(obj, threadName, timeStamp);
        stack->Get();

        pthread_mutex_lock(&Mutex);
        if(CallStacks[obj].size() >= MAX_CALLSTACKS_PER_MUTEX)
        {
            std::vector<CallStack*>::iterator it = CallStacks[obj].begin();
            CallStacks[obj].erase(it);
        }
        CallStacks[obj].push_back(stack);
        pthread_mutex_unlock(&Mutex);
    }

    timespec GetClockTime()
    {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        return now;
    }

	timespec FunctionIntercept::GetLastCallTime(int obj)
	{
		struct timespec ret;
		std::map<int, std::vector<CallStack*> >::iterator it;

		pthread_mutex_lock(&Mutex);
		it = CallStacks.find(obj);
		if(it != CallStacks.end())
		{
			std::vector<CallStack*>::iterator it2 = it->second.begin();
			ret = (*it2)->GetTimeStamp();
		}
		pthread_mutex_unlock(&Mutex);
		return ret;
	}

    int GetTimeDiffInMS(struct timespec t1, struct timespec t2)
    {
        int ms1 = t1.tv_sec*1000 + t1.tv_nsec/1000000;
        int ms2 = t2.tv_sec*1000 + t2.tv_nsec/1000000;

        return ms2 - ms1;
    }
}
TheInterceptor;

extern "C" {
long int syscall(long int param1, ...)
{
    va_list args;
    va_start(args, param1);
    int mutexObj = va_arg(args, int);//param2
    int mutexOperation = va_arg(args, int);//param3
    va_end(args);
    struct timespec t1, t2;

    if(SYS_futex == param1)
    {
        if(FUTEX_WAIT == mutexOperation)
        {
            t1 = TheInterceptor.GetClockTime();

            TheInterceptor.AddCallStack(mutexObj);
            TheInterceptor.Log("Acquiring lock 0x%x", mutexObj);
        }
        if(FUTEX_WAKE == mutexOperation)
        {
            t2 = TheInterceptor.GetClockTime();

            t1 = TheInterceptor.GetLastCallTime(mutexObj);
            TheInterceptor.Log("Releasing lock 0x%x after holding for %i ms", mutexObj, TheInterceptor.GetTimeDiffInMS(t2, t1));
        }
    }

    // Call the original method
    long int ret = _Original_syscall(param1);

    if(SYS_futex == param1)
    {
        if(FUTEX_WAIT == mutexOperation)
        {
            t2 = TheInterceptor.GetClockTime();
            TheInterceptor.Log("Acquired lock 0x%x in %i ms", mutexObj, TheInterceptor.GetTimeDiffInMS(t2, t1));
        }
    }

    return ret;
}
}

extern "C" void FlushLogs()
{
    TheInterceptor.Log("!!!FlushLogs !!!!");
    TheInterceptor.FlushLog();
}
/*
int main()
{
    syscall(SYS_futex, 2, FUTEX_WAIT);
    return 0;
}
*/

