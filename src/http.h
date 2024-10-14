//
// http.h -- http server functions
//
// Copied in part from Rui Santos' ESP32 camera server
// BSla, 3 jul 2023
//

#ifndef _HTTP_H
#define _HTTP_H

#include "esp_http_server.h"

extern void      httpSetup ();

extern esp_err_t index_handler (httpd_req_t *req);

#endif