//
// camera.cpp -- camera handling functions
// Birdcam
// Derived from:
//  ======
//  Rui Santos
//  Complete instructions at https://RandomNerdTutorials.com/esp32-cam-projects-ebook/
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  =====
// Ben Slaghekke, 3 Jul 2023

#include <Arduino.h>

#define _DEBUG 0
#include "debug.h"

#include "camera.h"

Camera camera;

#define CAMERA_MODEL_AI_THINKER
// #define CAMERA_MODEL_M5STACK_PSRAM
// #define CAMERA_MODEL_M5STACK_WITHOUT_PSRAM
// #define CAMERA_MODEL_M5STACK_PSRAM_B
// #define CAMERA_MODEL_WROVER_KIT

#if defined(CAMERA_MODEL_WROVER_KIT)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM 15
#define XCLK_GPIO_NUM 27
#define SIOD_GPIO_NUM 25
#define SIOC_GPIO_NUM 23

#define Y9_GPIO_NUM 19
#define Y8_GPIO_NUM 36
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 39
#define Y5_GPIO_NUM 5
#define Y4_GPIO_NUM 34
#define Y3_GPIO_NUM 35
#define Y2_GPIO_NUM 32
#define VSYNC_GPIO_NUM 22
#define HREF_GPIO_NUM 26
#define PCLK_GPIO_NUM 21

#elif defined(CAMERA_MODEL_M5STACK_WITHOUT_PSRAM)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM 15
#define XCLK_GPIO_NUM 27
#define SIOD_GPIO_NUM 25
#define SIOC_GPIO_NUM 23

#define Y9_GPIO_NUM 19
#define Y8_GPIO_NUM 36
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 39
#define Y5_GPIO_NUM 5
#define Y4_GPIO_NUM 34
#define Y3_GPIO_NUM 35
#define Y2_GPIO_NUM 17
#define VSYNC_GPIO_NUM 22
#define HREF_GPIO_NUM 26
#define PCLK_GPIO_NUM 21

#elif defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#elif defined(CAMERA_MODEL_M5STACK_PSRAM_B)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM 15
#define XCLK_GPIO_NUM 27
#define SIOD_GPIO_NUM 22
#define SIOC_GPIO_NUM 23

#define Y9_GPIO_NUM 19
#define Y8_GPIO_NUM 36
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 39
#define Y5_GPIO_NUM 5
#define Y4_GPIO_NUM 34
#define Y3_GPIO_NUM 35
#define Y2_GPIO_NUM 32
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 26
#define PCLK_GPIO_NUM 21

#else
#error "Camera model not selected"
#endif

//-------------------
void Camera::setup()
{
   LOG(">  Camera::setup()\n");
   camera_config_t config;
   config.ledc_channel = LEDC_CHANNEL_0;
   config.ledc_timer = LEDC_TIMER_0;
   config.pin_d0 = Y2_GPIO_NUM;
   config.pin_d1 = Y3_GPIO_NUM;
   config.pin_d2 = Y4_GPIO_NUM;
   config.pin_d3 = Y5_GPIO_NUM;
   config.pin_d4 = Y6_GPIO_NUM;
   config.pin_d5 = Y7_GPIO_NUM;
   config.pin_d6 = Y8_GPIO_NUM;
   config.pin_d7 = Y9_GPIO_NUM;
   config.pin_xclk = XCLK_GPIO_NUM;
   config.pin_pclk = PCLK_GPIO_NUM;
   config.pin_vsync = VSYNC_GPIO_NUM;
   config.pin_href = HREF_GPIO_NUM;
   config.pin_sccb_sda = SIOD_GPIO_NUM;
   config.pin_sccb_scl = SIOC_GPIO_NUM;
   config.pin_pwdn = PWDN_GPIO_NUM;
   config.pin_reset = RESET_GPIO_NUM;
   config.xclk_freq_hz = 20000000;
   config.pixel_format = PIXFORMAT_JPEG;

   if (psramFound())
   {
      config.frame_size = FRAMESIZE_VGA;
      config.jpeg_quality = 10;
      config.fb_count = 2;
   }
   else
   {
      config.frame_size = FRAMESIZE_SVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
   }

   // Camera init
   esp_err_t err = esp_camera_init(&config);
   if (err != ESP_OK)
   {
      ERROR("**** Camera init failed with error 0x%x", err);
   }
   else
   {
      // set camera effects
      sensor_t *s = esp_camera_sensor_get();
      s->set_special_effect(s, 2); // 2 = effect black and white
   }
   LOG("<  Camera::setup\n");
}

//-------------------------
esp_err_t Camera::capture(uint8_t **jpgBuffer, size_t &jpgBufferLen)
// OUT: jpgBuffer:    pointer to jpg frame buffer. MUST BE RELEASED AFTER USE
//      jpgBufferLen: length of jpg buffer
{
   esp_err_t res = ESP_OK;
   frameBuf = esp_camera_fb_get();
   if (!frameBuf)
   {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
   }
   else
   {
      if (frameBuf->width > 400)
      {
         if (frameBuf->format != PIXFORMAT_JPEG)
         {
            // convert to jpg
            bool jpeg_converted = frame2jpg(frameBuf, 80, &jpgBuf, &jpgBufLen);
            esp_camera_fb_return(frameBuf); // no longer needed; we now have the jpg buffer
            frameBuf = nullptr;
            if (!jpeg_converted)
            {
               Serial.println("JPEG compression failed");
               res = ESP_FAIL;
            }
         }
         else
         {
            jpgBufLen = frameBuf->len;
            jpgBuf = frameBuf->buf;
         }
      }
   }
   *jpgBuffer = jpgBuf;
   jpgBufferLen = jpgBufLen;
   return res;
}

//--------------------------------
void Camera::releaseFrameBuffer()
// to be called after sending frame buffer
{
   if (frameBuf)
   {
      esp_camera_fb_return(frameBuf);
      frameBuf = nullptr;
      jpgBuf = nullptr; // jpgBuf was part of frameBuf
   }
   else if (jpgBuf)
   {
      free(jpgBuf);
      jpgBuf = nullptr;
   }
}

//---------------------
void Camera::setVerticalFlip(bool flip)
{
   sensor_t *s = esp_camera_sensor_get();
   s->set_vflip(s, flip ? 1 : 0); // 0 = disable , 1 = enable
}

//-------------------------
void Camera::setHorizontalMirror(bool mirror)
{
   sensor_t *s = esp_camera_sensor_get();
   s->set_hmirror(s, mirror ? 1 : 0); // 0 = disable , 1 = enable
}
