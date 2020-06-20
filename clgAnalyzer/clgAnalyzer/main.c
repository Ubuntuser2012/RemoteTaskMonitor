#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <Windows.h>

#pragma warning(disable:4996)
#define MAX_THREADS 500
#define MAX_PROCESSES 200
#define MAX_TOKENS 32
#define MAX_TOKEN_SZ 256

enum PARSE_STATE
{
   PARSE_STATE_NONE=0,
   PARSE_STATE_THREAD,
   PARSE_STATE_PROCESS
};

typedef struct MULTI_CLG_DATA_
{
   float cpu;
   char* fileName;
   struct MULTI_CLG_DATA_* next;
}
MULTI_CLG_DATA;

typedef struct THREAD_DATA_
{
   int id;
   int runtime;
   float cpu;
   unsigned int priority;
   char name[64];
   struct THREAD_DATA_* left;
   struct THREAD_DATA_* right;
   struct MULTI_CLG_DATA_* mclg;
   int mclgi;
}
THREAD_DATA;

char fileExists(char* fileName)
{
   char ret=0;
   FILE* fp = fopen(fileName, "rb");
   if(fp)
   {
      fclose(fp);
      ret = 1;
   }
   return ret;
}

char insertMclgData(THREAD_DATA* node, float cpu, char* name)
{
   char ret=0;
   MULTI_CLG_DATA *mclg = (MULTI_CLG_DATA*)malloc(sizeof(MULTI_CLG_DATA));
   if(mclg)
   {
      mclg->cpu = cpu;
      mclg->fileName = name;
      mclg->next = 0;

      if(!node->mclg)
      {
         node->mclg = mclg;
      }
      else
      {
         MULTI_CLG_DATA *it;
         char brk=0;
         it = node->mclg;
         while(it)
         {
            if(strcmp(it->fileName, name)==0)
            {
               it->cpu += cpu;
               brk=1;
               break;
            }
            it = it->next;
         }
         if(brk)
            free(mclg);
         else
         {
            it = node->mclg;
            while(it->next)
            {
               it = it->next;
            }
            it->next = mclg;
         }
      }
      ret = 1;
   }
   return ret;
}

THREAD_DATA* dupThreadData(THREAD_DATA* node)
{
   THREAD_DATA* ret = (THREAD_DATA*)malloc(sizeof(THREAD_DATA));
   if(ret)
   {
      memcpy(ret, node, sizeof(THREAD_DATA));
   }
   return ret;
}

THREAD_DATA* insertByName(THREAD_DATA* parent, THREAD_DATA* node, char* clgFile, char SumSameThreadsCpu)
{
   if(parent)
   {
      int res = _stricmp(parent->name, node->name);
      if(res>0)
      {
         if(!parent->left)
         {
            if(clgFile)
            {
               parent->left = dupThreadData(node);
               parent->left->mclgi += insertMclgData(parent->left, node->cpu, clgFile);
            }
            else
               parent->left = node;
         }
         else
            parent->left = insertByName(parent->left, node, clgFile, SumSameThreadsCpu);
      }
      else if(res<0)
      {
         if(!parent->right)
         {
            if(clgFile)
            {
               parent->right = dupThreadData(node);
               parent->right->mclgi += insertMclgData(parent->right, node->cpu, clgFile);
            }
            else
               parent->right = node;
         }
         else
            parent->right = insertByName(parent->right, node, clgFile, SumSameThreadsCpu);
     }
      else//sort by priority
      {
         if(node->priority > parent->priority)
         {
            if(!parent->left)
            {
               if(clgFile)
               {
                  parent->left = dupThreadData(node);
                  parent->left->mclgi += insertMclgData(parent->left, node->cpu, clgFile);
               }
               else
                  parent->left = node;
            }
            else
               parent->left = insertByName(parent->left, node, clgFile, SumSameThreadsCpu);
         }
         else if(node->priority < parent->priority || (!SumSameThreadsCpu && !clgFile))
         {
            if(!parent->right)
            {
               if(clgFile)
               {
                  parent->right = dupThreadData(node);
                  parent->right->mclgi += insertMclgData(parent->right, node->cpu, clgFile);
               }
               else
                  parent->right = node;
            }
            else
               parent->right = insertByName(parent->right, node, clgFile, SumSameThreadsCpu);
         }
         else if(clgFile)
         {
            parent->mclgi += insertMclgData(parent, node->cpu, clgFile);
         }
         else//SumSameThreadsCpu
         {
            parent->cpu += node->cpu;
         }
      }
   }
   return parent;
}

