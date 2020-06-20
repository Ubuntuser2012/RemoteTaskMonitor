#include "../common/types.h"
#include "../common/util.h"
#include <time.h>

#ifdef windows
#include "cemon.h"
#elif defined(linux)
#include "linuxrtm.h"
#endif

#pragma warning(disable:4996)
//#pragma warning(default:4996)

static CLIENT arr_client[MAX_CLIENTS];
static int num_clients;

int GetNumClients() 
{ 
   return num_clients; 
}

void winsock_end()
{
#ifdef windows
   WSACleanup();
#endif

}

#ifdef windows
int winsock_init()
{
   int ret = 0; 
   WSADATA wsaData;

   ret = WSAStartup( MAKEWORD(2,2), &wsaData );
   if ( ret != 0 )
   {
      logg(TXT("WSAStartup err %i\n"), ret);
      goto Exit;
   }

   ret = 1;
Exit:
   return ret;
}
#endif

#ifdef linux
void closesocket(SOCKET s)
{
	close(s);
}

int lastError() 
{ 
	return errno; 
}

#endif

int socket_write(SOCKET s, const unsigned char *buf, int len, char block)
{
   int bytesSent = 0;
   int error = 0;
   int ret = 0;

   if(!block)
   {
      fd_set writefds;
      struct timeval timeout;//
      FD_ZERO(&writefds);
      FD_SET(s, &writefds);

      timeout.tv_sec = 10;
      timeout.tv_usec = 0;

      ret = select(1, 0, &writefds, 0, &timeout);
      switch(ret)
      {
      case 1://socket is ready for writing
         bytesSent = send(s, buf+bytesSent, len-bytesSent, 0);
         if(SOCKET_ERROR == bytesSent)
         {
            error = GetLastError();
            if(WSAEWOULDBLOCK == error)
            {
               bytesSent = -WSAEWOULDBLOCK;
               goto Exit;
            }
            logg(TXT("send() failed %i\n"), error);
         }
         else if(bytesSent < len)
         {
            logg(TXT("send() incomplete\n"));
         }
         else
         {
            //logg(TXT("sent %i bytes\n"), bytesSent);
         }
         break;
      case 0://timeout
         error = GetLastError();
         bytesSent = -1;//treat as error
         break;
      default:
         error = GetLastError();
         logg(TXT("send() failed %i\n"), error);
         bytesSent = -1;
      }
   }
   else
   {
      bytesSent = send(s, buf+bytesSent, len-bytesSent, 
#ifdef windows
         0);
#elif defined(linux)
         MSG_NOSIGNAL);
#endif
      if(SOCKET_ERROR == bytesSent)
      {
         error = GetLastError();
         logg(TXT("send() failed %i\n"), error);
      }
      else if(bytesSent < len)
      {
         logg(TXT("send() incomplete\n"));
      }
      else
      {
         //logg(TXT("sent %i bytes\n"), bytesSent);
      }
   }
Exit:
   return bytesSent;
}

int socket_read(SOCKET s, unsigned char *buf, int len, char block)
{
   int error = 0;
   int bytesReceived = recv(s, buf, len, 0);

   if(SOCKET_ERROR == bytesReceived)
   {
      error = GetLastError();
      if(!block)
      {
         if(WSAEWOULDBLOCK == error)
         {
            bytesReceived = -WSAEWOULDBLOCK;
            goto Exit;
         }
      }
      logg(TXT("recv() failed %i\n"), error);
   }
   else if( bytesReceived > 0)
   {
   }
   else if(bytesReceived == 0)
   {
      //socket was closed
      int error = GetLastError();
      error = 0;
   }
Exit:
   return bytesReceived;
}

void socket_server_end(SOCKET s, char block)
{
   int i;
   COMM_MESG end;
   end.cmd = COMM_CMD_CLOSE;
   end.numelm = 0;
   end.sz = sizeof(COMM_MESG);
   for(i=0; i<num_clients; i++)
   {
      socket_write(arr_client[i].so, (const unsigned char*)&end, end.sz, block);
      shutdown(arr_client[i].so, SD_BOTH);
      logg(TXT("closing client socket: 0x%x\n"), arr_client[i].so);
      closesocket(arr_client[i].so);
   }

   num_clients = 0;
   memset(arr_client, 0, MAX_CLIENTS);

   logg(TXT("socket_server_end\n"));
   closesocket(s);
}

