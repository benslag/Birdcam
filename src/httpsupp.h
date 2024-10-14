//
// httpsupp.h -- http support functions
//
// Ben Slaghekke, 31 October 2023
//
#ifndef _HTTPSUPP_H
#define _HTTPSUPP_H

#include <Arduino.h>
#include "esp_http_server.h"


extern esp_err_t sendPage           (httpd_req_t *req, const char *body, unsigned int refreshSeconds);
extern esp_err_t fetchQuery         (httpd_req_t *req, String &query);
extern bool      getValue           (String kvps, String key,  String &value);
extern bool      getValue           (String kvps, String key,  int &value);
extern void      registerUriHandler (httpd_handle_t &httpd, const char* uri, esp_err_t (*theHandler) (httpd_req_t *req));
extern String    getSiteName        ();
extern void      setSiteName        (String s);
extern String    getComment         ();
extern void      setComment         (String s);
extern void      performReboot      (httpd_req_t *req);
#endif