THREAD_DATA* insertByCPU(THREAD_DATA* parent, THREAD_DATA* node)
{
   if(parent)
   {
      if(node->cpu > parent->cpu)
      {
         if(!parent->left)
            parent->left = node;
         else
            parent->left = insertByCPU(parent->left, node);
      }
      else
      {
         if(!parent->right)
            parent->right = node;
         else
            parent->right = insertByCPU(parent->right, node);
      }
   }
   return parent;
}

void printTree(THREAD_DATA* node, FILE *fp)
{
   if(node)
   {
      if(node->left)
         printTree(node->left, fp);
      if(node->cpu>0.01f)
      {
         if(node->priority)
            fprintf(fp, "%s\t%i\t%.02f%% \n", node->name, node->priority, node->cpu);
         else
            fprintf(fp, "%s\t\t%.02f%% \n", node->name, node->cpu);
      }
      if(node->right)
         printTree(node->right, fp);
   }
}

void printTree2(THREAD_DATA* node, WIN32_FIND_DATA *multiFileData, int msz, FILE *fp)
{
   if(node)
   {
      int i;
      MULTI_CLG_DATA *mclg;

      if(node->left)
         printTree2(node->left, multiFileData, msz, fp);

      if(node->priority)
         fprintf(fp, "%s\t%i-\t", *node->name ? node->name : "-", node->priority);
      else
         fprintf(fp, "%s\t-\t", *node->name ? node->name : "-");

      mclg = node->mclg;
      for(i=0; i<msz; i++)
      {
         if(mclg)
         {
            if(strcmp(mclg->fileName, multiFileData[i].cFileName)==0)
            {
               fprintf(fp, "%.02f%%\t", mclg->cpu);
               mclg = mclg->next;
            }
            else
            {
               fprintf(fp, "-\t");
            }
         }
         else
         {
            fprintf(fp, "-\t");
         }
      }
      fprintf(fp, "\n");

      if(node->right)
         printTree2(node->right, multiFileData, msz, fp);
   }
}

char splitString(char* s, char res[MAX_TOKENS][MAX_TOKEN_SZ], int ressz, int resisz, char *delimiter, int numdelimiters)
{
   char ret=0;
   int i=0,j=0, k=0, n=0, d=0;
   char delimit = 0;
   for(i=0; i<(int)strlen(s); i++)
   {
      delimit = 0;
      for(d=0; d<numdelimiters; d++)
      {
         if(s[i]==delimiter[d])
         {
            delimit=1;
            break;
         }
      }
      if(delimit)
      {
         if(k==0)
         {
            j=i+1;
            k=0;
         }
         else if(k>0)
         {
            if(k>=resisz)
               goto Exit;
            if(n>=ressz)
               goto Exit;
            memcpy(res[n], &s[j], k);
            res[n][k]=0;
            j=i+1;
            n++;
            k=0;
         }
      }
      else
      {
         k++;
      }
   }
   if(k>0)
   {
      if(k>=resisz)
         goto Exit;
      if(n>=ressz)
         goto Exit;
      memcpy(res[n], &s[j], k-1);
      res[n][k-1]=0;
      n++;
   }
   ret = 1;
Exit:
   return ret;
}

