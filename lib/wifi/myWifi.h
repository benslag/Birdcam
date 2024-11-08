//
// mywifi.h -- wifi for birdcam
//
// BSla, 28 aug 2023

#ifndef _MYWIFI_H
#define _MYWIFI_H

#include <WiFi.h>


class MyWifi {
  public:
   MyWifi () {}
   ~MyWifi () {}
   void setup ();
   void loop ();
   String mySSID ();
   String myPassword ();
   void  setSSID (String SSID);
   void  setPassword (String password);
  private:
   String makeSSID ();
   bool setupAsAccessPoint ();
   bool setupAsClient (int nTries);
   void setupServer ();
};

extern MyWifi myWifi;

#endif