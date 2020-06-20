//http://www.faqs.org/rfcs/rfc1426.html
//http://www.faqs.org/rfcs/rfc1341.html
//http://www.oac.uci.edu/indiv/ehood/MIME/rfc2183.txt
//http://www.cis.ohio-state.edu/htbin/rfc/rfc2045.html
//http://www.faqs.org/rfcs/rfc1521.html

//http://www.expta.com/2010/03/how-to-use-telnet-to-send-smtp-email-to.html

#pragma warning( disable: 4996 )

//#include "../socket/socket.h"
#include "stdafx.h"
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include "../common/types.h"
#include "../ds/ds.h"
#include "smtp.h"
extern "C"{
#include "comm.h"
   };
#pragma comment (lib, "WS2_32.lib")

char SMTP::m_key[] = {'A', 
	'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 
	'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 
	'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 
	'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 
	'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', 
	'9', '+', '/', '='
};


void SMTP::Base64Encode(char* pFileName, unsigned char** ppEncodedData, unsigned long *pSize)
{
	unsigned long	i;
	unsigned long	j;
	unsigned char*	pFileIn=0;
	unsigned char*	pFileOut;
	unsigned long	padding=0;
	unsigned long	encodedsize=0;
	unsigned long	paddedsize=0;

	unsigned long	remainder=0;

	unsigned long	numbytesforcrlfs=0;
	struct			_stat st={0};
	FILE*			fp;
	TOKEN			token={0};

	if(!pFileName || !ppEncodedData)
		return;

	_stat(pFileName, &st);
	fp = fopen(pFileName, "rb");

	if(!fp)
		return;

	//total length must be a multiple of 3 bytes (24 bits)

	remainder	= (st.st_size % 3);
	padding		= remainder ? 3 - remainder : 0;
	paddedsize	= st.st_size + padding;
	//in encoded data, an 8 bit character corresponds to 6 bits in original data
	encodedsize	= paddedsize * 8 / 6;
	//after every 76 characters, add 2 bytes for "/r/n"
	numbytesforcrlfs = 2*((encodedsize/76) + (encodedsize%76 > 0 ? 1:0));

	encodedsize += numbytesforcrlfs;

	pFileIn = (unsigned char*) calloc(paddedsize, 1);
	pFileOut= (unsigned char*) calloc(encodedsize, 1);
	fread(pFileIn, st.st_size, 1, fp);
	memset(pFileOut+encodedsize-padding, '=', padding);

//the first bit in the stream will be the high-order bit in the first byte,
//and the eighth bit will be the low-order bit in the first byte, and so on.
//The output stream (encoded bytes)  must  be  represented  in lines  of  no 
//more than 76 characters each
	
	for(i=j=0; i < paddedsize; i+=3, j+=4)
	{
		//get the first 3 bytes
		token.byte.byte3 = pFileIn[i];
		token.byte.byte2 = pFileIn[i+1];
		token.byte.byte1 = pFileIn[i+2];

		//map to 4 characters each 6 bit
		pFileOut[j]		= token.bitfield.field4;
		pFileOut[j+1]	= token.bitfield.field3;
		pFileOut[j+2]	= token.bitfield.field2;
		pFileOut[j+3]	= token.bitfield.field1;

		//use the table to encode values
		pFileOut[j]		= m_key[ pFileOut[j] ];
		pFileOut[j+1]	= m_key[ pFileOut[j+1] ];
		pFileOut[j+2]	= m_key[ pFileOut[j+2] ];
		pFileOut[j+3]	= m_key[ pFileOut[j+3] ];

		//if you have written 76 bytes, then put a "\r\n"
		if(i%19==0)// 76/4=19 (Each time we have written 76 bytes)
		{//write carriage return
			pFileOut[j+4] = '\r';
			pFileOut[j+5] = '\n';
			j+=2;
		}
	}

	free(pFileIn);
	fclose(fp);

	if(pSize) *pSize = encodedsize;
	*ppEncodedData = pFileOut;
}

