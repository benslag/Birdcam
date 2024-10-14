//
// debug.h - a simple debugger
//
// This debugger uses Serial.printf to debug its output
// Note that if _Debug is 0, the arguments of DEBUG are NOT evaluated.
//   e.g. DEBUG ("The value of i = %d\n", i++);
//   If _DEBUG is defined as 0, i is NOT increased.
//
// Macro toCCP (x) can be used to print the value of a boolean.
//   e.g. DEBUG ("Boolean x = %s\n", toCCP (x));
//   prints either "Boolean x = true" or "Boolean x = false"
//   (note: CCP is const char ptr)
// 
// BSla, 10 october 2023
//
#ifndef __DEBUG_H
#define __DEBUG_H

#ifndef _DEBUG
#error Debug.h: _DEBUG is not defined (define as 0 or 1)
#endif

extern const char* toCCP (bool b);

#if _DEBUG == 1
#define DEBUG(...) DebugReal (__VA_ARGS__)
extern void DebugReal (const char *fmt, ...);
#elif _DEBUG == 0
#define DEBUG(...) {}
#else
#error Debug.h: _DEBUG is not defined as 0 or 1
#endif  // if _DEBUG==1

#endif  // __DEBUG_H
