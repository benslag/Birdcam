// Birdcam.cpp
//
//
// Derived from:
//  ======
//  Rui Santos
//  Complete instructions at https://RandomNerdTutorials.com/esp32-cam-projects-ebook/
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  =====
// Ben Slaghekke, 28 aug 2023 - split up into several modules
// Ben Slaghekke, 11 oct 2024 - moved to VSCode


#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"

#include "camera.h"
#include "shutter.h"
#include "myWifi.h"
#include "http.h"
#include "credentials.h"

#define  _DEBUG 0
#include "debug.h"


void waitForWakeup ()
{
#ifdef _DEBUG    // only if we are debugging  
   uint32_t startedAt = millis ();
   bool msgDone = false;
   int32_t d;
   while ((d = millis () - startedAt) < STARTUP_SECONDS * 1000) {
      if (d % 1000 == 0) {
         if (!msgDone) {
            int s = STARTUP_SECONDS-(d/1000);
            Serial.printf ("Starting in %2d seconds\n", s);
            msgDone = true;
         }
      }
      else msgDone = false;
      if (Serial.available ()) {
         while (Serial.available ()) {
            String s = Serial.readString ();
            s = s;
         }
         break; // cancel start time
      }
   }
#endif
}

//--------------------
void setup() 
{
   Serial.begin (115200);
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
   waitForWakeup ();
   shutter.setup ();
   camera. setup ();
   myWifi. setup ();
   httpSetup ();
   DEBUG ("setup done\n");
}

//-----------
void loop() 
{
   shutter.loop ();
}
