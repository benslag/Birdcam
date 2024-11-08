//
// urlencode.h -- URL encode or decode a string
//
#ifndef _URLENCODE_H
#define _URLENCODE_H

#include <Arduino.h>

class URLencode
{
public:
   static void encode(String &toEncode);
   static void decode(String &toDecode);

private:
   static char bin2hex(char c);
   static char hex2bin(char c);
   static void encodeC(const char c, String &output);
};
#endif