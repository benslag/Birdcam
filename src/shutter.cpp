//
// shutter.cpp -- shutter implementation
//
// Ben Slaghekke, 1 Aug 2023
//

#define _DEBUG 0
#include "debug.h"

#include "ESP32Servo.h"
#include "shutter.h"

#define STORE_SETTINGS

#ifdef STORE_SETTINGS
#include <Preferences.h>  // for storing in flash
#endif

#define SHUTTER_GPIO      (15)  // servo connects to this pin

#define VERSION           (24)  // must match contents in flash, otherwise flash is cleared

#define ABS_MIN_SPEED        (11)  // absolute min speed in us pulse width per second
#define ABS_MAX_SPEED      (4445)  // absolute max speed in us pulse width per second
#define MAX_US             (2500)  // microseconds maximum position
#define MIN_US              (500)  // microseconds minimum position
#define OPEN_POSITION      (2000)  // microseconds
#define CLOSED_POSITION    (1000)  // microseconds
#define STEP_INTERVAL        (20)  // milliseconds per update cycle IS NOT per se servo frequency!
#define SPEED              (1000)  // default move speed, us per second must be >= 50!
#define MAX_N_MOVES          (20)  // max # repeated moves
#define MOVE_INTERVAL_TIME  (100)  // time between moves

#ifdef STORE_SETTINGS
static Preferences preferences;
#endif
Shutter     shutter;

//-------------------
void Shutter::setup ()
{
    DEBUG ("Shutter setup\n");
    attach (SHUTTER_GPIO);
    localRestoreSettings ();
    currentPosition = endPosition;
    setState ();
    writeMicroseconds (currentPosition);
}


//-------------------
void Shutter::loop () 
// call from main loop
{
   if (state == Moving) {
      uint32_t m = millis ();
      if (m - recentSample > STEP_INTERVAL) {
         recentSample = m;

         int toGo = (endPosition - currentPosition) * moveDirection;  
         if (toGo <= 0) {
            // we are done!;
            currentPosition = endPosition;
            writeMicroseconds (currentPosition);
            setState ();
            if (nMoves == 0) saveSettings (false); // only nmoves and endposition
            moveIntervalTime = millis ();  // start timer
            if (nMoves == 0) {
              DEBUG ("Move complete; shutter is %s\n", state2str (state));
            }
         }
         else {
            currentPosition += moveSpeed;  // moveSpeed includes direction
            writeMicroseconds (currentPosition);
         }
      }
   }
   else repeatMove ();
}

//----------------
void Shutter::repeatMove ()
{
   if (nMoves > 0 && state != Moving) {
      if (millis () - moveIntervalTime > MOVE_INTERVAL_TIME) {
         if (currentPosition != openPosition) open ();
         else close ();
         moveIntervalTime = millis ();
         nMoves--;
      }
   }
}

//--------------
void Shutter::startRepeatedMoves (int _nMoves)
{
    if (_nMoves < 1) _nMoves = 1;
    if (_nMoves > MAX_N_MOVES) _nMoves = MAX_N_MOVES;

    nMoves = _nMoves;
    repeatMove ();
}


//-------------------
void Shutter::open ()
// open the shutter
{
    DEBUG ("Open shutter\n");
    moveTo (openPosition);
}

//--------------------
void Shutter::close () 
// close the shutter
{
    DEBUG ("Close shutter \n");
    moveTo (closedPosition);
}



//-------------------------------
void Shutter::step (int stepSize)
// move stepSize us relative to current pos  
{ 
   moveTo (endPosition + stepSize);
}

//--------------------------
void Shutter::setSpeed (unsigned int usPerSecond)
// set the (absolute) move speed, in us per sample time
{
   if (usPerSecond > ABS_MAX_SPEED) usPerSecond = ABS_MAX_SPEED;
   if (usPerSecond < ABS_MIN_SPEED) usPerSecond = ABS_MIN_SPEED;
   absMoveSpeed = usPerSecond * STEP_INTERVAL / 1000; 
}

//--------------------------
uint Shutter::getSpeed () 
{ 
   int sp =  absMoveSpeed * 1000 / STEP_INTERVAL; // move speed in usPerSecond
   if (sp < ABS_MIN_SPEED) sp = ABS_MIN_SPEED;
   if (sp > ABS_MAX_SPEED) sp = ABS_MAX_SPEED;
   return sp;
}