void  printResults2(char outputFile[1024], WIN32_FIND_DATA *multiFileData, int msz, THREAD_DATA *ps, THREAD_DATA *ts)
{//print results
   char *s=0;
   FILE *fp;
   int j;

   strcpy(outputFile, "all.cpu.txt");   
   fp = fopen(outputFile, "wt");
   if(!fp)
      fp=stdin;

   fprintf(fp, "Process\tPriority");
   for(j=0; j<msz; j++)
   {
      fprintf(fp, "\t%s", multiFileData[j].cFileName);
   }
   fprintf(fp, "\n\n");
   printTree2(ps, multiFileData, msz, fp);

   fprintf(fp, "\n\nThread\tPriority");
   for(j=0; j<msz; j++)
   {
      fprintf(fp, "\t%s", multiFileData[j].cFileName);
   }
   fprintf(fp, "\n\n");
   printTree2(ts, multiFileData, msz, fp);

   fclose(fp);
   printf("Output written to \"%s\"\n", outputFile);

}

void printResults(THREAD_DATA* pdata, int pi, THREAD_DATA* tdata, int ti, char* outputFile, char sortByCpu, char SumSameThreadsCpu)
{
   THREAD_DATA *proot;
   int i;
   char *s=0;
   FILE *fp;
   
   s = strstr(outputFile, ".txt");
   strcpy(s, ".cpu.txt");   
   
   fp = fopen(outputFile, "wt");
   if(!fp)
      fp=stdin;

   fprintf(fp, "================ Processes ================\n");
   proot = &pdata[0];
   for(i=1; i<pi; i++)
   {
      if(sortByCpu)
         insertByCPU(proot, &pdata[i]);
      else
         insertByName(proot, &pdata[i], 0, SumSameThreadsCpu);
   }
   printTree(proot, fp);

   fprintf(fp, "================ Threads ================\n");
   proot = &tdata[0];
   for(i=1; i<ti; i++)
   {
      if(sortByCpu)
         insertByCPU(proot, &tdata[i]);
      else
         insertByName(proot, &tdata[i], 0, SumSameThreadsCpu);
   }
   printTree(proot, fp);

   if(fp)
      fclose(fp);

   printf("Output written to \"%s\"\n", outputFile);
}

