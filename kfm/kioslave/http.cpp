#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <unistd.h>
#include "kio_errors.h"
#include <config-kfm.h>

/************************** Authorization stuff: copied from wget-source *****/

/* Encode a zero-terminated string in base64.  Returns the malloc-ed
   encoded line.  This is useful for HTTP only.

   Note that the string may not contain NUL characters.  */
char *
base64_encode_line(const char *s)
{
   /* Conversion table. */
   static char tbl[64] = {
      'A','B','C','D','E','F','G','H',
      'I','J','K','L','M','N','O','P',
      'Q','R','S','T','U','V','W','X',
      'Y','Z','a','b','c','d','e','f',
      'g','h','i','j','k','l','m','n',
      'o','p','q','r','s','t','u','v',
      'w','x','y','z','0','1','2','3',
      '4','5','6','7','8','9','+','/'
   };
   int len, i;
   char *res;
   unsigned char *p;

   len = strlen(s);
    res = (char *)malloc(4 * ((len + 2) / 3) + 1);
   p = (unsigned char *)res;
   /* Transform the 3x8 bits to 4x6 bits, as required by
      base64.  */
   for (i = 0; i < len; i += 3)
   {
      *p++ = tbl[s[0] >> 2];
      *p++ = tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
      *p++ = tbl[((s[1] & 0xf) << 2) + (s[2] >> 6)];
      *p++ = tbl[s[2] & 0x3f];
      s += 3;
   }
   /* Pad the result if necessary... */
   if (i == len + 1)
      *(p - 1) = '=';
   else if (i == len + 2)
      *(p - 1) = *(p - 2) = '=';
   /* ...and zero-teminate it.  */
   *p = '\0';
   return res;
}

char *create_www_auth(const char *user, const char *passwd)
{
   char *wwwauth;

   if (user && passwd)
   {
      char *t1, *t2;

      t1 = (char *)malloc(strlen(user) + 1 + 2 * strlen(passwd) + 1);
      sprintf(t1, "%s:%s", user, passwd);
      t2 = base64_encode_line(t1);
      free(t1);
      wwwauth = (char *)malloc(strlen(t2) + 24);
      sprintf(wwwauth, "Authorization: Basic %s\r\n", t2);
      free(t2);
   }
   else
      wwwauth = NULL;
   return(wwwauth);
}

/*****************************************************************************/

KProtocolHTTP::KProtocolHTTP()
{
#ifdef HTTP_PROXY_NAME
	init_sockaddr(&proxy_name, HTTP_PROXY_NAME, HTTP_PROXY_PORT);
	use_proxy = 1;
#else
	use_proxy = 0;
#endif
	connected = 0;
}

KProtocolHTTP::~KProtocolHTTP()
{
	Close();
}

long KProtocolHTTP::Size()
{
	return(size);
}

int KProtocolHTTP::Close()
{
	if(connected) close(sock);
	connected = 0;
	return SUCCESS;
}

int KProtocolHTTP::atEOF()
{
	return(bytesleft == 0 || feof(fsocket));
}

int KProtocolHTTP::Read(void *buffer, int len)
{
	if(atEOF()) return(0);

	long nbytes = fread(buffer,1,len,fsocket);

	/*printf("got nbytes: %d\n",nbytes);*/
	bytesleft -= nbytes;
	if(nbytes >= 0) return(nbytes);
	Error(KIO_ERROR_CouldNotRead,"Reading from socket failed", errno);
	return(FAIL);
}

int KProtocolHTTP::init_sockaddr(struct sockaddr_in *server_name, char *hostname, int port)
{
	struct hostent *hostinfo;
	server_name->sin_family = AF_INET;
	server_name->sin_port = htons( port );

	hostinfo = gethostbyname( hostname );

	if ( hostinfo == 0L ) return(FAIL);
	server_name->sin_addr = *(struct in_addr*) hostinfo->h_addr;
	return(SUCCESS);
}

int KProtocolHTTP::Open(KURL *url, int )
{
	if (connected) Close();

	sock = ::socket(PF_INET,SOCK_STREAM,0);

	if (sock < 0)
	{
	    Error(KIO_ERROR_CouldNotCreateSocket,"Could not create socket",errno);
	    return(FAIL);
	}

	if(use_proxy)
	{
		debugT("connecting to proxy...\n");
		if(::connect(sock,(struct sockaddr*)(&proxy_name),sizeof(proxy_name)))
		{
	    	Error(KIO_ERROR_CouldNotConnect,"Could not connect to proxy",errno);
	    	return(FAIL);
		}
	}
	else
	{
		struct sockaddr_in server_name;
		int port = url->port();
		if (! port )  // use default one
			port = 80;

		if(init_sockaddr(&server_name, url->host(), port) == FAIL)
		{
    		Error(KIO_ERROR_UnknownHost, "Unknown host", errno );
			return(FAIL);
		}
		debugT("connecting to host %s\n",url->host());
		if(::connect(sock,(struct sockaddr*)(&server_name),sizeof(server_name)))
		{
	    	Error(KIO_ERROR_CouldNotConnect, "Could not connect host", errno);
			return(FAIL);
		}
 	}
	connected = 1;

	fsocket = fdopen(sock,"r+");
	if(!fsocket)
	{
	    Error(KIO_ERROR_CouldNotConnect, "Could not fdopen socket", errno);
	    return(FAIL);
	}

	QString command = "GET ";

	if(use_proxy)
	{
		/** add hostname when using proxy **/
		command += "http://";
		command += url->host();
	}

	if ( url->path()[0] != '/' ) command += "/";
	command += url->path();
	command += " HTTP/1.0\n"; /* start header */
	if(strlen(url->user()) != 0)
	{
		char *www_auth = create_www_auth(url->user(),url->passwd());
		command += www_auth;
		free(www_auth);
	}
	command += "\n";  /* end header */

	debugT("opening ");
	write(0, command.data(), command.length());
	write(sock, command.data(), command.length());

	return(ProcessHeader());
}

int KProtocolHTTP::ProcessHeader()
{
	char buffer[1024];
	int len = 1;

	size = 0xFFFFFFF;

	debugT("Processing header:\n");
	while(len && fgets(buffer,1024,fsocket))
	{
		len=strlen(buffer);
		while(len && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
			buffer[--len] = 0;

		debugT("%s\n",buffer);
	    if ( strncmp( buffer, "Content-length: ", 16 ) == 0 
	    ||   strncmp( buffer, "Content-Length: ", 16 ) == 0 )
		size = atol( buffer + 16 );
	}
	bytesleft = size;
	debugT("---< end header >---\n");
	return(SUCCESS);
}
