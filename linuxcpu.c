#include "../common/types.h"
#include "../common/util.h"

//#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
//#include </usr/src/linux-headers-2.6.32-33/include/linux/jiffies.h>
//#include </usr/src/linux-headers-2.6.32-33-generic/include/linux/jiffies.h>
//#include <linux/jiffies.h>

#include "linuxrtm.h"

/// gcc linuxcpu.c -I/usr/src/linux-headers-2.6.32-33/include -lrt -o test
/// gcc linuxcpu.c -lrt -o test


struct sigaction sa;

void sighandler(int signum, siginfo_t *info, void *ptr)
{
    switch(signum)
    {
    case SIGTERM:
        printf("Received SIGTERM (%d)\n", signum);
        break;
    case SIGABRT:
        printf("Received SIGABRT (%d)\n", signum);
        break;
    case SIGSEGV:
        printf("Received SIGSEGV (%d)\n", signum);
        break;
    default:
        printf("Received signal %d\n", signum);
    }

    printf("Signal originates from process %lu\n",
        (unsigned long)info->si_pid);

    dieRtm();
}


void sigregister()
{
    int ret;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGABRT);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGSEGV);
    sa.sa_sigaction = sighandler;
    sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
    ret = sigaction(SIGTERM, &sa, NULL);
    ret = sigaction(SIGSEGV, &sa, NULL);
    ret = sigaction(SIGABRT, &sa, NULL);
}

void Sleep(int ms)
{
	usleep(ms*1000);//convert to microseconds
}

void getMemoryInfo(MY_MEM_INFO *memstatus)
{
    FILE* fp = fopen("/proc/meminfo", "r");
    memset(memstatus, 0, sizeof(MY_MEM_INFO));
    if(fp)
    {
        char_t res[MAX_TOKENS][MAX_TOKEN_SZ];
        char_t delimiter[3]={":"};
        char line[1024];
        char gotTotalMem = 0;
        char gotFreeMem = 0;
        char stmp[64];
        while(fgets(line, sizeof(line), fp) > 0)
        {
            if(1==sscanf(line, "MemTotal:%s", stmp))
            {
                memstatus->totalMem = atoi(stmp);//kb
                gotTotalMem = 1;
            }
            else if(1==sscanf(line, "MemFree:%s", stmp))
            {
                memstatus->freeMem = atoi(stmp);//kb
                gotFreeMem = 1;
            }
            if(gotTotalMem && gotFreeMem)
                break;
        }
        fclose(fp);
    }
}


void getSystemInfo(MY_SYSTEM_INFO *sysinfo)
{
    FILE* fp = fopen("/proc/cpuinfo", "r");

    memset(sysinfo, 0, sizeof(MY_SYSTEM_INFO));
    //sysinfo->pid = getpid();
    sysinfo->os = MY_OS_LINUX;
    sysinfo->l.numberOfProcessors = 0;
/*
processor	: 0
vendor_id	: GenuineIntel
cpu family	: 6
model		: 26
model name	: Intel(R) Xeon(R) CPU           W3520  @ 2.67GHz
 */
    if(fp)
    {
        char_t res[MAX_TOKENS][MAX_TOKEN_SZ];
        char_t delimiter[3]={":"};
        memset(res, 0, sizeof(res));
        char line[1024];
        char stmp[64];
        unsigned long ultmp=0;
        while(fgets(line, sizeof(line), fp) > 0)
        {
            if(1==sscanf(line, "processor	: %lu", &ultmp))
                sysinfo->l.numberOfProcessors = ultmp+1;
            else if(1==sscanf(line, "model name	: %s", stmp))
            {
                char *s = strchr(line, ':');
                if(s)
                    wcscpy2(sysinfo->l.processorArchitecture, s+1, sizeof(sysinfo->l.processorArchitecture)/sizeof(sysinfo->l.processorArchitecture[0]));
            }
        }
        fclose(fp);
    }
}

unsigned long timeElapsed()
{
	unsigned long now_ms = 0;
	struct timespec tp;
    mytime(&tp);
	now_ms = tp.tv_sec*1000 +  tp.tv_nsec/1000000;//sec to ms + nano to ms
	return now_ms;
}

int GetLastError()
{
    return errno;
}

#if 0//todo replace splitString
unsigned long get_cpu_time()
{
    unsigned long total_cpu_time = 0;
    FILE* fp = fopen("/proc/stat", "r");
    if(fp)
    {
        char *end = 0;
        char buffer[1024];
        char_t res[MAX_TOKENS][MAX_TOKEN_SZ];
        char_t delimiter[3]={" \t\n"};

        fread(buffer, 1, sizeof(buffer), fp);
        fclose(fp);

        end = strchr(buffer, '\n');
        if(end)
            *end = 0;
        //printf("%s\n", buffer);

        memset(res, 0, sizeof(res));
        if(splitString(buffer, sizeof(buffer), res, MAX_TOKENS, MAX_TOKEN_SZ, delimiter, 3))
        {
            int i;
            total_cpu_time = 0;
/*            total_cpu_time += strtoul(res[1], 0, 10);//normal processes executing in user mode
            total_cpu_time += strtoul(res[2], 0, 10);//niced processes executing in user mode
            total_cpu_time += strtoul(res[3], 0, 10);//processes executing in kernel mode
*/			for(i=1; res[i][0]; i++)
            {
                //printf("[%i]=>%s, ", i, res[i]);
                total_cpu_time += strtoul(res[i], 0, 10);;
            }
        }
        else printf("split failed\n");

        //printf("total_cpu_time: %lu jiffies\n", total_cpu_time);
    }
    return total_cpu_time;
}
#endif

