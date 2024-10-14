//
// adjust.cpp -- http handling related to shutter adjustment
//
// Ben Slaghekke, 31 October 2023
//

#include <Arduino.h>
#include "shutter.h"
#include "httpsupp.h"
#include "html.h"
#include "http.h"
#include <Preferences.h>
#include "adjust.h"

#define _DEBUG 0
#include "debug.h"


Preferences preferences;

// forwards
static void setShutterValues     (int  op, int  cp, int  sp);
static void getShutterValues     (int &op, int &cp, int &sp);
static esp_err_t handleStartMove (httpd_req_t *req, int op, int cp, int sp, int nMoves);
static esp_err_t handleExit      (httpd_req_t *req, int op, int cp, int sp, String &eV);


//-----------------
esp_err_t firstAdjustHandler (httpd_req_t *req)
// Initial call of adjust handler. This call always closes the shutter first.
// Later calls are directly to adjustHandler and require a refresh time 
{
   DEBUG ("First adjust handler\n");
   shutter.close ();
   shutter.waitComplete ();
   return adjustHandler (req, 0); // no refresh time
}


//----------------
esp_err_t adjustHandler (httpd_req_t *req, unsigned int refreshSeconds)
{
   // in degrees
   int op; int cp; int sp;

   getShutterValues (op, cp, sp);

   DEBUG ("adjustHander: degrees: op = %d, cp = %d, sp = %d\n", op, cp, sp);

   bool _isOpen   = shutter.isOpen   ();
   bool _isClosed = shutter.isClosed ();
   String ps (_isOpen?"De sluiter is nu open":_isClosed?"De sluiter is nu gesloten":"");
   String adj (adjustHtml);
   String totalMoves (shutter.getNShutterMoves ());
   String movesLeft  (shutter.movesLeft ());
   adj.replace ("$SHUTTERPOS$", ps);
   adj.replace ("$OPENPOS$",    String (op));
   adj.replace ("$CLOSEDPOS$",  String (cp));
   adj.replace ("$SPEED$",      String (sp));
   adj.replace ("$TOTALMOVES$", totalMoves);
   adj.replace ("$MOVESLEFT$",  movesLeft);
   DEBUG ("Calling sendPage; totalMoves = %s, movesLeft = %s\n", totalMoves.c_str (), movesLeft.c_str ());
   return sendPage (req, adj.c_str(), refreshSeconds);
}



//--------------------------
esp_err_t adjust2Handler (httpd_req_t *req)
{
   esp_err_t result = ESP_OK;
 
   // expressed in degrees
   int op = 0, cp=0, sp=0, nMoves=0;

   DEBUG ("adjust2Handler called\n");

   String kvps;   // key-value pairs
   result = fetchQuery (req, kvps);
   DEBUG ("After fetch Query, result = %d, kvps = %s\n", result, kvps.c_str());

   if (result == ESP_OK) {
      getValue (kvps, "openpos", op); // in degrees
      getValue (kvps, "clpos",   cp);
      getValue (kvps, "speed",   sp);
      getValue (kvps, "ntimes",  nMoves);
      DEBUG ("adjust2: openpos = %d, clpos = %d, speed = %d, nMoves = %d <deg>\n", op, cp, sp, nMoves);

      String eV;
      if (getValue (kvps, "Exit", eV)) {
         DEBUG ("Found exit value; it is |%s|\n", eV.c_str ());
         if (eV == "Start bewegingen") { 
            result = handleStartMove (req, op, cp, sp, nMoves);
         }
         else result = handleExit (req, op, cp, sp, eV);
      }
      else if (getValue (kvps, "Open", eV)) {
         DEBUG ("button open\n");
         setShutterValues (op, cp, sp);
         shutter.open ();
         shutter.waitComplete ();
         result = adjustHandler (req, 0);
      }
      else if (getValue (kvps, "Sluit", eV)) {
         DEBUG ("button close\n");
         setShutterValues (op, cp, sp);
         shutter.close ();
         shutter.waitComplete ();
         result = adjustHandler (req, 0);
      }
      else {
        result = ESP_FAIL;
        DEBUG ("***** adjust 2: no Open, Sluit or Exit\n");
        DEBUG ("      kvps = %s\n", kvps.c_str ());
      }
   }
   else result = ESP_FAIL;
   
   if (result != ESP_OK) {
      httpd_resp_send_500 (req);
      result = ESP_FAIL;
   }
   return result;
}

//--static functions---------------------------------------

//--------------------------
static void setShutterValues (int op, int cp, int sp)
// set shutter values in degrees (per second)
{
   shutter.setValues (shutter.toUs (op), shutter.toUs (cp), shutter.speedToUs (sp));
}


//--------------------------
static void getShutterValues (int &op, int &cp, int &sp)
// get shutter values in degrees (per second)
{
   shutter.getValues (op, cp, sp);
   op = shutter.toDeg (op);
   cp = shutter.toDeg (cp);
   sp = shutter.speedToDeg (sp);
}

//--------------------------
static esp_err_t handleStartMove (httpd_req_t *req, int op, int cp, int sp, int nMoves)
// open pos, closed pos, speed in deg/sec
{
   esp_err_t result = ESP_OK;
   static bool isRepeatedCall = false;

   DEBUG ("handleStartMove; nMoves = %d, it is a %s call\n", nMoves, isRepeatedCall?"repeated":"original");
   if (!isRepeatedCall) {
      // this is the first reply to the form; not the result of an auto-refresh
      if (nMoves > 0) {  // kickoff repeated moves
         DEBUG ("handleStartMove: start %d moves\n", nMoves);
         setShutterValues (op, cp, sp);
         shutter.startRepeatedMoves (nMoves);
         isRepeatedCall = true;            // expect repeated call back
         result = adjustHandler (req, 1);  // continue repeated move, refresh once per second
      }
      else {
         // Start moves called, but nMoves = 0
         isRepeatedCall = false;
         result = adjustHandler (req, 0);  // no autorepeat; redisplay same screen
      }
   }   
   else {
      // this call is the result of an auto-refresh
      if (shutter.movesLeft () > 0) {
         result = adjustHandler (req, 1); // auto-refresh after 1 second
      }
      else {
         // moves are done
         DEBUG ("Shutter moves are done!\n");
         isRepeatedCall = false;
         result = adjustHandler (req, 0);  // no auto-refresh
      }
   }
   return result;
}

//--------------------------
static esp_err_t handleExit (httpd_req_t *req, int op, int cp, int sp, String &eV)
// open pos, closed pos, speed in deg/sec
{
   esp_err_t result = ESP_OK;
   DEBUG ("handleExit: exit value = %s\n", eV.c_str ());
   if (eV == "OK") {
      setShutterValues (op, cp, sp);
      shutter.saveSettings ();
      shutter.close ();
      shutter.waitComplete ();
      result = index_handler (req);
   }
   else if (eV == "Cancel") {
      shutter.restoreSettings ();
      shutter.close ();
      shutter.waitComplete ();
      result = index_handler (req);
   }
   else {
      DEBUG ("***** handleExit: got unknown exit value %s\n", eV.c_str ());
      result = ESP_FAIL;
   }
   return result;
}
