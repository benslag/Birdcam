//
// httpsupp.cpp -- http support functions
//
// Ben Slaghekke, 31 October 2023
//

#include <Arduino.h>


#include "XString.h"
#include "html.h"
#include <Preferences.h>

#include "httpsupp.h"
#define _DEBUG 1
#include "debug.h"

extern Preferences preferences;

esp_err_t sendPage (httpd_req_t *req, const char *body, unsigned int refreshSeconds)
{
   DEBUG ("sendPage: refreshSeconds = %d\n", refreshSeconds);
    String refreshS;
    if (refreshSeconds > 0) {
       refreshS += String ("<meta http-equiv=\"refresh\" content=\"") + String (refreshSeconds) + String ("\">");
    }
    DEBUG ("refresh clause = %s\n", refreshS.c_str ());

    String head (theHead);
    head.replace ("$REFRESH$", refreshS);  

    String siteName ("<h1>" + getSiteName () + "</h1>");
    String comment ("<br>" + getComment () + "<br>");
    esp_err_t r = httpd_resp_set_type(req, "text/html");
    if (r == ESP_OK) r = httpd_resp_send_chunk (req, head.c_str (),             head.length ());
    if (r == ESP_OK) r = httpd_resp_send_chunk (req, (const char *)styleHead,   strlen(styleHead));
    if (r == ESP_OK) r = httpd_resp_send_chunk (req, (const char *)startBody,   strlen(startBody));
    if (r == ESP_OK) r = httpd_resp_send_chunk (req, siteName.c_str (),         siteName.length());
    if (r == ESP_OK) r = httpd_resp_send_chunk (req, comment.c_str (),          comment.length());
    if (r == ESP_OK) r = httpd_resp_send_chunk (req, (const char *)body,        strlen(body));
    if (r == ESP_OK) r = httpd_resp_send_chunk (req, (const char *)endHtml,     strlen(endHtml));
    if (r == ESP_OK) r = httpd_resp_send_chunk (req, nullptr, 0);
    return r;
}

//------------------------
esp_err_t fetchQuery (httpd_req_t *req, String &query)
// fetch the query string that the client used to access this server
{
//   String S;
   char   *buf = nullptr;
   size_t  bufLen;
   int result = ESP_OK;
  
   bufLen = httpd_req_get_url_query_len (req) + 1;
   if (bufLen <= 1) {
      httpd_resp_send_404 (req); // not found
      result =  ESP_FAIL;
   }
   else {
      // we got a buffer size
      buf = (char*) malloc(bufLen);
      if (!buf) {
         httpd_resp_send_500(req);  // internal server error
         result = ESP_FAIL;
      }
   }

   if (result == ESP_OK) {
      // we have a buffer
      result =  httpd_req_get_url_query_str (req, buf, bufLen);
      DEBUG ("fetchQuery: after get_url_query_str buf = >%s<, len = %d, result = %d\n", buf, bufLen, result);
      if (result != ESP_OK) {  
         httpd_resp_send_404 (req);  // not found
      }       
      else {
         query = XString (buf).URLdecode();
      }
   }

   if (buf) {
      free (buf);
      buf = nullptr;
   }
   return result;
}


//-----------------------------
bool getValue (String kvps, String key,  String &value)
// from a string with <key=value> pairs, fetch a string value for the key
{
   bool found = false;
   value = "";
   int index = kvps.indexOf (key);
   if (index > -1) {
      int indexOfEq = index+key.length();
      if (kvps[indexOfEq] == '=') {
         found = true;
         int endIndex = kvps.indexOf('&', indexOfEq);
         if (endIndex == -1) endIndex = kvps.length ();
         value = kvps.substring (indexOfEq+1, endIndex);
         value.trim ();
      }
   }
   return found;
}

//-----------------------------
bool getValue (String kvps, String key,  int &value)
// from a string with <key=value> pairs, fetch an int value for the key
{
   String s;
   bool found = getValue (kvps, key, s);
   if (found) {
      value = s.toInt ();
   }
   return found;
}



//--------------------------
void registerUriHandler (httpd_handle_t &httpd, const char* uri, esp_err_t (*theHandler) (httpd_req_t *req))
{
   httpd_uri_t theUri = {
      .uri = uri,
      .method = HTTP_GET,
      .handler = theHandler,
      .user_ctx = nullptr,
      .is_websocket = false,
      .handle_ws_control_frames = false,
      .supported_subprotocol = nullptr
   };
   httpd_register_uri_handler(httpd, &theUri);
}

//--------------------------
void setSiteName (String s)
{
   preferences.begin ("Site", false);
   preferences.putString ("Name", s);
   preferences.end ();
}

//--------------------------
String getSiteName ()
{
   preferences.begin ("Site", true); // name, read-only
   String s (preferences.getString ("Name", "*Site naam niet opgegeven*"));
   preferences.end ();

   return s;
}

//--------------------------
void setComment (String s)
{
   DEBUG ("setComment: set to '%s'\n", s.c_str ());
   preferences.begin ("Comment", false);
   preferences.putString ("Name", s);
   preferences.end ();
}

//--------------------------
String getComment ()
{
   preferences.begin ("Comment", true); // name, read-only
   String s (preferences.getString ("Name", ""));
   preferences.end ();
   DEBUG ("getComment: comment = '%s'\n", s.c_str ());
   return s;
}




//----------------
void performReboot (httpd_req_t *req)
{
   sendPage (req, rebootBody, 0 );
   delay (2000);
   ESP.restart ();
}