SOCKET socket_server_init(short port, char block)
{
   int ret=0;
   int error = 0;
   unsigned long iMode = 1;//non-blocking
   struct sockaddr_in stSockAddr;
   SOCKET SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

   if(-1 == SocketFD)
   {
      logg(TXT("can not create socket\n"));
      goto Exit;
   }
   if(!block)
   {
#ifdef windows
      if(ioctlsocket(SocketFD, FIONBIO, &iMode) != 0)
#elif defined(linux)
      if(fcntl(SocketFD, F_SETFL, O_NONBLOCK) != 0)
#endif
      {
         error = GetLastError();
         logg(TXT("ioctlsocket() failed: %i\n"), error);
         goto Exit;
      }
   }
   memset(&stSockAddr, 0, sizeof stSockAddr);

   port = (port == 0) ? htons(SERVER_PORT) : htons(port);
   stSockAddr.sin_family = AF_INET;
   stSockAddr.sin_port = port;
   stSockAddr.sin_addr.s_addr = INADDR_ANY;
   //stSockAddr.sin_addr.s_addr = 0;//inet_addr("172.17.1.1");
   //stSockAddr.sin_addr.S_un.S_addr = htonl(stSockAddr.sin_addr.S_un.S_addr);

   if(-1 == bind(SocketFD,(struct sockaddr *)&stSockAddr, sizeof stSockAddr))
   {
      error = GetLastError();
      logg(TXT("bind() failed: %i - 0x%x:%i\n"), error, ntohl(stSockAddr.sin_addr.s_addr), ntohs(stSockAddr.sin_port));
      closesocket(SocketFD);
      goto Exit;
   }

   if(-1 == listen(SocketFD, 10))
   {
      error = GetLastError();
      logg(TXT("listen() failed: %i\n"), error);
      closesocket(SocketFD);
      goto Exit;
   }

   logg(TXT("listening on port %i\n"), ntohs(stSockAddr.sin_port));

   ret = 1;
Exit:
   return ret ? SocketFD : 0;
}

SOCKET socket_accept(SOCKET SocketFD, char block)
{
   SOCKET ret=0;
   int error = 0;
   unsigned int addrlen = sizeof(struct sockaddr_in);
   struct sockaddr_in stSockAddr;

   memset(&stSockAddr, 0, addrlen);


   ret = accept(SocketFD, (struct sockaddr*)&stSockAddr, &addrlen);

   if(ret == INVALID_SOCKET)
   {
      error = GetLastError();
      if(!block)
      {
         if(WSAEWOULDBLOCK == error)
            goto Exit;
      }
      logg(TXT("error accept failed %i\n"), error);
      closesocket(SocketFD);
      ret = 0;
   }
   else if(ret)
   {
      if(num_clients < MAX_CLIENTS)
      {
		unsigned long ip = 0;
         mytime(&arr_client[num_clients].last_activity);
         arr_client[num_clients].so = ret;
         arr_client[num_clients].state=CLIENT_SOCKET_STATE_CONNECTED;
         arr_client[num_clients].rdbuffer_head=0;
         arr_client[num_clients].rdbuffer_tail=0;
         arr_client[num_clients].rdbuffer_size=sizeof(arr_client[num_clients].rdbuffer);
         num_clients++;
		 ip = ntohl(stSockAddr.sin_addr.s_addr);
         logg(TXT("accepted client # %i %i.%i.%i.%i:%i, socket 0x%x\n"), num_clients, 
			 (ip>>24)&0xff, 
			 (ip>>16)&0xff, 
			 (ip>>8)&0xff, 
			 ip&0xff, ntohs(stSockAddr.sin_port), ret);
      }
      else
      {
         COMM_MESG msg;
         msg.cmd = COMM_CMD_CLIENTS_MAXED;
         msg.numelm = MAX_CLIENTS;
         msg.sz = sizeof(msg);
         socket_write(ret, (const unsigned char*)&msg, msg.sz, block);
         closesocket(ret);
         logg(TXT("cannot serve more than %i clients\n"), MAX_CLIENTS);
         ret = 0;
      }
   }
Exit:
   return ret;
}

int socket_find_client(SOCKET client_so)
{
    int i = -1;
    if(num_clients>0)
    {
        for(i=0; i<num_clients; i++)
        {
           if((arr_client[i].state & CLIENT_SOCKET_STATE_CONNECTED) && (client_so == arr_client[i].so))
              break;
        }
    }
    return i;
}