char analyze1(char* resFile, THREAD_DATA tdata[], int *maxt, THREAD_DATA pdata[], int *maxp)
{
   char ret=0;
   int hr;
   int min;
   int sec;
   int millisec;
   int microsec;
   int state = PARSE_STATE_NONE;
   FILE* fp = 0;
   char line[2048];
   char token[MAX_TOKENS][MAX_TOKEN_SZ];
   unsigned int runtime=0;
   int ti=0, pi=0;
   char delimiters[4];
   int i = 0;
   if(!resFile)
      goto Exit;
   fp = fopen(resFile, "rt");
   if(!fp)
      goto Exit;
   
   memset(tdata, 0, sizeof(tdata));
   memset(pdata, 0, sizeof(pdata));

   delimiters[0]=' ';
   delimiters[1]=':';
   delimiters[2]='-';
   delimiters[3]='.';

   while(!feof(fp))
   {
      if(!fgets(line, sizeof(line), fp))
         break;
      if(memcmp(line, "Total time  ", 12)==0)
      {
         memset(token, 0, sizeof(token));
         if(splitString(line, token, MAX_TOKENS, MAX_TOKEN_SZ, delimiters, 4)>0)
         {
            hr = atoi(token[2]);
            min = atoi(token[3]);
            sec = atoi(token[4]);
            millisec = atoi(token[5]);
            microsec = atoi(token[6]);
            min += hr*60;
            sec += min*60;
            millisec += sec*1000;
            microsec += millisec*1000;
            runtime = microsec;
         }
         else
         {
            printf("Fatal Error: Increase MAX_TOKENS:%i and/or MAX_TOKEN_SZ:%i\n", MAX_TOKENS, MAX_TOKEN_SZ);
            goto Exit;
         }
      }
      else if(memcmp(line, "Thread information:", 19)==0)
      {
         state = PARSE_STATE_THREAD;
         i = 0;
      }
      else if(memcmp(line, "Process information:", 20)==0)
      {
         state = PARSE_STATE_PROCESS;
         i = 0;
      }
      else if(PARSE_STATE_THREAD == state && i>1)
      {
         if(ti>=*maxt)
         {
            printf("Fatal Error: Exceeded MAX_THREADS:%i\n", MAX_THREADS);
            goto Exit;
         }
         memset(token, 0, sizeof(token));
         if(splitString(line, token, MAX_TOKENS, MAX_TOKEN_SZ, delimiters, 4)>0)
         {
            sscanf_s(token[0], "%x", &tdata[ti].id); 
            hr = atoi(token[3]);
            min = atoi(token[4]);
            sec = atoi(token[5]);
            millisec = atoi(token[6]);
            microsec = atoi(token[7]);
            min += hr*60;
            sec += min*60;
            millisec += sec*1000;
            microsec += millisec*1000;
            tdata[ti].runtime = microsec;
            tdata[ti].cpu = tdata[ti].runtime * 100 / (float) runtime;
            tdata[ti].priority = atoi(token[9]);
            strcpy(tdata[ti].name, token[10]);
            //if(tdata[ti].cpu>0.01f)
            //   printf("%s %.02f \n", tdata[ti].name, tdata[ti].cpu);
            ti++;
         }
         else
         {
            printf("Fatal Error: Increase MAX_TOKENS:%i and/or MAX_TOKEN_SZ:%i\n", MAX_TOKENS, MAX_TOKEN_SZ);
            goto Exit;
         }
     }
      else if(PARSE_STATE_PROCESS == state && i>1)
      {
         if(!strstr(line, "--:--:--.---.---"))
         {
            if(pi>=*maxp)
            {
               printf("Exceeded MAX_PROCESSES:%i - Aborting!\n", MAX_PROCESSES);
               goto Exit;
            }
            memset(token, 0, sizeof(token));
            if(splitString(line, token, MAX_TOKENS, MAX_TOKEN_SZ, delimiters, 4)>0)
            {
               sscanf_s(token[0], "%x", &pdata[pi].id);
               hr = atoi(token[1]);
               min = atoi(token[2]);
               sec = atoi(token[3]);
               millisec = atoi(token[4]);
               microsec = atoi(token[5]);
               min += hr*60;
               sec += min*60;
               millisec += sec*1000;
               microsec += millisec*1000;
               pdata[pi].runtime = microsec;
               pdata[pi].cpu = pdata[pi].runtime * 100.0f / (float) runtime;
               sprintf(pdata[pi].name, "%s.%s", token[12], token[13]);
               //if(pdata[pi].cpu>0.01f)
               //   printf("%s %.02f \n", pdata[pi].name, pdata[pi].cpu);
               pi++;
            }
            else
            {
               printf("Fatal Error: Increase MAX_TOKENS:%i and/or MAX_TOKEN_SZ:%i\n", MAX_TOKENS, MAX_TOKEN_SZ);
               goto Exit;
            }
         }
      }
      i++;
   }
   ret = 1;
   *maxt = ti;
   *maxp = pi;
Exit:
   if(fp)
      fclose(fp);
   return ret;
}

