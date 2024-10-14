//
// XString.h -- extended String capabilities
//
// BSla, 13 oct 2023
//
#ifndef _XSTRING_H
#define _XSTRING_H

#include <Arduino.h>
#include <IPAddress.h>

class XString: public String {
 public:
    XString ():String () {}
    XString (const String &s): String (s) {}
    XString (const char *s):   String (s) {}
    XString (const IPAddress ip);                   // form 192.168.23.197
    XString URLencode ();                                      // URL-encode a string
    XString URLdecode ();                                      // URL-decode a string
 private:
    static char bin2hex (char c);
    static char hex2bin (char c);
	  static char decodeC (char cL, char cR) {return ((hex2bin (cL) << 4) | hex2bin (cR));}
    void        encodeC (const char c, XString &output);
};

#endif

