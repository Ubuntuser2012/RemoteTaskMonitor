#ifndef SMTP_H
#define SMTP_H


#define RCPT_OK  1//recpt was ok
#define RCPT_TIMEOUT -1 //timed out on one of the response
#define RCPT_BAD 0 //not a valid email for the smtp server

//this program will send an email message to a smtp server
//the server address is in szSMTPhost, can be a dot format name or a domain name
//change the recepient name by modifying the string beginning "rcpt to:

typedef struct TOKEN
{
	union
	{
		struct 
		{
			unsigned long field1 : 6;//low order field
			unsigned long field2 : 6;
			unsigned long field3 : 6;
			unsigned long field4 : 6;
			unsigned long blank	: 8;//high order field
		}
		bitfield;

		struct
		{
			unsigned long byte1 : 8;//low order byte
			unsigned long byte2 : 8;
			unsigned long byte3 : 8;
			unsigned long blank : 8;//high order byte
		}
		byte;

		long dword;
	};
}
TOKEN;

class STRING;

class SMTP
{
private:
	static char m_key[];
	void Base64Encode(char* pFileName, unsigned char** ppEncodedData, unsigned long *pSize);
	bool WriteEncodedData(char** ppmesg, long* plen, unsigned char* pEncodedData, unsigned long EncodedDataSize);
	bool AddAttachment(char** ppmesg, long* plen, char* filename);
	void PrepareMail(
			char** ppmesg, long *plen,
         const char* szTo,
			const char* szSubject, const char* szMesssage,
			char szFilePathName[][1024], unsigned long nNumAttachments);


public:
   unsigned long SendMail(STRING &host, short port, STRING &from, STRING &to, STRING &subject, STRING &body, STRING &err);
   unsigned long SendMail(
      STRING &host,
      short port,
      STRING &from,
      STRING recepient[], unsigned long num_recepients,
      long  recepient_status[],//0=good,1=bad address,-1=timedout
      STRING &subject, STRING &body,
      char szFilePathName[][1024], unsigned long nNumAttachments,
      STRING &serr);
};

#endif