int readDir(char cwd[1024], WIN32_FIND_DATA ** multiFileData, int *msz, char clgTxt)
{
   HANDLE hFindFile;
   WIN32_FIND_DATA findFileData;
   char dir[1024];
   int ret=0;
   int i=0;

   if(clgTxt)
      sprintf(dir, "%s\\*.txt", cwd);
   else
      sprintf(dir, "%s\\*.clg", cwd);

   memset(&findFileData, 0, sizeof(findFileData));
   
   *msz = 0;
   hFindFile = FindFirstFile(dir, &findFileData);
   if(hFindFile)
   {
      do
      {
         if(!clgTxt || !strstr(findFileData.cFileName, "cpu.txt"))
         {
            (*msz)++;

         }
         ret = FindNextFile(hFindFile, &findFileData);
      }
      while(ret);
      FindClose(hFindFile);
   }
   if(*msz)
   {
      *multiFileData = (WIN32_FIND_DATA*) malloc(sizeof(WIN32_FIND_DATA)*(*msz));
      if(*multiFileData)
      {
         memset(*multiFileData, 0, sizeof(WIN32_FIND_DATA)*(*msz));
         hFindFile = FindFirstFile(dir, &findFileData);
         if(hFindFile)
         {
            do
            {
               if(!clgTxt || !strstr(findFileData.cFileName, "cpu.txt"))
               {
                  memcpy(&((*multiFileData)[i]), &findFileData, sizeof(WIN32_FIND_DATA));
                  i++;
               }
               ret = FindNextFile(hFindFile, &findFileData);
            }
            while(ret && i<*msz);
            FindClose(hFindFile);
         }
      }
   }

   return ret;
}

int clgToTxt(char* cwd, char* inputFile, int szInputFile, char* outputFile, int szOutputFile)
{
   char ret=0;
   char *s = 0;
   char cmd[1024];
   char fileName[1024];

   strcpy(outputFile, inputFile);
   s = strstr(outputFile, ".clg");
   if(!s)
   {
      printf("Input file name does not have clg extension\n");
      goto Exit;
   }
   strcpy(s, ".fix");

   sprintf(cmd, "FixCeLog \"%s\" \"%s\"", inputFile, outputFile);
   printf("%s\n", cmd);
   system(cmd);

   sprintf(fileName, "%s\\%s", cwd, outputFile);
   if (!fileExists(fileName))
   {
      printf("FixCeLog.exe failed - \"%s\"\n", fileName);
      goto Exit;
   }

   strcpy(inputFile, outputFile);
   s = strstr(outputFile, ".fix");
   if(!s)
   {
      printf("Could not create \"%s\"\n", outputFile);
      goto Exit;
   }
   strcpy(s, ".txt");

   sprintf(cmd, "readlog -s \"%s\" \"%s\"", inputFile, outputFile);
   printf("%s\n", cmd);
   system(cmd);
   sprintf(fileName, "%s\\%s", cwd, outputFile);
   if (!fileExists(fileName))
   {
      printf("readlog.exe failed - \"%s\"\n", outputFile);
      goto Exit;
   }

   sprintf(fileName, "%s\\%s", cwd, outputFile);
   ret=1;
Exit:
   return ret;
}