//---------------------------
void Shutter::saveSettings (bool saveAll)
// save the shutter settings to flash
// they are restored in the init code
{
#ifdef STORE_SETTINGS 
   DEBUG ("Shutter::saveSettings ");
   preferences.begin ("Servo",     false); // name, read-only
   preferences.putUInt ("EndPos",       endPosition);
   preferences.putUInt ("ShutterMoves", getNShutterMoves());
   if (saveAll) {
      preferences.putUInt ("Version",   VERSION);
      preferences.putUInt ("OpenPos",   openPosition);
      preferences.putUInt ("ClosedPos", closedPosition);
      preferences.putUInt ("Speed",     getSpeed ());
   }
   preferences.end ();
   DEBUG ("done\n");
#endif
}

//------------------------------
void Shutter::restoreSettings ()
// restores settings and moves the shutter 
// to the saved position
{
   DEBUG ("Restore settings ");
   localRestoreSettings ();
   if (currentPosition != endPosition) {
      moveTo (endPosition);
   }
   DEBUG ("done \n");
}


//---------------------
void Shutter::report ()
{
    Serial.printf ("Open pos = %d, closed pos = %d, cur pos = %d, end pos = %d, moveDir = %d, moveSpeed = %d, state = %s\n",
            openPosition, closedPosition, currentPosition, endPosition, moveDirection,  getSpeed (), state2str (state));
}

//----------------------
void  Shutter::waitComplete ()
{  
   while (isMoving ()) {
      loop ();
   }
}

//------------------------
void  Shutter::getValues  (int &openPos, int &closedPos, int &moveSpeed) 
{
    openPos = openPosition; closedPos = closedPosition;
    moveSpeed = getSpeed ();
}

//---------------------------
void  Shutter::setValues  (const int openPos, const int closedPos, const int moveSpeed)
{
   bool _isOpen   = isOpen   ();
   bool _isClosed = isClosed ();
   openPosition = openPos; 
   closedPosition = closedPos;
   setSpeed (moveSpeed);
   if (_isOpen) open ();
   else if (_isClosed) close ();
   waitComplete ();
}


// -- private methods

//----------------------
const char *Shutter::state2str (Shutter::State s)
{
  return (s==Moving?"Moving":s==Idle?"Idle":s==Open?"Open":s==Closed?"Closed":"ILLEGAL STATE");
}

//----------------------
Shutter::State Shutter::setState ()
{
  if      (currentPosition != endPosition)    state = Moving;
  else if (currentPosition == openPosition)   state = Open;
  else if (currentPosition == closedPosition) state = Closed;
  else                                        state = Idle;
  DEBUG ("setState: cp = %d, ep = %d, clp = %d, op = %d, state = %s\n",
          currentPosition, endPosition, closedPosition, openPosition, state2str (state));
  return state; 
}

//------------------------------
void Shutter::moveTo (uint32_t destination)
{
   if (state != Moving && destination != currentPosition) {
      state = Moving;
      clipWrite (destination, endPosition);
      moveDirection = (endPosition >= currentPosition)?1:-1;
      moveSpeed = absMoveSpeed * moveDirection;
	    nShutterMoves++;
      DEBUG ("Nbr of shutter moves = %d\n", nShutterMoves);
   }
}

//-----------------------------------
void Shutter::localRestoreSettings ()
// restores servo settings, but
// does NOT overwrite currentPos
{
#ifdef STORE_SETTINGS
    preferences.begin ("Servo", true); // name, read-only
    uint32_t ve = preferences.getUInt ("Version", 0);
    if (ve == VERSION) {
       uint32_t op   = preferences.getUInt ("OpenPos",   OPEN_POSITION);
       clipWrite (op, openPosition);
       uint32_t cp   = preferences.getUInt ("ClosedPos", CLOSED_POSITION);
       clipWrite (cp, closedPosition);
       uint32_t ep   = preferences.getUInt ("EndPos",    CLOSED_POSITION);
       clipWrite (ep, endPosition);
       uint32_t sp   = preferences.getUInt ("Speed",     SPEED);
	     nShutterMoves = preferences.getUInt ("ShutterMoves", 0);
       if (sp > ABS_MAX_SPEED) sp = 400;
       setSpeed (sp);
    }
    else {
       clipWrite (OPEN_POSITION,   openPosition);
       clipWrite (CLOSED_POSITION, closedPosition);
       clipWrite (CLOSED_POSITION, endPosition);
       setSpeed  (SPEED);
    }
    preferences.end ();
#else
    clipWrite (OPEN_POSITION,   openPosition);
    clipWrite (CLOSED_POSITION, closedPosition);
    clipWrite (CLOSED_POSITION, endPosition);
    setSpeed  (SPEED);
#endif
}


//---------------------
void Shutter::clipWrite (int pos, int &dest)
// clip a position within allowed position range, and write it
{
   if      (pos < MIN_US) pos = MIN_US;
   else if (pos > MAX_US) pos = MAX_US;
   dest = pos;
}