int get_ps_path(int pid, char_t* path, int sz)
{
	int ret = 0;
	char exe[128];
	sprintf(exe, "/proc/%i/exe", pid);
	//printf("%s\n", stat);

	ret = readlink(exe, path, sz);
	return ret;
}

void kill_ps(int pid)
{
	kill(pid, SIGKILL);
}

int start_ps(char_t* path)
{
	int ret = 0;
	int pid = fork();
	switch(pid)
	{
		case -1://error
			break;
		case 0://child process
			execve(path, 0, 0);
			exit(0);
			break;
		default://this process
			ret = 1;
	}
	return ret;
}

void kill_thread(int tid)
{
	pthread_kill(tid, SIGKILL);
}

void suspend_thread(int tid)
{
	pthread_kill(tid, SIGSTOP);
}

void resume_thread(int tid)
{
	pthread_kill(tid, SIGCONT);
}

int set_thread_priority(int id, int priority)
{
	return 0;
}

int get_thread_priority(int id)
{
	return 0;
}

/*
void iter(const char* path, int isps)
{
    DIR* pDir = opendir(path);
	struct dirent* pEntry;
    printf("==>%s<===\n", path);
    while ( (pEntry = readdir ( pDir)) )
	{
		if ( DT_DIR & pEntry->d_type &&
			strcmp ( pEntry->d_name, ".") &&
			strcmp ( pEntry->d_name, "..") &&
			isdigit(pEntry->d_name[0]))
		{
			int id = atoi(pEntry->d_name);
			char stats[1024];
			char tasks[1024];

			if(isps)
			{
				char abspath[1024];
				get_ps_path(id, abspath, sizeof(abspath));
				printf("[pid %i] %s\n", id, abspath);
			}
			else
			{
				printf("[tid %i]\n", id);
			}

			sprintf(stats, "%s/%s/stat", path, pEntry->d_name);//d_name will be the process or thread id
			sprintf(tasks, "%s/%s/task", path, pEntry->d_name);
            process_stat(stats, 0);

			if(isps)
			{
                iter(tasks, 0);
			}
			printf("============================\n");
		}//if
	}//while
}
*/
void setCount(DWORD n, DWORD *tn, DWORD *pn, bool isps)
{
    if(isps)
        *pn = n;
    else
        *tn = n;
}

void process_stat(const char* stat, EXECOBJ_DATA *p)
{
    FILE* fp = fopen(stat, "r");
    if(fp)
    {
        char buffer[1024];
        fread(buffer, 1, sizeof(buffer), fp);
        fclose(fp);

        //printf("%s\n", buffer);
/*
1978 (indicator-sessi) S 1 1824 1824 0 -1 4202496 1360 0 1 0 6 6 0 0 20 0 1 0 76
96 18419712 1165 4294967295 134512640 134548648 3217357264 3217356768 10810402 0
 0 4096 0 3223426422 0 0 17 1 0 0 0 0 0
*/
        {
            unsigned long ultmp;
            int itmp;
            char stmp[32];
            char ctmp;
            unsigned long utime, stime, priority, nice, mem, rt_priority, policy;
            sscanf(buffer, "%lu %s %c %lu %lu %lu %lu %i %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"
                   "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                   &ultmp, stmp, &ctmp, &ultmp, &ultmp, &ultmp, &ultmp, &itmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &utime, &stime, &ultmp, &ultmp, &priority, &nice, &ultmp, &ultmp, &ultmp,
                   &mem, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &rt_priority, &policy, &ultmp, &ultmp, &ultmp, &ultmp);
            //printf("1 utime %lu, stime %lu, priority %lu, nice %lu, mem %lu, rt_priority %lu, policy %lu\n",
            //       utime, stime, priority, nice, mem, rt_priority, policy);
            {
                    long user_hz = sysconf(_SC_CLK_TCK);//ticks per second
                    p->ft = timeElapsed();
                    //mytime(&p->ft);
                    p->cpuTimeSpent = (utime + stime)*1000/user_hz;//conversion to milliseconds
                    p->priority = priority;
                    p->nice = nice;
                    //p->num_threads = strtoul(res[19], &endptr, 10);//ld
                    p->mem = mem;
                    p->rt_priority = rt_priority;
                    p->policy = policy;
            }
        }
    }
}