int main(int argc, char** argv)
{
   int ret=0;
   char sortByCpu=0;
   char multipleClgFiles=0;
   char inputFile[1024];
   char outputFile[1024];
   char fileName[1024];
   char cwd[1024];
   char* s;
   char SumSameThreadsCpu = 1;
   THREAD_DATA tdata[MAX_THREADS];
   THREAD_DATA pdata[MAX_PROCESSES];
   int ti = MAX_THREADS;
   int pi = MAX_PROCESSES;
   char clgTxt=0;

   switch(argc)
   {
   case 3:
      if(strcmpi(argv[1], "-name")==0)
         sortByCpu = 0;
      else if(strcmpi(argv[1], "-cpu")==0)
         sortByCpu = 1;
      else
      {
         printf("Invalid option: %s\n", argv[1]);
         goto Exit;
      }
      strcpy(inputFile, argv[2]);
      break;
   case 2:
      if(strcmpi(argv[1], "-all-clg")==0)
      {
         multipleClgFiles = 1;
         clgTxt=0;
      }
      else if(strcmpi(argv[1], "-all-txt")==0)
      {
         multipleClgFiles = 1;
         clgTxt=1;
      }
      strcpy(inputFile, argv[1]);
      break;
   default:
      printf("====== clgAnalyzer ======\n");
      printf("clgAnalyzer analyzes clg files which are Kernel Tracker logs. It calculates the CPU %% utilized by running threads and processes.\n");
      printf("Syntax:\n");
      printf("\tclgAnalyzer <input-file-name>.clg\n");
      printf("This executes \"FixCeLog.exe\" followed by \"readlog.exe -s\" on the input file.\n");
      printf("If you already have a txt file output from FixCeLog.exe and readlog.exe, then you can directly feed that txt file:\n");
      printf("\tclgAnalyzer <input-file-name>.txt\n");
      printf("To print results sorted by thread and process names:\n");
      printf("\tclgAnalyzer -name <input>.clg\n");
      printf("To print results sorted by thread and process cpu utilization:\n");
      printf("\tclgAnalyzer -cpu <input>.clg\n");
      printf("To process all clg files in the directory:\n");
      printf("\tclgAnalyzer -all-clg\n");
      printf("To process all txt files in the directory that were created by running \"FixCeLog\" followed by \"readlog -s\":\n");
      printf("\tclgAnalyzer -all-txt\n");
      goto Exit;
   }

   if(!GetCurrentDirectory(sizeof(cwd), cwd))
   {
      printf("Could not retrieve working directory\n");
      goto Exit;
   }
   sprintf(fileName, "%s\\FixCeLog.exe", cwd);
   if (!fileExists(fileName))
   {
      printf("FixCeLog.exe not in working directory\n");
      goto Exit;
   }
   sprintf(fileName, "%s\\readlog.exe", cwd);
   if (!fileExists(fileName))
   {
      printf("readlog.exe not in working directory\n");
      goto Exit;
   }

   if(multipleClgFiles)
   {
      THREAD_DATA ps, ts;
      WIN32_FIND_DATA *multiFileData = 0;
      int msz=0;
      int i, j;
      readDir(cwd, &multiFileData, &msz, clgTxt);
      if(msz && multiFileData)
      {
         for(i=0; i<msz; i++)
         {
            ti = MAX_THREADS;
            pi = MAX_PROCESSES;
            memset(tdata, 0, sizeof(tdata));
            memset(pdata, 0, sizeof(pdata));
            strcpy(inputFile, multiFileData[i].cFileName);
            s = strstr(inputFile, ".txt");
            if(s)
            {
               ret = analyze1(inputFile, tdata, &ti, pdata, &pi);
            }
            else if(clgToTxt(cwd, inputFile, sizeof(inputFile), outputFile, sizeof(outputFile)))
            {
               ret = analyze1(outputFile, tdata, &ti, pdata, &pi);
            }
            if(i==0)
            {
               memcpy(&ps, &pdata[0], sizeof(THREAD_DATA));
               memcpy(&ts, &tdata[0], sizeof(THREAD_DATA));
            }
            for(j=0; j<pi; j++)
            {
               insertByName(&ps, &pdata[j], multiFileData[i].cFileName, 0);
            }
            for(j=0; j<ti; j++)
            {
               insertByName(&ts, &tdata[j], multiFileData[i].cFileName, 0);
            }
         }
         printResults2(outputFile, multiFileData, msz, &ps, &ts);
         free(multiFileData);
         msz=0;
      }
   }
   else
   {
      ti = MAX_THREADS;
      pi = MAX_PROCESSES;
      memset(tdata, 0, sizeof(tdata));
      memset(pdata, 0, sizeof(pdata));
      s = strstr(inputFile, ".txt");
      if(s)
      {
         ret = analyze1(inputFile, tdata, &ti, pdata, &pi);
         strcpy(outputFile, inputFile);
      }
      else if(clgToTxt(cwd, inputFile, sizeof(inputFile), outputFile, sizeof(outputFile)))
      {
         ret = analyze1(outputFile, tdata, &ti, pdata, &pi);
      }
      if(ret)
      {
         printResults(pdata, pi, tdata, ti, outputFile, sortByCpu, SumSameThreadsCpu);
      }
   }

Exit:
   return ret;
}