bool SMTP::WriteEncodedData(char** ppmesg, long* plen, 
							 unsigned char* pEncodedData, unsigned long EncodedDataSize)
{
	if(!plen || !ppmesg)
		return false;
	
	long TotalLen = (*plen)+EncodedDataSize;
	*ppmesg = (char*) realloc(*ppmesg, TotalLen+1024);//1024 extra bytes
	
	memcpy((*ppmesg)+(*plen), pEncodedData, EncodedDataSize);
	*plen = (*plen)+EncodedDataSize;
	
	return true;
}

bool SMTP::AddAttachment(char** ppmesg, long* plen, char* filename)
{
	if(!filename || *filename==0 || !ppmesg || !plen || *plen==0)
	{
		return false;
	}
	else
	{
		unsigned char* pEncodedData=0;
		unsigned long EncodedDataSize=0;

		sprintf((*ppmesg)+(*plen),
			"--&perimeter&\r\n"
				"Content-Type: application/octet-stream;\r\n"
				"Content-Transfer-Encoding: base64\r\n"
				"Content-Disposition: attachment;\r\n"
				"\tfilename=\"%s\"\r\n"
				"\r\n",
				filename);

		(*plen) += lstrlenA((*ppmesg)+(*plen));
		Base64Encode(filename, &pEncodedData, &EncodedDataSize);
		WriteEncodedData(ppmesg, plen, pEncodedData, EncodedDataSize);
		sprintf((*ppmesg)+(*plen), "\r\n");
		(*plen) += 2;

		free(pEncodedData);
		return true;
	}
}

void SMTP::PrepareMail(
			char** ppmesg, long *plen,
         const char *szTo,
			const char* szSubject, const char* szMesssage,
			char szFilePathName[][1024], unsigned long nNumAttachments)

{
	unsigned long i;

	sprintf((*ppmesg),
		"to:%s\r\n"
      "Subject:%s\r\n"
		"MIME-Version: 1.0\r\n"
		"Content-Type: multipart/mixed; boundary=\"&perimeter&\"\r\n"
		"\r\n"
		"--&perimeter&\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Disposition: inline\r\n"
			"\r\n"
			"%s\r\n"
			"\r\n",
		szTo, szSubject, szMesssage);

	(*plen) = lstrlenA((*ppmesg));

	for(i=0; i<nNumAttachments; i++)
	{
		//---{ copy the file data to end of (*ppmesg) array
		AddAttachment(ppmesg, plen, szFilePathName[i]);
		//---} file data copied
	}

	sprintf((*ppmesg)+(*plen),
		"--&perimeter&--\r\n"
		"\r\n"
		"\r\n.\r\n");

	(*plen) += lstrlenA((*ppmesg)+(*plen));
}

