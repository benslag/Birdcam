//
// XString.cpp
//
// Ben Slaghekke, 13 Oct 2023
//
#include "XString.h"

//----------------------
XString::XString (const IPAddress ip)
{
   uint32_t ipu = ip;
   for (int i = 0; i < 4; i++) {
       *this += (int (ipu & 0xFF));
       ipu = ipu >> 8;
       if (i<3) *this += '.';
   }
}

//----------------------
XString XString::URLencode ()
{
   XString output;
   const char *cp = this->c_str ();

   char c;
   while ((c = *cp++) ) {
      encodeC (c, output);
   } 
   return output;
}

//----------------------
XString XString::URLdecode ()
// decode a string, return size of string
//
{
    XString output;
    char const *inPtr = this -> c_str ();
    char c;
    while ((c=*inPtr++)) {
       if (c == '%') {
          char c1 = *inPtr++;
          char c2 = *inPtr++;
          output += decodeC (c1, c2);
       }
       else if (c == '+') output += ' ';
       else output += c;
	}
  return output;
}

char XString::bin2hex (char cc)
{
   char c = cc & 0xF;
   if (c <= 9) c += '0';
   else c += 'A' - 10;
   return c;
}

//----------------------
void XString::encodeC (const char c, XString &output)
{
	if ((c >= '1' && c <= '9') ||
      (c >= 'A' && c <= 'Z') ||
		  (c >= 'a' && c <= 'z')    ) {
	   output += c;
	}
	else if (c == ' ') {
		 output += '+';
	}
	else {
		output += '%';
    output += bin2hex (c >> 4);
    output += bin2hex (c);
	}
}



char XString::hex2bin (char c)
{
    if (c >= '0'&& c <= '9') c -= '0';
    else if (c >= 'A' && c <= 'F') c -= ('A'-10);
    else if (c >= 'a' && c <= 'f') c -= ('a'-10);
	c = c & 0xF;
	return c;
}



