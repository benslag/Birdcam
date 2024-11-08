//
// httpsupp.cpp -- http support functions
//
// Ben Slaghekke, 31 October 2023
//

#include <Arduino.h>

#include "html.h"
#include <Preferences.h>
#include "urlencode.h"

#include "httpsupp.h"
#define _DEBUG 1
#include "debug.h"

static Preferences preferences;

static const char *cName = "httpsupp";

esp_err_t sendPage(httpd_req_t *req, const char *body, unsigned int refreshSeconds)
{
   const char *fName = "sendPage";
   LOG(">  %s: %s: refreshSeconds = %d\n", cName, fName, refreshSeconds);
   String refreshS("");
   if (refreshSeconds > 0)
   {
      refreshS += String("<meta http-equiv=\"refresh\" content=\"") + String(refreshSeconds) + String("\">");
      LOG("   %s: %s: refresh clause = %s\n", cName, fName, refreshS.c_str());
   }

   String head(theHead);
   head.replace("$REFRESH$", refreshS);

   String siteName("<h1>" + getSiteName() + "</h1>");
   String comment("<br>" + getComment() + "<br>");
   esp_err_t r = httpd_resp_set_type(req, "text/html");
   if (r == ESP_OK)
      r = httpd_resp_send_chunk(req, head.c_str(), head.length());
   if (r == ESP_OK)
      r = httpd_resp_send_chunk(req, (const char *)styleHead, strlen(styleHead));
   if (r == ESP_OK)
      r = httpd_resp_send_chunk(req, (const char *)startBody, strlen(startBody));
   if (r == ESP_OK)
      r = httpd_resp_send_chunk(req, siteName.c_str(), siteName.length());
   if (r == ESP_OK)
      r = httpd_resp_send_chunk(req, comment.c_str(), comment.length());
   if (r == ESP_OK)
      r = httpd_resp_send_chunk(req, (const char *)body, strlen(body));
   if (r == ESP_OK)
      r = httpd_resp_send_chunk(req, (const char *)endHtml, strlen(endHtml));
   if (r == ESP_OK)
      r = httpd_resp_send_chunk(req, nullptr, 0);
   LOG("<  %s: %s ()", cName, fName);
   return r;
}

//------------------------
esp_err_t fetchQuery(httpd_req_t *req, String &query)
// fetch the query string that the client used to access this server
{
   const char *fName = "fetchQuery";
   LOG(">  %s: %s (...)\n", cName, fName);
   char *buf = nullptr;
   size_t bufLen;
   int result = ESP_OK;

   bufLen = httpd_req_get_url_query_len(req) + 1;
   if (bufLen <= 1)
   {
      httpd_resp_send_404(req); // not found
      result = ESP_FAIL;
   }
   else
   {
      // we got a buffer size
      buf = (char *)malloc(bufLen);
      if (!buf)
      {
         httpd_resp_send_500(req); // internal server error
         result = ESP_FAIL;
      }
   }

   if (result == ESP_OK)
   {
      // we have a buffer
      result = httpd_req_get_url_query_str(req, buf, bufLen);
      LOG("   %s: %s: after get_url_query_str buf = >%s<, len = %d, result = %d\n", cName, fName, buf, bufLen, result);
      if (result != ESP_OK)
      {
         httpd_resp_send_404(req); // not found
      }
      else
      {
         query = buf;
         URLencode::decode(query);
      }
   }

   if (buf)
   {
      free(buf);
      buf = nullptr;
   }
   LOG("<   %s: %s\n", cName, fName);
   return result;
}

//-----------------------------
bool getValue(String kvps, String key, String &value)
// from a string with <key=value> pairs, fetch a string value for the key
{
   const char *fName = "getValue";
   LOG(">  %s: %s (key = %s)\n", cName, fName, key.c_str());
   bool found = false;
   value = "";
   int index = kvps.indexOf(key);
   if (index > -1)
   {
      int indexOfEq = index + key.length();
      if (kvps[indexOfEq] == '=')
      {
         found = true;
         int endIndex = kvps.indexOf('&', indexOfEq);
         if (endIndex == -1)
            endIndex = kvps.length();
         value = kvps.substring(indexOfEq + 1, endIndex);
         value.trim();
      }
   }
   LOG("<  %s: %s: key %s: value = '%s', found = %s\n", cName, fName, key.c_str(), value.c_str(), toCCP(found));
   return found;
}

//-----------------------------
bool getValue(String kvps, String key, int &value)
// from a string with <key=value> pairs, fetch an int value for the key
{
   String s;
   bool found = getValue(kvps, key, s);
   if (found)
   {
      value = s.toInt();
   }
   LOG(">< %s: getValue (int): key %s value = %d\n", cName, key.c_str(), value);
   return found;
}

//--------------------------
void registerUriHandler(httpd_handle_t &httpd, const char *uri, esp_err_t (*theHandler)(httpd_req_t *req))
{
   const char *fName = "registerUriHandler";
   LOG(">< %s: %s\n", cName, fName);
   httpd_uri_t theUri = {
       .uri = uri,
       .method = HTTP_GET,
       .handler = theHandler,
       .user_ctx = nullptr,
       .is_websocket = false,
       .handle_ws_control_frames = false,
       .supported_subprotocol = nullptr};
   httpd_register_uri_handler(httpd, &theUri);
}

//--------------------------
void setSiteName(String s)
{
   const char *fName = "setSiteName";
   LOG(">  %s::%s (name = %s)\n", cName, fName, s.c_str());
   preferences.begin("Site", false);
   preferences.putString("Name", s);
   preferences.end();
   LOG("<  %s::%s ()\n", cName, fName);
}

//--------------------------
String getSiteName()
{
   const char *fName = "getSiteName";
   LOG(">  %s::%s ()\n", cName, fName);
   preferences.begin("Site", true); // name, read-only
   String s(preferences.getString("Name", "*Site naam niet opgegeven*"));
   preferences.end();
   LOG("<  %s::%s = %s\n", cName, fName, s.c_str());
   return s;
}

//--------------------------
void setComment(String s)
{
   const char *fName = "setComment";
   LOG(">  %s::%s (s = '%s')\n", cName, fName, s.c_str());
   preferences.begin("Comment", false);
   preferences.putString("Name", s);
   preferences.end();
   LOG("<  %s::%s ()\n", cName, fName);
}

//--------------------------
String getComment()
{
   const char *fName = "getComment";
   LOG(">  %s::%s ()\n", cName, fName);
   preferences.begin("Comment", true); // name, read-only
   String s(preferences.getString("Name", ""));
   preferences.end();
   LOG("<  %s::%s ()= '%s'\n", cName, fName, s.c_str());
   return s;
}

//----------------
void performReboot(httpd_req_t *req)
{
   const char *fName = "performReboot";
   LOG(">  %s::%s ()\n", cName, fName);
   sendPage(req, rebootBody, 0);
   LOG("------------------------- Rebooting in 2 seconds------------------------------\n");
   delay(2000);
   ESP.restart();
}
