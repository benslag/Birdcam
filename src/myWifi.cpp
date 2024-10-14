//
// mywifi.cpp -- wifi for birdcam
//
// BSla, 28 aug 2023
// 07 05 2024 BSla 'fold' the chip ID into 16 bits, instead of using only the lowest 16 

#include <Preferences.h>
#include <DNSServer.h>
#include "myWifi.h"
#include "XString.h"

#define MAX_CONNECTIONS 2

#define  _DEBUG 1
#include "debug.h"

static Preferences preferences;

MyWifi myWifi;

String MyWifi::mySSID () 
{
   preferences.begin ("Site", true); // name, read-only
   String s (preferences.getString ("SSID", ""));
   preferences.end ();
   return s;
}

String MyWifi::myPassword () 
{
   preferences.begin ("Site", true); // name, read-only
   String s (preferences.getString ("Password", ""));
   preferences.end ();
   return s;
}

void MyWifi::setSSID (String s)
{
   preferences.begin ("Site", false);
   preferences.putString ("SSID", s);
   preferences.end ();
}

void MyWifi::setPassword (String s)
{
   preferences.begin ("Site", false);
   preferences.putString ("Password", s);
   preferences.end ();
}

String MyWifi::makeSSID ()
{
   uint64_t chipID = ESP.getEfuseMac ();
   
   uint16_t chip16 = 0;
   // 'fold' the 64 bits ID into 16; ^ = exor
   for (int i = 0; i < 64; i += 16) {
      chip16 = chip16 ^ uint16_t ((chipID >> i) & 0xffff);
   }

   String SSID ("Birdcam-");
   SSID += String (chip16, 16);
   return SSID;
}

bool MyWifi::setupAsAccessPoint ()
{
   WiFi.mode (WIFI_AP);
   WiFi.softAP (makeSSID ().c_str ());
// WiFi.softAP (makeSSID ().c_str (), password,MAX_CONNECTIONS);
   DEBUG ("Wait 100 ms for AP_START...\n");
   delay (100);
  
   DEBUG ("Set softAPConfig\n");
   IPAddress Ip (192,168,1,1);
   IPAddress NMask (255, 255, 255, 0);
// IPAddress gateway (192,168,1,254);
// IPAddress primaryDNS (8,8,8,8);
// IPAddress secundaryDNS (8,8,4,4);
   WiFi.softAPConfig (Ip, Ip, NMask);
   IPAddress myIP = WiFi.softAPIP();
   DEBUG ("Accesspoint IP address = %s\n", XString (myIP).c_str ());
   return true;
}

bool MyWifi::setupAsClient (int nTries)
{
   DEBUG ("Setup as client; %d tries", nTries);
   // connect to Wi-Fi 
   bool connected = false;
   int waitCount = 0;
   String ssid      = mySSID ();
   String password  = myPassword ();
   if (ssid.length () > 0 && password.length () > 0) {
      for (int i = 0; i < nTries; i++) {
         DEBUG ("\nTry %d as client, network %s  ", i+1, ssid.c_str ());
         WiFi.begin (ssid, password);  // from credentials.h
         while (!connected && waitCount++ < 10) {
            connected = (WiFi.status() == WL_CONNECTED);
            if (!connected ) {
               delay (500);
               DEBUG (".");
            }
         }
         if (connected) {
            break; // the for-loop
         }
         else {
            waitCount = 0;
         }
      }
   }
   return connected;
}


void MyWifi::setup ()
{
   DEBUG ("myWifi setup\n");
   bool connected = false;
   // connect to Wi-Fi net
   while (!connected) { 
      connected = setupAsClient (6);
      if (connected) {
         XString x;
         DEBUG ("\nWiFi connected. IP address = %s\n", XString (WiFi.localIP ()).c_str ());
      }
      else {
         connected = setupAsAccessPoint ();
      }
   }
   if (!connected) {
      DEBUG ("\nWifi NOT connected!\n");
   }
}