unsigned long
SMTP::SendMail(
			STRING &host,
         short port,
			STRING &from,
			STRING recepient[], unsigned long num_recepients,
			long  recepient_status[],//0=good,1=bad address,-1=timedout
			STRING &subject, STRING &body,
			char szFilePathName[][1024], unsigned long nNumAttachments,
			STRING &serr)
{
	unsigned long FailCnt;
	long len=0;
	long result=0;
	unsigned long i;
	unsigned long bRet=false;
   unsigned long replySize = 1024;
	char* reply = (char*) malloc(replySize);
	char* mesg = (char*) malloc(10240);
	//char* reply = 0;
	//CSocket sock;
   SOCKET so;
   int err = 0;
   int bytes_read=0;
   char block = 1;

   STRING msg, recepients;

	if(!winsock_init())
	{
		serr.sprintf(L"WSAStartup failed. Err: %i", GetLastError());
		goto Exit;
	}

   so = socket_client_init();

	if(!so)
	{
      serr.sprintf(L"Bind failed. Err: %i", GetLastError());
		goto Exit;
	}

   socket_blocking(so);

   if(!socket_client_connect(so, host.ansi(), port, &err))
   {
		serr.sprintf(L"Failed to connect with SMTP Server. Err: %i", GetLastError());
		goto Exit;
	}

	msg.sprintf(L"HELO Oosman\r\n");
   socket_write(so, (const unsigned char*)msg.ansi(), msg.size(), block);

   bytes_read = socket_read(so, (unsigned char*)reply, replySize, block);
	if(bytes_read<=0 || !strstr(reply, "220"))
	{
		serr.sprintf(L"No response from SMTP server. Err: %i", GetLastError());
		goto Exit;
	}

	if(bytes_read>0 && !strstr(reply, "250"))
	{
		bytes_read = socket_read(so, (unsigned char*)reply, replySize, block);
		if(bytes_read<=0 || !strstr(reply, "250"))
		{
			serr.sprintf(L"SMTP server did not acknowledge 'HELO' message. Err: %i", GetLastError());
			goto Exit;
		}
	}

	msg.sprintf(L"mail from:%s\r\n", from.s());
   socket_write(so, (const unsigned char*)msg.ansi(), msg.size(), block);

	bytes_read = socket_read(so, (unsigned char*)reply, replySize, block);
	if(bytes_read<=0 || !strstr(reply, "250"))
	{
		serr.sprintf(L"SMTP server did not recognize sender: ");
		serr.append(from.s());
		goto Exit;
	}

	FailCnt = 0;
	for(i=0; i<num_recepients; i++)
	{
		msg.sprintf(L"rcpt to:%s\r\n", recepient[i].s());
      socket_write(so, (const unsigned char*)msg.ansi(), msg.size(), block);

		bytes_read = socket_read(so, (unsigned char*)reply, replySize, block);
		if(bytes_read<=0 || !strstr(reply, "250"))
		{
         if(recepient_status) 
            recepient_status[i] = result==RCPT_TIMEOUT ? RCPT_TIMEOUT : RCPT_BAD;
         FailCnt++;
		}
		else
		{
         if(recepient_status) 
         {
            recepient_status[i] = RCPT_OK;
            recepients.append(recepient[i].s());
            recepients.append(L";");
         }
		}
	}
	
	if(FailCnt)
	{
		if(FailCnt>1 && FailCnt == num_recepients)
			serr.sprintf(L"SMTP server did not recognize all email recipients");
		else if(FailCnt>1)
			serr.sprintf(L"SMTP server did not recognize some email recipients");
		else
			serr.sprintf(L"SMTP server did not recognize email recipient");

		if(recepient_status)
		{
			for(i=0; i<num_recepients; i++)
			{
				if(recepient_status[i]!=RCPT_OK)
				{
					serr.append(L"; ");
					serr.append(recepient[i].s());
				}
			}
		}
		
		if(FailCnt == num_recepients)
		{
			goto Exit;
		}
	}

	msg.sprintf(L"data\r\n");
   socket_write(so, (const unsigned char*)msg.ansi(), msg.size(), block);

	bytes_read = socket_read(so, (unsigned char*)reply, replySize, block);
	if(bytes_read<=0 || !strstr(reply, "354"))
	{
		serr.sprintf(L"SMTP server was not ready to accept data or timed out. Err: %i", GetLastError());
		goto Exit;
	}

	PrepareMail(&mesg, &len, recepients.ansi(), subject.ansi(), body.ansi(), szFilePathName, nNumAttachments);
	socket_write(so, (const unsigned char*)mesg, len, block);
	
   bytes_read = socket_read(so, (unsigned char*)reply, replySize, block);
	if(bytes_read<=0 || !strstr(reply, "250"))
	{
		serr.sprintf(L"MIME data sent but never acknowledged. Err: %i", GetLastError());
		goto Exit;
	}
	
	msg.sprintf(L"quit\r\n");
   socket_write(so, (const unsigned char*)msg.ansi(), msg.size(), block);

	bytes_read = socket_read(so, (unsigned char*)reply, replySize, block);
	if(bytes_read<=0 || !strstr(reply, ""))
	{
		serr.sprintf(L"'quit' command never acknowledged. Err: %i", GetLastError());
		goto Exit;
	}

	bRet=true;

Exit:
	winsock_end();

	if(mesg) 
      free(mesg);
   if(reply)
      free(reply);

	return bRet;
}

unsigned long SMTP::SendMail(
   STRING &host, 
   short port, 
   STRING &from, 
   STRING &to, 
   STRING &subject, 
   STRING &body, 
   STRING &err)
{

   STRING recepient[1];
   recepient[0].set(to.s());
   long  recepient_status[1];

   return SendMail(
      host,
      port,
      from,
      recepient, 1,
      recepient_status,//0=good,1=bad address,-1=timedout
      subject, body,
      0, 0,
      err);

}