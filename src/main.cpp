// main.cpp (Birdcam.ino)
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
// Ben Slaghekke,  8 nov 2024 - moved to VSCode

#include "soc/soc.h" // disable brownout problems
#include "soc/rtc_cntl_reg.h"

#include "camera.h"
#include "shutter.h"
#include "myWifi.h"
#include "http.h"
#include "timer.h"
#include "credentials.h"

#define _DEBUG 1
#include "debug.h"

static uint32_t startTime;
static uint32_t previousTime;
static void loopTime();

static void myDebugPrinter(const char *buffer)
{
   Serial.print(buffer);
}

//--------------------
void setup()
{
   Serial.begin(115200);

   Serial.print("Serial.begin done\n");
   DEBUGAddPrintFunction(myDebugPrinter);

   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

   LOG("This is main.setup\n");

   shutter.setup();
   camera.setup();
   myWifi.setup();
   httpSetup();

   startTime = millis();
   previousTime = micros();
   LOG("setup done\n");
}

//-----------
void loop()
{
   loopTime();
   shutter.loop();
}

static void loopTime()
{
#if _DEBUG == 1
   static int uptime = 0;
   static int loopCount = 0;
   static uint32_t maximumTime = 0;
   static Timer reportTimer(1 MINUTE);

   uint32_t now = micros();
   uint32_t thisLoopTime = now - previousTime;
   previousTime = now;
   if (thisLoopTime > maximumTime)
   {
      maximumTime = thisLoopTime;
   }

   if (reportTimer.hasExpired())
   {
      reportTimer.restart();

      uint32_t m = millis();
      uint32_t expired = m - startTime;

      float usPerLoop = float(expired) * 1000 / loopCount;
      LOG("main loop: uptime = %d minutes; avg loop time = %5.1f us, max loop time = %d us\n",
          uptime, usPerLoop, maximumTime);
      loopCount = 0;
      maximumTime = 0;
      uptime++;
      startTime = m;
   }
   loopCount++;
#endif
}
