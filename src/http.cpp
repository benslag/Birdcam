//
// http.cpp -- http related functions
//
// Copied in part from Rui Santos' ESP32 camera server
// BSla, 3 jul 2023
//
#include <Arduino.h>
#include <Preferences.h>

#include "html.h"
#include "camera.h"
#include "shutter.h"

#include "myWifi.h"
#include "adjust.h"
#include "httpsupp.h"
#include "http.h"

#define _DEBUG 0
#include "debug.h"

#define PART_BOUNDARY "123456789000000000000987654321"
#define BLANK_PASSWORD "******"


static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

//----------------
esp_err_t index_handler (httpd_req_t *req)
{  esp_err_t result;
   if (shutter.isClosed ()) {
      DEBUG ("Index handler: shutter closed; show normal index page\n");
      result = sendPage (req, indexBody, 0);
   }
   else {
      DEBUG ("Index handler: shutter not closed; show warning index page\n");
      result = sendPage (req, shutterOpenBody, 0);
   }
   return result;
}

//----------------
static esp_err_t page2_handler (httpd_req_t *req)
{
   shutter.open(); 
   shutter.waitComplete ();
   return sendPage (req, page2Body, 0);
}

//----------------
static esp_err_t page3_handler (httpd_req_t *req)
{
   shutter.close ();
   shutter.waitComplete ();
   return sendPage (req, page3Body, 0);
}

//----------------
static esp_err_t siteInfoHandler (httpd_req_t *req)
{
   String cbs (siteInfoBody);
   cbs.replace ("$site$", getSiteName ());
   cbs.replace ("$comment", getComment ());
   cbs.replace ("$SSID$", myWifi.mySSID ());
   cbs.replace ("$pass$", BLANK_PASSWORD);
   return sendPage (req, cbs.c_str(), 0);
}


//-------------------
static esp_err_t stream_handler (httpd_req_t *req)
{
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  int frameNo = 0;
  while (true)
  {
      res = camera.capture (&_jpg_buf, _jpg_buf_len);

      if (res == ESP_OK) {
         size_t hlen = snprintf ((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
         res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
      }
      if (res == ESP_OK) {
         res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
      }
      if (res == ESP_OK) {
         res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
      }
      camera.releaseFrameBuffer ();

      if (res != ESP_OK) {
         break;
      }
      if (++frameNo % 100 == 0) {
         DEBUG ("JPG: frame %d length %u bytes\n", frameNo, (uint32_t)(_jpg_buf_len));
      }
   }

   return res;
}


//--------------------------
static esp_err_t siteInfo2Handler (httpd_req_t *req)
{
   bool doReset = false;
   bool success = true;;
   DEBUG ("cmd_handler called\n");
   esp_err_t result;
   String kvps;   // key-value pairs
   result = fetchQuery (req, kvps);
   DEBUG ("After fetch Query, result = %d, kvps = %s\n", result, kvps.c_str());

   if (result == ESP_OK) {
      String eV;
      if (getValue (kvps, String ("Exit"), eV)) {
         if (eV == String ("OK")) {
            String siteName;
            if (getValue (kvps, String ("sitename"), siteName)) {
               setSiteName (siteName);
            }
            String SSID;
            String password;
            if (getValue (kvps, String ("SSID"), SSID)) {
               if (getValue (kvps, String ("Password"), password)) {
                  // we have an ID and a password
                  if (SSID != myWifi.mySSID ()) {
                     myWifi.setSSID (SSID);
                     doReset = true;
                  }
                  if ((password != String (BLANK_PASSWORD)) &&
                       (password != myWifi.myPassword ())      ) {
                     myWifi.setPassword (password);
                     doReset = true;
                  }
               }
               else success = false;
            }
            else success = false;
         }
         else if (eV == String ("Cancel")) {
            // 'Cancel': do nothing
         }
         else success = false;
      }
      else success = false;
   }
   else success = false;
   
   if (!success) {
      httpd_resp_send_500 (req);
      result = ESP_FAIL;
   }

   if (result == ESP_OK) {
      if (doReset) {
         performReboot (req);
      }
      else {
         result = page3_handler (req);
      }
   }
   return result;
}

//----------------------------
void httpSetup ()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    registerUriHandler (camera_httpd, "/",          index_handler);
    registerUriHandler (camera_httpd, "/page2",     page2_handler);
    registerUriHandler (camera_httpd, "/page3",     page3_handler);
    registerUriHandler (camera_httpd, "/siteinfo",  siteInfoHandler);
    registerUriHandler (camera_httpd, "/siteinfo2", siteInfo2Handler);
    registerUriHandler (camera_httpd, "/adjust",    firstAdjustHandler);
    registerUriHandler (camera_httpd, "/adjust2",   adjust2Handler);
  }

  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    registerUriHandler (stream_httpd, "/stream",    stream_handler);
  }
}

