#include <string.h>
#include <iconv.h>

int code_convert(char *inbuf,size_t inlen,char *outbuf, size_t outlen)
{
  iconv_t cd;
  int rc;
  char **pin = &inbuf;
  char **pout = &outbuf;
  char from_charset[] = "gb2312";
  char to_charset[] = "utf-8";
  cd = iconv_open(to_charset,from_charset);
  if (cd==0) return -1;
  memset(outbuf,0,outlen);
  if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
  iconv_close(cd);
  return 0;
}