int socket_read_clienti(int i, PROCESS_CLIENT_REQUEST process_client_request, char block)
{
    int ret = 0;
    if(i>-1 && i<num_clients)
    {
       if(arr_client[i].rdbuffer_size-arr_client[i].rdbuffer_tail > 0)
       {

//logg(TXT("arr_client[i].rdbuffer_head %i\n"), arr_client[i].rdbuffer_head);
//logg(TXT("arr_client[i].rdbuffer_tail %i\n"), arr_client[i].rdbuffer_tail);
//logg(TXT("arr_client[i].rdbuffer_size %i\n"), arr_client[i].rdbuffer_size);
//logg(TXT("buffer 0x%x\n"), &arr_client[i].rdbuffer[arr_client[i].rdbuffer_tail]);
          ret=socket_read(arr_client[i].so, &arr_client[i].rdbuffer[arr_client[i].rdbuffer_tail], arr_client[i].rdbuffer_size-arr_client[i].rdbuffer_tail, block);

          if(!block && ret==-WSAEWOULDBLOCK)
          {
             goto Exit;
          }
          else if(ret<=0)
          {
			 logg(TXT("socket error in recv, ret %i\n"), ret);
             socket_close_client(arr_client[i].so);
             goto Exit;//error
          }
          else if(ret>0)
          {
//			 logg(TXT("recvd %i bytes\n"), ret);
	         mytime(&arr_client[i].last_activity);
             arr_client[i].rdbuffer_tail+=ret;
          }
       }//if(arr_client[i].rdbuffer_size-arr_client[i].rdbuffer_tail > 0)

       while(arr_client[i].rdbuffer_head < arr_client[i].rdbuffer_tail)
       {
          process_client_request(arr_client[i].so, arr_client[i].rdbuffer, &arr_client[i].rdbuffer_head, arr_client[i].rdbuffer_tail);
          if(arr_client[i].rdbuffer_head>=arr_client[i].rdbuffer_tail)
          {
             arr_client[i].rdbuffer_head=arr_client[i].rdbuffer_tail=0;
             break;
          }
       }

       memmove(&arr_client[i].rdbuffer[0], &arr_client[i].rdbuffer[arr_client[i].rdbuffer_head], arr_client[i].rdbuffer_tail-arr_client[i].rdbuffer_head);
       arr_client[i].rdbuffer_tail=arr_client[i].rdbuffer_tail-arr_client[i].rdbuffer_head;
       arr_client[i].rdbuffer_head=0;
    }
    ret = 1;
Exit:
    return ret;
}

void socket_read_client(SOCKET client_so, PROCESS_CLIENT_REQUEST process_client_request, char block)
{
  int i = socket_find_client(client_so);
  socket_read_clienti(i, process_client_request, block);
}


int socket_write_clients(const unsigned char *buf, int len, char state, char block)
{
	int ret = 0;
   if(num_clients>0)
   {
      int i;
      for(i=0; i<num_clients; i++)
      {
         if(arr_client[i].state & state)
         {
            if(SOCKET_ERROR==socket_write(arr_client[i].so, buf, len, block))
            {
               socket_close_client(arr_client[i].so);
            }
			else
			{
	            mytime(&arr_client[i].last_activity);
				ret++;
			}
         }
      }
   }
   return ret;
}

void socket_write_client(SOCKET s, const unsigned char *buf, int len, char block)
{
   if(SOCKET_ERROR==socket_write(s, buf, len, block))
   {
      socket_close_client(s);
   }
}

void socket_set_client_state(SOCKET s, unsigned char state)
{
   if(num_clients>0)
   {
      int i;
      for(i=0; i<num_clients; i++)
      {
         if(arr_client[i].so == s)
         {
            arr_client[i].state |= state;
         }
      }
   }
}

void socket_unset_client_state(SOCKET s, unsigned char state)
{
   if(num_clients>0)
   {
      int i;
      for(i=0; i<num_clients; i++)
      {
         if(!s || arr_client[i].so == s)
         {
            arr_client[i].state &= ~state;
         }
      }
   }
}

void socket_unset_clients_state(unsigned char state)
{
   if(num_clients>0)
   {
      int i;
      for(i=0; i<num_clients; i++)
      {
         arr_client[i].state &= ~state;
      }
   }
}

unsigned char socket_is_any_client_state(unsigned char state)
{
   if(num_clients>0)
   {
      int i;
      for(i=0; i<num_clients; i++)
      {
         if(arr_client[i].state & state)
         {
            return 1;
         }
      }
   }
   return 0;
}

void socket_close_client(SOCKET ConnectFD)
{
   logg(TXT("close socket 0x%x, num_clients %i\n"), ConnectFD, num_clients);
   if(num_clients>0)
   {
      int i;
      for(i=0; i<num_clients; i++)
      {
         if(arr_client[i].so==ConnectFD)
         {
            arr_client[i].so = 0;
            memset(&arr_client[i].last_activity, 0, sizeof(arr_client[i].last_activity));
            arr_client[i].rdbuffer_head=0;
            arr_client[i].rdbuffer_tail=0;
            num_clients--;
            break;
         }
      }
      if(i<num_clients)
      {
         memmove(&arr_client[i].so, &arr_client[i+1].so, num_clients-i);
      }
   }
   shutdown(ConnectFD, SD_BOTH);
   closesocket(ConnectFD);
   logg(TXT("num_clients %i\n"), num_clients);
}

