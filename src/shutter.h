//
// shutter.h: class for a servo controlled shutter
// 
// Ben Slaghekke, 1 Aug 2023
//
#ifndef _SHUTTER_H
#define _SHUTTER_H

#include <ESP32Servo.h>


class Shutter: public Servo {
  public:
    enum State {Closed, Open, Moving, Idle};  // Idle is a non-moving position not Closed or Open
    Shutter () {}
    void     setup ();            // init Shutter
    void     loop ();             // call from main loop
    void     open  ();            // open, do not wait for completion
    void     close ();            // close, do not wait for completion
    void     startRepeatedMoves (int nMoves);  // move <nMoves> times
    bool     isMoving ()          { return (state == Moving || currentPosition != endPosition    || nMoves != 0);}
    bool     isOpen ()            { return (state != Moving && currentPosition == openPosition   && nMoves == 0);}
    bool     isClosed ()          { return (state != Moving && currentPosition == closedPosition && nMoves == 0);}
    void     step (int stepSize); // step relative to current position
    void     markOpen   ()        {openPosition   = endPosition;} // call this the open position
    void     markClosed ()        {closedPosition = endPosition;} // call this the closed position
    uint     getSpeed ();         // in us per second
    void     setSpeed (unsigned int usPerSecond); // set move speed in us per second
    void     saveSettings (bool saveAll = true);     // keep settings in flash 
    void     restoreSettings ();  // restore
    void     report     ();       // print values
	  uint32_t getNShutterMoves ()  {return nShutterMoves;}  // total shutter moves
	  uint32_t movesLeft        ()  {return nMoves;}         // moves left for this repeated move
    void     waitComplete ();     // wait for completion of move(s); 
    void     getValues  (int &openPos, int &closedPos, int &moveSpeed);
    void     setValues  (const int openPos, const int closedPos, const int moveSpeed);
    int      toUs (const int angle)  {return (speedToUs (angle) + 500);}   // 1000 us = 90 deg
    int      toDeg (const int angle) {return (speedToDeg(angle - 500));}
    int      speedToUs  (const int angle) {return (angle * 11111 + 500) / 1000;} // speed in deg per second
    int      speedToDeg (const int angle) {return (angle * 1000 + 5555) / 11111;}
  private:
    const char *state2str (State s);
    void     moveTo     (uint32_t destination);
    void     localRestoreSettings (); // restore settings but not servo position
    void     clipWrite  (int value, int &destination);     // clip position and write 
    void     repeatMove (void);
    State    setState ();
  
    int      openPosition;      // the open position in usec
    int      closedPosition;    // the closed position in usec
    int      endPosition;       // destination position of servo
    uint32_t absMoveSpeed;      // microseconds per sample time, positive
	  uint32_t nShutterMoves;     // total number of shutter moves
    State    state;             // Closed, Open, Moving, Idle
    int      currentPosition;   // current position of the servo
    int      moveDirection;     // 1 if moving to higher uS, -1 if moving to lower uS
    int      moveSpeed;         // microseconds per sample time 
    uint32_t recentSample;      // most recently sampled time [millis ()]
    uint32_t nMoves;            // for repeated moves
    uint32_t moveIntervalTime;  // for measuring time between moves
};

extern Shutter shutter;

#endif
