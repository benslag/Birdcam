#include "urlencode.h"

//----------------------
void URLencode::encode (String &toEncode)
{
   String output;
   const char *cp = toEncode.c_str ();

   char c;
   while ((c = *cp++) ) {
      encodeC (c, output);
   } 
   toEncode = output;
}

//----------------------
void URLencode::decode (String &toDecode)
// decode a string
//
{
    String output;
    char const *inPtr = toDecode.c_str ();
    char c;
    while ((c = *inPtr++))
    {
       if (c == '%')
       {
          char c1 = *inPtr++;
          char c2 = *inPtr++;
          output += char (hex2bin(c1) << 4 | hex2bin(c2));
       }
       else if (c == '+')
          output += ' ';
       else
          output += c;
    }
    toDecode = output;
}

char URLencode::bin2hex(char cc)
{
   char c = cc & 0xF;
   if (c <= 9)
      c += '0';
   else
      c += 'A' - 10;
   return c;
}

char URLencode::hex2bin(char c)
{
   if (c >= '0' && c <= '9')
      c -= '0';
   else if (c >= 'A' && c <= 'F')
      c -= ('A' - 10);
   else if (c >= 'a' && c <= 'f')
      c -= ('a' - 10);
   c = c & 0xF;
   return c;
}

//----------------------
void URLencode::encodeC(const char c, String &output)
{
   if ((c >= '1' && c <= '9') ||
       (c >= 'A' && c <= 'Z') ||
       (c >= 'a' && c <= 'z'))
   {
      output += c;
   }
   else if (c == ' ')
   {
      output += '+';
   }
   else
   {
      output += '%';
      output += bin2hex(c >> 4);
      output += bin2hex(c);
   }
}