SOCKET socket_client_init()
{
   int ret=0;
   int error = 0;
   SOCKET SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

   if(-1 == SocketFD)
   {
      error = GetLastError();
      logg(TXT("can not create socket %i\n"), error);
      goto Exit;
   }


   ret = 1;
Exit:
   return ret ? SocketFD : 0;
}

int socket_client_connect(SOCKET SocketFD, const char* ip, short port, int *error)
{
	return socket_client_connect2(SocketFD, inet_addr(ip), port, error);
}

int socket_client_connect2(SOCKET SocketFD, ULONG ip, short port, int *error)
{
   int ret = 0;
   struct sockaddr_in stSockAddr;
   memset(&stSockAddr, 0, sizeof stSockAddr);

   stSockAddr.sin_family = AF_INET;
   stSockAddr.sin_port = htons(port);
   stSockAddr.sin_addr.S_un.S_addr = ip;

#ifdef windows
   if (INADDR_NONE == stSockAddr.sin_addr.S_un.S_addr)
#elif defined(linux)
   if (0 == stSockAddr.sin_addr.s_addr)
#else
      logg(TXT("???\n"));
#endif
   {
      logg(TXT("inet_addr() error\n"));
      goto Exit;
   }

   if (SOCKET_ERROR == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof stSockAddr))
   {
      *error = GetLastError();
      if(WSAEWOULDBLOCK != *error)
      {
         logg(TXT("connect() failed %i\n"), *error);
         goto Exit;
      }
   }

   ret = 1;
Exit:
   return ret;
}

int socket_set_broadcast(SOCKET SocketFD)
{
	int ret = 0;
	int error = 0;
	char optval = 1;//broadcast mode enabled
	if (setsockopt(SocketFD, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) != 0)
	{
		error = GetLastError();
		logg(TXT("setsockopt() failed %i\n"), error);
		goto Exit;
	}
	ret = 1;
Exit:
	return ret;
}
int socket_nonblocking(SOCKET SocketFD)
{
   int ret =0;
   int error = 0;
   unsigned long iMode = 1;//non-blocking
   if(ioctlsocket(SocketFD, FIONBIO, &iMode) != 0)
   {
      error = GetLastError();
      logg(TXT("ioctlsocket() failed %i\n"), error);
      goto Exit;
   }
   ret = 1;
Exit:
   return ret;
}

int socket_blocking(SOCKET SocketFD)
{
   int ret =0;
   int error = 0;
   unsigned long iMode = 0;//blocking
   if(ioctlsocket(SocketFD, FIONBIO, &iMode) != 0)
   {
      error = GetLastError();
      logg(TXT("ioctlsocket() failed %i\n"), error);
      goto Exit;
   }
   ret = 1;
Exit:
   return ret;
}

int socket_ready(CLIENT **clients, fd_set *readfds, int timeout_s, int timeout_ms)
{
  int ret = 0;
  if(num_clients > 0)
   {
       int error = 0;
       int i;
       struct timeval timeout;

       FD_ZERO(readfds);

       for(i=0; i<num_clients; i++)
       {
          FD_SET(arr_client[i].so, readfds);
       }

       timeout.tv_sec = timeout_s;
       timeout.tv_usec = timeout_ms;

       ret = select(num_clients, readfds, 0, 0, &timeout);
       if(ret>0)
       {
          ret = ret;
       }
       else if(ret==0)//timeout
       {
          ret = 0;
          //logg(TXT("socket_ready() timeout\n"));
       }
       else
       {
          error = GetLastError();
          logg(TXT("socket_ready() err %i\n"), error);
          ret = -1;
       }
   }
   
   *clients = arr_client;
   return ret;
}

int socket_remove_overdue_clients(char block)
{
   int del=0;
   int i;
   FILETIME now;
   mytime(&now);

   for(i=0; i<num_clients; )
   {
      long64 diff = subFileTime(&now, &arr_client[i].last_activity);
      long64 diff2 =  diff / 10000000;
      if(diff2 >= WAIT_ON_SOCKET_TIMEOUT_S)
      {
            socket_close_client(arr_client[i].so);
            del++;
      }
      else
      {
         i++;
      }
   }

   return del;
}
