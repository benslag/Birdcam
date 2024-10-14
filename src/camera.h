//
// camera.h -- camera related items
//
// BSla, 3 Jul 2023

#ifndef _CAMERA_H
#define _CAMERA_H

#include "esp_camera.h"


class Camera {
 public:
   Camera (){}
   ~Camera () {}
   void setup ();
   void loop () {}

   esp_err_t capture (uint8_t **jpgBuffer, size_t &jpgBufferLen);
   void releaseFrameBuffer ();

   void setVerticalFlip     (bool flip);
   void setHorizontalMirror (bool mirror);

 private:
   camera_fb_t *frameBuf  = nullptr;
   uint8_t     *jpgBuf    = nullptr;
   size_t       jpgBufLen = 0;
};

extern Camera camera;
#endif