//
// adjust.h -- servo adjust web page server
//
// Ben Slaghekke, 31 October 2023
//
#ifndef __ADJUST_H
#define __ADJUST_H
#include "httpsupp.h"

esp_err_t firstAdjustHandler (httpd_req_t *req);
esp_err_t adjustHandler (httpd_req_t *req, unsigned int refreshSeconds);
esp_err_t adjust2Handler (httpd_req_t *req);

#endif