void thread_stat(const char* stat, EXECOBJ_DATA *p)
{
    FILE* fp = fopen(stat, "r");
    if(fp)
    {
        char buffer[1024];
        fread(buffer, 1, sizeof(buffer), fp);
        fclose(fp);

        //printf("%s\n", buffer);
/*
2820 (ServiceThread) S 2771 2774 2774 0 -1 8256 1008 468 3 0 931 495 0 0 20 0 10 0 8955 317997056 27772 4294967295 134512640 134528211 322122
2960 3084064832 1233954 0 0 16789504 83655 3223467686 0 0 -1 2 0 0 0 0 0
*/
        {
            unsigned long ultmp;
            int itmp;
            char stmp[32];
            char ctmp;
            unsigned long utime, stime, priority, nice, mem, rt_priority, policy;
            sscanf(buffer, "%lu %s %c %lu %lu %lu %lu %i %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"
                   "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                   &ultmp, stmp, &ctmp, &ultmp, &ultmp, &ultmp, &ultmp, &itmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &utime, &stime, &ultmp, &ultmp, &priority, &nice, &ultmp, &ultmp, &ultmp,
                   &mem, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &ultmp, &rt_priority, &policy, &ultmp, &ultmp, &ultmp, &ultmp);
            //printf("1 utime %lu, stime %lu, priority %lu, nice %lu, mem %lu, rt_priority %lu, policy %lu\n",
            //       utime, stime, priority, nice, mem, rt_priority, policy);
            {
                    long user_hz = sysconf(_SC_CLK_TCK);//ticks per second

                    memset(p->name, 0, sizeof(p->name));
                    //wcscpy4(stmp, p->name, sizeof(p->name)/sizeof(char2_t));
                    wcscpy2(p->name, stmp, sizeof(p->name)/2-2);

                    p->ft = timeElapsed();
                    //mytime(&p->ft);
                    p->cpuTimeSpent = (utime + stime)*1000/user_hz;//conversion to milliseconds
                    p->priority = priority;
                    p->nice = nice;
                    //p->num_threads = strtoul(res[19], &endptr, 10);//ld
                    p->mem = mem;
                    p->rt_priority = rt_priority;
                    p->policy = policy;
            }
        }
    }
}

bool getPsStats(const char *path, bool isps, DWORD ppid, COMM_MESG **msg, DWORD *pidsz, DWORD *tidsz, DWORD *pn, DWORD *tn)
{
    bool lRes = 0;
    struct dirent* pEntry;
    EXECOBJ_DATA *data = isps ? (EXECOBJ_DATA*)(&msg[COMM_CMD_PDATA][1]) :
                                (EXECOBJ_DATA*)(&msg[COMM_CMD_TDATA][1]);
    DWORD cnt = isps ? *pidsz : *tidsz;
    DWORD n = isps ? *pn : *tn;
    DIR* pDir = opendir(path);

    if(!pDir)
    {
        logg(TXT("getPsStats - could not open direcotry %s\n"), path);
        goto Exit;
    }
    for(; (pEntry = readdir ( pDir));)
    {
        int id = 0;
        char stats[1024];
        char abspath[1024];
        char *exe = 0;
        int exesz = 0;
        int m;

        if ( !(DT_DIR & pEntry->d_type) ||
            strcmp ( pEntry->d_name, ".")==0 ||
            strcmp ( pEntry->d_name, "..")==0)
            continue;

        id = atoi(pEntry->d_name);
        if(id <= 0)
            continue;

        if(!expandList(msg, pidsz, tidsz, n, isps))
            goto Exit;

        memset(abspath, 0, sizeof(abspath));
        get_ps_path(id, abspath, sizeof(abspath));
        if(!abspath[0])
            continue;

        m = sizeof(data[n].name);
        exe = strrchr(abspath, '/');
        memset(data[n].name, 0, n);
        if(exe)
        {
            char *e = &exe[1];
            wcscpy2(data[n].name, e, m/2-2);
        }

        sprintf(stats, "%s/%s/stat", path, pEntry->d_name);//d_name will be the process or thread id
        process_stat(stats, &data[n]);

        setCount(n, tn, pn, isps);
        if(isps)
        {
            char tpath[1024];
            sprintf(tpath, "%s/%s/task", path, pEntry->d_name);

            data[n].pid = id;
            data[n].tid = 0;
            getPsStats(tpath, !isps, id, msg, pidsz, tidsz, pn, tn);
        }
        else
        {
            data[n].pid = ppid;
            data[n].tid = id;
            data[n].mem = 0;

            char tpath[1024];
            sprintf(tpath, "%s/%s/stat", path, pEntry->d_name);

            thread_stat(tpath, &data[n]);
        }
        n++;
    }
    setCount(n, tn, pn, isps);
    closedir(pDir);
    return 1;
Exit:
    return 0;
}

/*
int main(int argc, char **argv)
{
	int n;
	for(n=0; n<2; n++)
	{
		iter("/proc", 1);
		sleep(2);
	}//for
	return 0;
}
*/

