/*
  Program written by JelleWho as a school project for the self driving car


  =====TODO=====
  TODO FIXME [MID] Add a nice engine PWM down curve
  TODO FIXME [LOW] Update LED only if the array changed? Would this be faster?
  TODO FIXME [LOW] Change LED overwrite (it should display manual control)
  TODO FIXME [HIGH] Invert Relay 1-8 since LOW=ON

  TODO FIXME [HIGH] Test!!!! if on startup power would be send to the engine. the relays will blow if this happens! so test first
  Thoughts of Jelle on this issue: (It would be fine, but checking doesn't hurt)
      1) The ON/OFF relay will turn on
      2) The SHORT ENGING relay will turn on
      3) The PWM is not set, so no voltage would be set
      4) In boot both above names relays will be turned off (we are talking about ms here I think)



  TODO TEST [HIGH] change SteeringReadNow() mapping . We can also do an /2 of the speed in JelleHead() to responce slower
  TODO TEST [MID] I think the map of the Emergency time led stuff needs to have the MAP fuction tweaked. Since it can be we skip the last step and some LEDS keep being on

  Kown glitches:
  - After approximately 50 days of continues time the emergency animation could play again (overflow of TimeStart) (LONG value, and when it's is 0 we start the animation)
*/
#include "FastLED/FastLED.h"                                        //Include the FastLED library to control the LEDs in a simple fashion
#include "interrupt.h"                                              //Include the interrupt file so we can use interrupts

//const (read only) █ <variable type> https://www.arduino.cc/reference/en/#variables █ <Pin ║ Digital, Analog, pWn ║ Input, Output║ name>
const byte PWO_LED = 5;                                             //Where the <LED strip> is connected to
const byte PWO_Motor = 6;                                           //Frequency controller motor relay
const byte PWO_Steering = 7;                                        //Frequency controller steering relay
const byte PDO_LEDBlink = 13;                                       //LED that’s blinks each loop relay
const byte PDO_MotorOnOff = 23;                           //K1      //Steering on/off relay
const byte PDO_MotorReversePoles = 25;                    //K2      //Reverse polarity motor relay
const byte PDO_MotorBrakeAnchor = 27;                     //K3      //Motor brake anchor relay (short the motor)
const byte PDO_SteeringOnOff = 29;                        //K4      //Steering on/off relay
const byte PDO_SteeringReversePoles = 31;                 //K5      //Reverse polarity steering relay
//const byte PDO_SpareRelay = 33;                         //K6      //(no pinMode set)
const byte PDO_MotorReversePoles2 = 35;                   //K7      //Reverse polarity motor relay
const byte PDO_SteeringReversePoles2 = 37;                //K8      //Reverse polarity steering relay
const byte PDO_Emergency = 53;                                      //Emergency button feedback
const byte PAI_SensorFrontLeft = A0;
const byte PAI_SensorFrontRight = A1;
const byte PAI_SensorRight = A2;
const byte PAI_SensorBack = A3;
const byte PAI_SensorLeft = A4;
const byte PAI_SensorPotmeterStuur = A5;

//Just some configurable things
const int DelayPole = 50;                                           //The delay after/for the reverse-ment of poles (reverse engine moment)
const int DelayAncher = 10;                                         //The delay after/for the anchor is turned on
const int MaxValuePWM = 255 / 2;                                    //Max number we can send to the Engine frequency generator
const int TotalLEDs = (48 + 36) * 2;                                //The total amount of LEDS in the strip
const byte SteeringMinimum = 25;                                    //Below this diffrence we won't steer
const unsigned int AnimationTimeEmergency = 1000;                   //Delay in ms for the animation of the reinitialize of the program (Emergency has been lifted)
const unsigned int AnimationTimeBooting = 2000;                     //Delay in ms for the animation on start

//Just some numbers we need to transfer around..
CRGB LEDs[TotalLEDs];                                               //This is an array of LEDs. One item for each LED in your strip.
byte Retrieved[3];                                                  //The array where we put the com data in
byte SensorFrontLeft;                                               //The contestant value of the sensor though the loop
byte SensorFrontRight;                                              //^^
byte SensorRight;                                                   //^^
byte SensorLeft;                                                    //^^
byte SensorBack;                                                    //^^

int Engine;                                                         //Engine PWM value
int EngineFrom;                                                     //Engine status start position
int EngineGoTo;                                                     //Engine status stop position
int Steering;                                                       //Steering PWM value
int SteeringFrom;                                                   //Steering status start position
int SteeringGoTo;                                                   //Steering status stop position

bool Emergency;                                                     //The emergency button state
bool OverWrite;                                                     //If we need to overwrite the self thinking
bool PcEverConnected;                                               //If we ever found the PC (and need to send the messages)
bool PcActivity;                                                    //If we have pc com activity
bool LED_SensorDebug;                                               //If we need to debug the sensors (show sensor data in the LEDs)
bool LED_Backwards;                                                 //If the backwards LEDS needs to be on (if not it forwards)
bool LED_Left;                                                      //If the left LEDS needs to be on
bool LED_Right;                                                     //If the right LEDS needs to be on
bool LED_Forwards;                                                  //If the driving LEDS needs to be on (if not we are braking)
bool LED_Emergency;                                                 //If the emergency LEDS needs to be on

void setup() {                                                      //This code runs once on start-up
  pinMode(PDO_MotorOnOff, OUTPUT);                                  //  (A) Sometimes the Arduino needs to know what pins are OUTPUT and what are INPUT, since it could get confused and create an error. So it's set manual here
  digitalWrite(PDO_MotorOnOff, HIGH);                               //Set Relay to be OFF        (Relay inversed)
  pinMode(PDO_SteeringOnOff, OUTPUT);                               //^^(A)
  digitalWrite(PDO_SteeringOnOff, HIGH);                            //Set Relay to be OFF        (Relay inversed)
  pinMode(PDO_MotorReversePoles, OUTPUT);                           //^^(A)
  pinMode(PDO_LEDBlink, OUTPUT);                                    //^^
  pinMode(PDO_MotorReversePoles, OUTPUT);                           //^^
  pinMode(PDO_MotorBrakeAnchor, OUTPUT);                            //^^
  pinMode(PDO_SteeringReversePoles, OUTPUT);                        //^^
  pinMode(PDO_MotorReversePoles2, OUTPUT);                          //^^
  pinMode(PDO_SteeringReversePoles2, OUTPUT);                       //^^
  pinMode(PDO_Emergency, INPUT);                                    //^^
  pinMode(PAI_SensorFrontLeft, INPUT);                              //^^
  pinMode(PAI_SensorFrontRight, INPUT);                             //^^
  pinMode(PAI_SensorRight, INPUT);                                  //^^
  pinMode(PAI_SensorBack, INPUT);                                   //^^
  pinMode(PAI_SensorLeft, INPUT);                                   //^^
  pinMode(PAI_SensorPotmeterStuur, INPUT);                          //^^
  pinMode(PWO_LED, OUTPUT);                                         //^^
  pinMode(PWO_Motor, OUTPUT);                                       //^^
  pinMode(PWO_Steering, OUTPUT);                                    //^^
  Serial.begin(9600);                                               //Opens serial port (to pc), sets data rate to 9600 bps
  FastLED.addLeds<WS2812B, PWO_LED, GRB>(LEDs, TotalLEDs);          //Set the LED type and such
  FastLED.setBrightness(150);                                       //Scale brightness
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 255));               //Set the whole LED strip to be blue (startup animation)
  FastLED.show();                                                   //Update
  unsigned long TimeStart = millis();                               //Set the StartTime as currenttime
  unsigned long TimeCurrent = 0;                                    //No time has passed since start time
  while (TimeCurrent < AnimationTimeBooting) {                      //While we still need to show an animation
    int x = map(TimeCurrent, 0, AnimationTimeBooting, 0, TotalLEDs);//Remap value
    TimeCurrent = millis() - TimeStart;                             //Recalculate time past since start of the animation
    fill_solid(&(LEDs[0]), x, CRGB(0, 0, 0));                       //Set X LEDs to be off
    FastLED.show();                                                 //Update
    delay(1);                                                       //Just some delay (Humans wont know this change, but the Arduino likes it)
  }
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 0));                 //Set All LEDs to be off (just to be save
  FastLED.show();                                                   //Update
  Retrieved[0] = 126;                                               //Fake the emergency button from the PC, (just once on boot so when you connect the PC the PC takes this over)
  digitalWrite(PDO_LEDBlink, HIGH);                                 //Let the LED blink so we know the program has started
  attachInterrupt(digitalPinToInterrupt(PDO_Emergency), EmergencyPressed, FALLING); //If the emergency button is pressed, turn motor off (this is checked 16.000.000 times / second or so
  Serial.println("[!E0]");                                          //Send a 'we did not understand' so the PC will know we are here and see them
  Serial.setTimeout(2);                                             //Set the timeout time of data read (ms)
  PcEverConnected = true;                                           //TEMP TODO We can remove this line, so it will default to not sending data (will take less CPU)
}

void loop() {                                                       //Keep looping the next code
  //Just some things we only need in this loop
  static int EngineGoInSteps;                                       //The amount to steps to execute the move in
  static int EngineCurrentStep;                                     //The current step we are in
  static String LastPCStateSteering = "";                           //Last steering position send to the PC, so this is what the PC knows
  static String LastPCStateEngine = "";                             //Last engine position send to the PC, so this is what the PC knows
  static unsigned long TimeLastTime = 0;                            //The last time we run this code
  const static unsigned int TimeDelay = 1;                          //Delay in ms for the blink, When a oscilloscope is attacked to pin 13, the real loop delay can be
  unsigned long TimeCurrent = millis();                             //Get currenttime
  unsigned long TimeStart = 1;                                      //A timer to keep the Emergency active for some time (just to remove hiccups in the contact) if not 0 we need to wait this amount of ms
  if (TimeCurrent - TimeLastTime >= TimeDelay) {                    //If to much time has passed
    TimeLastTime = TimeCurrent;                                     //Set last time executed as now (since we are doing it right now, and no not fucking, we are talking about code here)
    digitalWrite(PDO_LEDBlink, !digitalRead(PDO_LEDBlink));         //Let the LED blink so we know the program is running
  }
  Emergency = digitalRead(PDO_Emergency);                           //Get emergency button state (we save this so this state is contestant in this loop)
  SensorFrontLeft  = map(analogRead(PAI_SensorFrontLeft),  0, 674, 0, 255); //Get the sensor data (so it would be consistence though this loop) (There being remapped to the max of a byte range)
  SensorFrontRight = map(analogRead(PAI_SensorFrontRight), 0, 674, 0, 255); //^^
  SensorRight      = map(analogRead(PAI_SensorRight),      0, 674, 0, 255); //^^
  SensorLeft       = map(analogRead(PAI_SensorLeft),       0, 674, 0, 255); //^^
  SensorBack       = map(analogRead(PAI_SensorBack),       0, 674, 0, 255); //^^
  if (Serial.available() > 0) {                                     //https://www.arduino.cc/en/Reference/ASCIIchart to see the asci chart to know what numbers are what
    PcActivity = true;                                              //Set the PcActivity
    PcEverConnected = true;                                         //We have found an PC, so give feedback about states from now on
    /*
      PC -> Arduino
      1st Byte = '~'                                            = This is the emergency button state where '~' it's save (Anything else is not save)
      2nd Byte = 'M' or 'm' or 'S' or 'A' or 'D' or 'd' or 'E'  = (opt1) What Value need to be changed; Motor, motor reversed, Steering, Auto mode on, Debug on, debug off. Or 'E' for Error in data please resend
      3rd Byte = '0 to 255'                                     = (opt1) Value
      4th Byte = '#'                                            = (opt1)

      Arduino -> PC
      1st Byte = '['                                            = Start
      2nd Byte = 'M' or 'S' or [E]                              = What Value is comming. Or 'E' for Error in data please resend
      3rd Byte = '-255 to 255'                                  = (opt1) Value
      4th Byte = ']'                                            = (opt1) Stop
    */
    Retrieved[0] = Serial.read();                                   //Get Emergency info (1 = it's fine, !1 WE ARE ON FIRE!)
    Retrieved[1] = Serial.read();                                   //Read next data
    Retrieved[2] = Serial.parseInt();                               //Read next data (its an int)
    Retrieved[3] = Serial.read();                                   //Just useless, but tells the end of the int
    //Serial.println("RX=0:" + String(Retrieved[0]) + " 1:" + String(Retrieved[1]) + " 2:" + String(Retrieved[2]) + " 3:" + String(Retrieved[3]));//+ " 4:" + String(Retrieved[4]) + " Number:" + String(Retrieved[11]) + " Sizes:" + String(SizeRetrieved) + ", " + String(SizeRetrievedCalc));
    while (Serial.available() > 0) {                                //If there is still data bunched up (or you accidentally send a line enter "/n/r")
      Serial.print("[?" + String(Serial.read()) + "]");             //Get the data and throw it in the trash
      delay(1);                                                     //Some delay so we are sure we retrieved all the data
    }
    bool DataValid = false;                                         //Set the tetrieved data to be incorrect as default
    if (Retrieved[0] != 255 and Retrieved[1] != 255 and Retrieved[3] != 255) { //If data is correct (no emthy data has been recieved)
      DataValid = true;                                             //Set data to be valid
    } else if (Retrieved[1] == 'E') {                               //If "69" retrieved (Rotate)
      DataValid = true;                                             //Set data to be valid
    }
    if (!DataValid) {                                               //Data is invalid
      Serial.print("[E]");                                          //Request data again
    } else {
      if (Retrieved[1] == 'R') {                                    //If "82" retrieved (Rotate)
        OverWrite = true;                                           //Set the OverWrite to true to OverWrite the thinking of the Arduino
        SteeringGoTo = Retrieved[3];                                //Set where to rotate to
      } else if (Retrieved[1] == 'M') {                             //If "77" retrieved (Motor)
        OverWrite = true;                                           //Set the OverWrite to true to OverWrite the thinking of the Arduino
        EngineGoTo = Retrieved[2];                                  //Set the EngineGoTo state
      } else if (Retrieved[1] == 'm') {                             //If "109" retrieved (motor reversed)
        OverWrite = true;                                           //Set the OverWrite to true to OverWrite the thinking of the Arduino
        EngineGoTo = Retrieved[2] * -1;                             //Reverse motor
      } else if (Retrieved[1] == 'A') {                             //If "65" retrieved (Auto mode)
        OverWrite = false;                                          //reset OverWrite state (Arduino things by it’s self again)
      } else if (Retrieved[1] == 'D') {                             //If "68" retrieved (Debug)
        LED_SensorDebug = true;                                     //Debug mode on
      } else if (Retrieved[1] == 'd') {                             //If "100" retrieved (Debug)
        LED_SensorDebug = false;                                    //Debug mode off
      }
      LastPCStateEngine = "";                                       //Fuck up LastPCStateEngine so it will resend it
      LastPCStateSteering = "";                                     //Fuck up LastPCStateSteering so it will resend it
    }
  }
  if (Retrieved[0] != 126 || Emergency == 0) {                      //Are we on fire? (1 = it's fine, 0 = WE ARE ON FIRE!)
    //CALL THE FIREDEPARMENT AND STOP WHAT WE ARE DOING WE ARE ON FIRE!
    LED_Emergency = true;                                           //Set the emergency LED on
    SteeringGoTo = SteeringReadNow();                               //Stop with rotating, keep where ever it is at this moment
    EngineGoTo = 0;                                                 //Set EngineGoTo to 0
    TimeStart = 0;                                                  //Reset so the Emergency button will keep being active for X time after released
  } else {
    if (TimeStart == 0) {                                           //If we haven’t yet began
      TimeStart = millis();                                         //Set the StartTime as current time
      fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 255, 255));         //Set the whole LED strip to be yellow (Emergency released animation)
      FastLED.show();                                               //Update
    }
    TimeCurrent = millis() - TimeStart;                             //Recalculate time past since start of the animation
    if (TimeCurrent < TimeDelay) {                                  //If we still need to wait a bit (just to make sure we are ready to be turned on again, and it's not a hickup)
      int x = map(TimeCurrent, 0, AnimationTimeEmergency, 0, TotalLEDs);//Remap value
      fill_solid(&(LEDs[0]), x, CRGB(0, 0, 0));                     //Set X LEDs to be off
      FastLED.show();                                               //Update
    } else {
      LED_Emergency = false;                                        //Set the emergency LED off
      if (!OverWrite) {                                             //If the Arduino needs to think (not user input overwrite)
        //Add code here to control the Engine and steering

        //Input
        //use PAI_SensorFrontLeft for the state of SensorFrontLeft
        //use PAI_SensorFrontRight for the state of SensorFrontRight
        //use PDI_SensorLeft for the state of SensorLeft
        //use PDI_SensorBack for the state of SensorBack
        //use PDI_SensorRight for the state of SensorRight

        //Output
        //Use 'EngineGoTo = 255;'  to turn on the engine on
        //Use 'EngineGoTo = 0;'  to turn on the engine off
        //Use 'EngineGoTo = -255;' to turn on the engine on but backwards
        //Use 'EngineGoInSteps = Y' to tell the amount of steps to do (when left 0 it will be set to 1K)
        //Use 'SteeringGoTo = X;'  to rotate to X
        HeadJelle();                                                //Call jelle's head code (change if we want to use a different head
        //--------------------End Head--------------------
      }
    }
  }
  //--------------------Engine control--------------------
  //Serial.println("GoTo" + String(EngineGoTo) + " Engine" + String(Engine) + " Step" + String(EngineCurrentStep));
  if (EngineGoTo == 0) {                                            //If EngineGoTo is off
    if (Engine != EngineGoTo) {                                     //If we are not yet updated
      SetEngineOn(false);                                           //Set Engine off
      Engine = 0;                                                   //Reset
      EngineFrom = 0;                                               //Reset
      EngineGoInSteps = 0;                                          //Reset
    }
  } else {
    if (Engine != EngineGoTo) {                                     //If we are not yet done [Switch engine to right state (turn if off if we do this)]
      if (EngineGoTo > 0) {                                         //If we need to move forward
        SetEngineForward(true);                                     //Set Engine to move forward
      } else {                                                      //If we need to move backwards [Switch engine to right state (turn if off if we do this)]
        SetEngineForward(false);                                    //Set Engine to move backwards
      }
      SetEngineOn(true);                                            //Set Engine on
      if (Engine < EngineGoTo) {                                    //If we need to speed up
        Engine++;
      } else {                                                      //If we need to speed down
        Engine--;                                                   //remove 1 to the engine speed
      }
      //        if (EngineGoInSteps == 0) {                                 //If no step amount is given
      //          EngineGoInSteps = 15 ;                                    //Set the step to do amount
      //          EngineCurrentStep = 0;                                    //Reset current step
      //        }
      //        EngineCurrentStep++;                                        //Remove one from the list to do (since we are doing one now)
      //        if (EngineCurrentStep < EngineGoInSteps / 2) {              //If we are starting up
      //          //Go to https://www.desmos.com/calculator and past this:    \frac{\frac{b}{2}}{\left(\frac{a}{2}\cdot\ \frac{a}{2}\right)}x^2
      //          Engine = MaxValuePWM / 2 / (EngineGoInSteps * EngineGoInSteps / 4) * EngineCurrentStep * EngineCurrentStep;
      //        } else {
      //          //Go to https://www.desmos.com/calculator and past this:    -2ba^{-2}\left(x-a\right)^2+b
      //          Engine = -2 * MaxValuePWM * pow(EngineGoInSteps, -2) * (EngineCurrentStep - EngineGoInSteps) * (EngineCurrentStep - EngineGoInSteps) + MaxValuePWM;
      //        }
    }
  }
  analogWrite(PWO_Motor, abs(Engine));                                   //Write the value to the engine
  //--------------------Steering control--------------------
  //SteeringGoTo      = -127 tot 127    = Head program asked to go here
  //Steering          = 0    tot 255    = The PWM value right now
  //SteeringReadNow() = -127 tot 127    = The place where the steering is at

  //SteeringNow       = 0    tot 255    = The place where the steering is at
  //int SteeringNow = analogRead(PAI_SensorPotmeterStuur);

  if (SteeringGoTo < 0) {                                           //If we are going to rotate to the left
    LED_Left = true;                                                //Set left LED to be on
  } else if (SteeringGoTo > 0) {                                    //If we are going to rotate to the right
    LED_Right = true;                                               //Set right LED to be on
  } else {
    LED_Left = false;                                               //Set left LED bool to off
    LED_Right = false;                                              //Set right LED bool to off
  }
  if (SteeringGoTo == SteeringReadNow()) {                          //If we are where we need to be
    Steering = 0;                                                   //Stop stearing engine
  } else {
    if (SteeringGoTo > SteeringReadNow()) {                         //If we go right
      SetSteeringLeft(true);                                        //Steer to the left
    } else {                                                        //If we go left (Not right, not straight; so left)
      SetSteeringLeft(false);                                       //Steer to the right
    }
    Steering = abs(SensorFrontLeft - SensorFrontRight);             //Set the speed to be the amount of free space between the sensors (and map to 0-255)
  }
  if (Steering < SteeringMinimum) {                                 //If we don't need to steer
    SetEngineOn(false);                                             //Set Steering engine on
  } else {
    SetEngineOn(true);                                              //Set Steering engine off
  }
  analogWrite(PWO_Steering, Steering);                              //Write the value to the engine
  //--------------------PC communication--------------------





  Serial.println("E" + String(Engine) + " G" + String(EngineGoTo) + " A" + String(SensorFrontLeft) + " " + String(SensorFrontRight) + " " + String(SensorBack));
  delay(250);




  if (PcEverConnected) {
    String EmergencyButtonState = "!";                              //Create a string (and set it to warning, will be overwritten if its save)
    if (Emergency == 1) {                                           //Check if the emergency button isn't pressed
      EmergencyButtonState = "~";                                   //Set status of emergency button to save
    }
    String NewPCState = "[" + EmergencyButtonState + String("M") + String(Engine) + "]"; //Create a new state where the pc should be in right now
    if (LastPCStateEngine != NewPCState) {                          //If the PC state is not what it should be (and the PC needs an update)
      //Serial.print(NewPCState);                                     //Write the info to the PC
      LastPCStateEngine = NewPCState;                               //Update what the PC should have
    }
    NewPCState = "[" + EmergencyButtonState + String("S") + String(Steering) + "]"; //Create a new state where the pc should be in right now
    if (LastPCStateSteering != NewPCState) {                        //If the PC state is not what it should be (and the PC needs an update)
      //Serial.print(NewPCState);                                     //Write the info to the PC
      LastPCStateSteering = NewPCState;                             //Update what the PC should have
    }
  }
  //--------------------LED Control--------------------
  LEDControl();
}

void SetSteeringOn(bool SteerOn) {                                  //If called with (true) the steering motor will be turned on
  if (SteerOn) {                                                    //If we need to steer
    if (digitalRead(PDO_SteeringOnOff) == HIGH) {                   //If the engine is off (Relay inversed)
      analogWrite(PDO_SteeringOnOff, LOW);                          //Turn engine on       (Relay inversed)
      delay(DelayAncher);                                           //Wait some time
    }
  } else {
    analogWrite(PDO_SteeringOnOff, 0);                              //Set the PWM to be off (just to be sure)
    if (digitalRead(PDO_SteeringOnOff) == LOW) {                    //If the engine is on  (Relay inversed)
      digitalWrite(PDO_SteeringOnOff, HIGH);                        //Turn engine off      (Relay inversed)
      delay(DelayAncher);                                           //Wait some time
    }
  }
}

void SetSteeringLeft(bool State) {                                  //If called with (true) the steer engine will be set to move left (if will only move to the left if also power is applied)
  byte D;                                                           //Create a new byte (Basically the analog StateDirection state)
  if (State) {                                                      //If StateDirection needs to be HIGH
    LED_Left = true;                                                //Flag LED_Left to be on
  } else {
    D = 1;                                                          //Set analog value to be HIGH
    LED_Left = false;                                               //Flag LED_Left to be on
  }
  if (digitalRead(PDO_SteeringReversePoles) == !D) {                //If we need to move forward or backwards but we aren't (Relay inversed)
    SetEngineOn(false);                                             //Make sure the engine if off
    analogWrite(PDO_SteeringReversePoles, D * 255);                 //Set right direction     (Relay inversed)
    analogWrite(PDO_SteeringReversePoles2, D * 255);                //Set right direction     (Relay inversed)
    delay(DelayPole);                                               //Wait some time to make sure engine is off
  }
}

void SetEngineOn(bool EngineOn) {                                   //If called with (true) the main engine will be turned on
  if (EngineOn) {                                                   //If we need to drive
    if (digitalRead(PDO_MotorOnOff) == HIGH) {                      //If the engine is off    (Relay inversed)
      if (digitalRead(PDO_MotorBrakeAnchor) == LOW) {               //If Engine Shorted       (Relay inversed)
        digitalWrite(PDO_MotorBrakeAnchor, HIGH);                   //Don't short the engine  (Relay inversed)
        delay(DelayAncher);                                         //Wait some time
      }
      analogWrite(PDO_MotorOnOff, LOW);                             //Turn engine on          (Relay inversed)
      delay(DelayAncher);                                           //Wait some time
    }
  } else {
    analogWrite(PWO_Motor, 0);                                      //Set the PWM to be off (just to be sure)
    if (digitalRead(PDO_MotorOnOff) == LOW) {                       //If the engine is on     (Relay inversed)
      digitalWrite(PDO_MotorOnOff, HIGH);                           //Turn engine off         (Relay inversed)
      delay(DelayAncher);                                           //Wait some time
      if (digitalRead(PDO_MotorBrakeAnchor) == HIGH) {              //If Engine isn't Shorted (Relay inversed)
        digitalWrite(PDO_MotorBrakeAnchor, LOW);                    //Short the engine        (Relay inversed)
        delay(DelayAncher);                                         //Wait some time
      }
    }
  }
}

void SetEngineForward(bool State) {                                 //If called with (true) the main engine will be set to move forward
  byte D;                                                           //Create a new byte (Basically the analog StateDirection state)
  if (State) {                                                      //If StateDirection needs to be HIGH
    LED_Forwards = true;                                            //Set forwards driving LED on
    LED_Backwards = false;                                          //Set backwards driving LED off
  } else {
    D = 1;                                                          //Set analog value to be HIGH
    LED_Forwards = false;                                           //Set forwards driving LED on
    LED_Backwards = true;                                           //Set backwards driving LED off
  }
  if (digitalRead(PDO_MotorReversePoles) == !D) {                   //If we need to move forward or backwards but we aren't (Relay inversed)
    SetEngineOn(false);                                             //Make sure the engine if off
    analogWrite(PDO_MotorReversePoles, D * 255);                    //Set right direction      (Relay inversed)
    analogWrite(PDO_MotorReversePoles2, D * 255);                   //Set right direction      (Relay inversed)
    delay(DelayPole);                                               //Wait some time to make sure engine is off
  }
}

int SteeringReadNow() {                                             //returns the steering potmeter on a scale of -127 to 127
  return map(analogRead(PAI_SensorPotmeterStuur ), 0, 1024, -127, 127); //Read and remap pot meter, and send it back to the caller
}

void EmergencyPressed() {                                           //If the emergency button is pressed (checked 111111/sec?)
  Emergency = 0;                                                    //Set the emergency pin to be low (this will tell the rest of the loop this has happened)
  SteeringGoTo = SteeringReadNow();                                 //Stop with rotating, keep where ever it is at this moment
  EngineGoTo = 0;                                                   //Set EngineGoTo to 0 (so it doesn't turn on)
  LED_Forwards = false;                                             //Set engine LED to be off
  analogWrite(PWO_Motor, 0);                                        //Turn engine off (Set PWN to 0)
  SetEngineOn(false);                                               //Turn engine off
  SetSteeringOn(false);                                             //Turn steering engine off
  Engine = EngineGoTo;                                              //Update the engine state
  EngineFrom = Engine;                                              //Set current engine state to be the new state to start engine from
  LED_Emergency = true;                                             //Set the emergency LED on
  if (PcEverConnected) {                                            //If a PC has ever been connected
    Serial.println("[!E0]");                                        //Tell the PC an idiot has pressed the button
  }
}

void HeadJelle() {                                                  //The code of jelle that calculates in his way where to move to and at what speed and such
  //--------------------Jelle's head--------------------
  static int Z = 0;
  static byte SensorFreeSpaceLimit = 200;                           //A (Dont forget that Sensor 255=It's a hit, and 0=nothing to see!)
  static byte MiniumDifference = 10;                                //B Minium diffrence for updating the GOTO (else we would change speed to much)
  static byte MiniumStepsBackwards = 500;                           //C Amount of loops to do backwards
  static float DividerSteering = 10;                                //D 
  static byte MaxBackwardsSpeedDevider = 4;                         //E Max speed divided by this is the max speed we can drive backwards
  if (Z > 0) {                                                      //If we need to move backwards
    Z--;                                                            //Remove one from Z (Z = amounts of steps to do backwards)
    if (Z >= MiniumStepsBackwards - 1) {                            //If this is the first step backwards (or rather going to be)
      EngineGoTo = 0;                                               //Turn engine off (this will forge the engine to break
    } else {
      EngineGoTo = map(SensorBack, 0, SensorFreeSpaceLimit, 0, 255) * -1 / MaxBackwardsSpeedDevider; //Set engine state, Will be overwritten when false (Remapped so we can use the full raneg [remember we don't move when to close])
      if (SensorBack < SensorFreeSpaceLimit) {                      //If there is nothing behind us
        if (SensorRight < SensorFreeSpaceLimit) {                   //If there is nothing right of us
          SteeringGoTo = -127;                                      //Steer all the way left
        } else {
          if (SensorLeft < SensorFreeSpaceLimit) {                  //If there is nothing left of us
            SteeringGoTo = 127;                                     //Steer all the way right
          } else {
            EngineGoTo = 0;                                         //Turn engine off (we can't move backwards)
          }
        }
      } else {
        EngineGoTo = 0;                                             //Turn engine off (we can't move backwards)
      }
    }
  } else if (SensorFrontLeft < SensorFreeSpaceLimit and SensorFrontRight < SensorFreeSpaceLimit) {  //If there is nothing in front of us
    int NewTryEngineGoTo = (510 - SensorFrontLeft - SensorFrontRight) / 2;                          //Calculate difference in sensors
    if ((abs(abs(NewTryEngineGoTo) - EngineGoTo) > MiniumDifference) or NewTryEngineGoTo > 250) {   //If the change is not to small
      EngineGoTo = map(NewTryEngineGoTo, 255 - SensorFreeSpaceLimit, 255, 0, 255);                  //Set the speed to be the amount of free space between the 2 front sensors (Remapped so we can use the full raneg [remember we don't move when to close])
      if ((NewTryEngineGoTo > 0 and SensorRight < SensorFreeSpaceLimit) or (NewTryEngineGoTo < 0 and SensorLeft < SensorFreeSpaceLimit)) { //If there is nothing on that side of us we want to steer to
        SteeringGoTo = (NewTryEngineGoTo / DividerSteering);        //Steer to that side (with the intensity of 'NewTryEngineGoTo/DividerSteering'
      } else {
        SteeringGoTo = 0;                                           //Stop steering
      }
    } else {
      SteeringGoTo = 0;                                             //Stop steering
    }
  } else {
    Z = MiniumStepsBackwards;                                       //Tell the code that we need to go backwards from now on
  }
  if (EngineGoTo > 254) {
    EngineGoTo = 255;
  }

  //EngineGoInSteps = 1000;                                         //Set the step to do amount
  //EngineCurrentStep = EngineGoInSteps;                            //Reset current step
}

void LEDControl() {                                                 //Code that controls all the LEDs
  //60 LEDs/M
  //0.8m x 0.6m = 2.8M omtrek (and 0.48 m² surface)
  //3M x 60LEDS/M = 180LEDs total * (3x20ma) = 10800ma (11A) Power bank is 26800 so we can do a few hours at full power!
  //pin voor LED = PWO_LED
  //https://github.com/FastLED/FastLED/wiki/Pixel-reference
  //WS2812 LED data takes 30µs per pixel. If you have 100 pixels, then that means interrupts will be disabled for 3000µs, or 3ms.
  //48 x 36 LEDS = 24 <Corner> 36 <> 48 <> 36 <> 24

  const static byte DelayAnimationDriving = 100;                    //Delay in ms for the animation (minimum time)
  const static byte DelayanimationBlink = 50;                       //^^
  const static byte DiscoSpeed = 1;                                 //Speed of change when in disco mode
  const static byte DelayEmergency = 15;                            //delay in ms between each step in emergency mode

  static int CounterEmergency;                                      //Create a counter for the animation step (for things like the direction indicators current amount of on LEDs)
  static byte CounterBack;                                          //^^
  static byte CounterFront;                                         //^^
  static byte CounterLeft;                                          //^^
  static byte CounterRight;                                         //^^
  static bool UpdateLEDs;                                           //If the LED colour needs to be updated
  static bool LEDLeftWasOn;                                         //Create a bool so we can reset the LED when its going off
  static bool LEDRightWasOn;                                        //^^
  static bool LEDDrivingWasOn;                                      //^^
  static bool LEDBackwardsWasOn;                                    //^^
  static bool LEDBrakeWasOn;                                        //^^
  static bool LEDEmergencyWasOn;                                    //^^
  static bool LEDDebugWasOn;                                        //^^
  static bool LEDPcEverConnectedWasOn;                              //^^
  static bool LEDDisco;                                             //^^
  static byte PositionLeftFront = 32;                               //Start of front left turning light
  static byte PositionLeftBack = 136;                               //Start of back left turning light
  static byte LeftLength = 25;                                      //Total length
  static byte PositionRightFront = 52;                              //Start of front right turning light
  static byte PositionRightBack = 116;                              //Start of back right turning light
  static byte RightLength = 25;                                     //Total length
  static byte PositionFrontMiddle = 42;                             //Middle of the front section (middle is between two so it has an offset of 0,5 of course)
  static byte FrontLength = 10;                                     //Half of the length
  static byte PositionFrontMiddleStatic = 1;                        //half of how many LEDs will always be on
  static byte BackMiddle = 126;                                     //Middle of the LED section at the back
  static byte BackLength = 10;                                      //Half of the length
  static byte BackMiddleStatic = 1;                                 //Half of the length of the LEDs that will always be on
  static byte EmergencySize = 10;                                   //Length of the sections
  static byte EmergencyAmount = 4;                                  //Quantity of the sections
  if (LED_Left) {                                                   //Turning left
    LEDLeftWasOn = true;                                            //Flag that the LED is (was) on, so we can turn it off when its going off
    EVERY_N_MILLISECONDS(DelayanimationBlink) {                     //Do every 'DelayanimationBlink' ms
      fill_solid(&(LEDs[PositionLeftFront - LeftLength]), LeftLength,   CRGB(0, 0, 0));     //Turn front off
      fill_solid(&(LEDs[PositionLeftFront - 2]), 2,                     CRGB(255, 128, 0)); //Will set the first two LEDs at the start of the front section to on so they will always be on if this function is calLED
      fill_solid(&(LEDs[PositionLeftFront - CounterLeft]), CounterLeft, CRGB(255, 128, 0)); //Will set the increasing front left section
      fill_solid(&(LEDs[PositionLeftBack]), LeftLength,                 CRGB(0, 0, 0));     //Turn back off
      fill_solid(&(LEDs[PositionLeftBack]), 2,                          CRGB(255, 128, 0)); //Will set the first two LEDs at the start of the back section to on so they will always be on if this function is calLED
      fill_solid(&(LEDs[PositionLeftBack]), CounterLeft,                CRGB(255, 128, 0)); //Will set the increasing back left section
      CounterLeft++;                                                //This will make the front LED section length bigger
      if ((CounterLeft > LeftLength)) {                             //If we are at max length
        CounterLeft = 0;                                            //Reset counter
      }
      UpdateLEDs = true;                                            //Update
    }
  } else if (LEDLeftWasOn) {                                        //If the LED now has turned of
    CounterLeft = 0;                                                //Reset counter
    LEDLeftWasOn = false;                                           //Reset flag so this will trigger only when it happens, not when its off
    fill_solid(&(LEDs[PositionLeftFront - LeftLength]), LeftLength, CRGB(0, 0, 0)); //Clear those LEDs
    fill_solid(&(LEDs[PositionLeftBack]), LeftLength,               CRGB(0, 0, 0)); //Clear those LEDs
    UpdateLEDs = true;                                              //Update
  }
  if (LED_Right) {                                                  //Turning right
    LEDRightWasOn = true;                                           //Flag that the LED is (was) on, so we can turn it off when its going off
    EVERY_N_MILLISECONDS(DelayanimationBlink) {                     //Do every 'DelayanimationBlink' ms
      fill_solid(&(LEDs[PositionRightFront]), RightLength,                CRGB(0, 0, 0));     //Setting the front section to black
      fill_solid(&(LEDs[PositionRightFront]), 2,                          CRGB(255, 128, 0)); //Will set the first two LEDs at the start of the front section to on so they will always be on if this function is calLED
      fill_solid(&(LEDs[PositionRightFront]), CounterRight,               CRGB(255, 128, 0)); //Will set the increasing front right section
      fill_solid(&(LEDs[PositionRightBack - RightLength]), RightLength,   CRGB(0, 0, 0));     //Setting the back section to black
      fill_solid(&(LEDs[PositionRightBack - 2]), 2,                       CRGB(255, 128, 0)); //Will set the first two LEDs at the start of the back section to on so they will always be on if this function is calLED
      fill_solid(&(LEDs[PositionRightBack - CounterRight]), CounterRight, CRGB(255, 128, 0)); //Will set the increasing back right section
      CounterRight++;                                               //This will make the back-LED section length bigger
      if ((CounterRight > RightLength)) {                           //If both sections are as big as they can go,
        CounterRight = 0;                                           //And it will reset the back counter
      }
      UpdateLEDs = true;                                            //Update
    }
  } else {
    CounterRight = 0;                                               //Reset counter
    if (LEDRightWasOn) {                                            //If the LED now has turned of
      LEDRightWasOn = false;                                        //Reset flag so this will trigger only when it happens, not when its off
      fill_solid(&(LEDs[PositionRightFront]), RightLength,                CRGB(0, 0, 0)); //Setting the front section to black
      fill_solid(&(LEDs[PositionRightBack - RightLength]), RightLength,   CRGB(0, 0, 0)); //Setting the back section to black
      UpdateLEDs = true;                                            //Update
    }
  }
  if (LEDBrakeWasOn and (LED_Forwards or LED_Backwards)) {          //If the LED now has turned of
    LEDBrakeWasOn = false;                                          //Reset flag so this will trigger only when it happens, not when its off
    fill_solid(&(LEDs[108]), 36, CRGB(0, 0, 0));                    //Clear those LEDs
    UpdateLEDs = true;                                              //Update
  }
  if (LED_Forwards) {                                                //Drive Forwards
    LEDDrivingWasOn = true;                                         //Flag that the LED is (was) on, so we can turn it off when its going off
    EVERY_N_MILLISECONDS(DelayAnimationDriving) {                   //Do every 'DelayanimationBlink' ms
      fill_solid(&(LEDs[PositionFrontMiddle - FrontLength]), (FrontLength * 2),                               CRGB(0, 0, 0));   //Setting the section to black
      fill_solid(&(LEDs[PositionFrontMiddle - (PositionFrontMiddleStatic)]), (PositionFrontMiddleStatic * 2), CRGB(0, 255, 0)); //Setting the static LEDs
      fill_solid(&(LEDs[PositionFrontMiddle - CounterFront]), (CounterFront * 2),                             CRGB(0, 255, 0)); //Setting the moving LEDs
      CounterFront++;                                               //increasing the position
      if (CounterFront > FrontLength) {                             //If the section is bigger then the maximum,
        CounterFront = 0;                                           //Reset counter
      }
      UpdateLEDs = true;                                            //Update
    }
  } else {
    CounterFront = 0;                                               //Reset counter
    if (LEDDrivingWasOn) {                                          //If the LED now has turned of
      LEDDrivingWasOn = false;                                      //Reset flag so this will trigger only when it happens, not when its off
      fill_solid(&(LEDs[PositionFrontMiddle - FrontLength]), (FrontLength * 2), CRGB(0, 0, 0)); //Clear those LEDs
      UpdateLEDs = true;                                            //Update
    }
  }
  if (LED_Backwards) {                                              //Drive Backwards (and check later if we are standing still)
    LEDBackwardsWasOn = true;                                       //Flag that the LED is (was) on, so we can turn it off when its going off
    EVERY_N_MILLISECONDS(DelayAnimationDriving) {                   //Do every 'DelayanimationBlink' ms
      fill_solid(&(LEDs[BackMiddle - BackLength]), (BackLength * 2),                CRGB(0, 0, 0));       //Resetting the back strip
      fill_solid(&(LEDs[BackMiddle - (BackMiddleStatic)]), (BackMiddleStatic * 2),  CRGB(255, 255, 255)); //Setting the static LEDs
      fill_solid(&(LEDs[BackMiddle - CounterBack]), (CounterBack * 2),              CRGB(255, 255, 255)); //Setting the moving LEDs
      CounterBack++;                                                //Increasing the position value
      if (CounterBack > BackLength) {                               //If the position value is bigger than the maximum,
        CounterBack = 0;                                            //Reset counter
      }                                                             //Ending that check
      UpdateLEDs = true;                                            //Update
    }
  } else {
    CounterBack = 0;                                                //Reset counter
    if (LEDBackwardsWasOn) {                                        //If the LED now has turned of
      LEDBackwardsWasOn = false;                                    //Reset flag so this will trigger only when it happens, not when its off
      fill_solid(&(LEDs[BackMiddle - BackLength]), (BackLength * 2), CRGB(0, 0, 0)); //Clear those LEDs
      UpdateLEDs = true;                                            //Update
    }
    if (!LED_Forwards) {                                            //If not moving at all
      LEDBrakeWasOn = true;                                         //Flag that the LED is (was) on, so we can turn it off when its going off
      fill_solid(&(LEDs[108]), 36, CRGB(255, 0, 0));                //Enable brake lights
      UpdateLEDs = true;                                            //Update
    }
  }
  if (PcEverConnected) {                                            //Pc ever connected
    LEDPcEverConnectedWasOn = true;                                 //Flag that the LED is (was) on, so we can turn it off when its going off
    LEDs[5] = CRGB(0, 0, 255);                                      //Status indication LED if pc is connected
    if (PcActivity) {                                               //Pc activity
      PcActivity = false;                                           //Reseting PC activity so it can be set again if it has activity
      LEDs[5] = CRGB(0, 255, 0);                                    //Making status indication LED different colour
    }
    UpdateLEDs = true;                                              //Update
  } else if (LEDPcEverConnectedWasOn) {                             //If the LED now has turned of
    LEDPcEverConnectedWasOn = false;                                //Reset flag so this will trigger only when it happens, not when its off
    LEDs[5] = CRGB(0, 0, 0);                                        //Clear the LEDs
    UpdateLEDs = true;                                              //Update
  }
  if (LED_SensorDebug) {                                            //Sensor debug through status indication LEDs (brightness change at the moment)
    LEDDebugWasOn = true;                                           //Flag that the LED is (was) on, so we can turn it off when its going off
    LEDs[0] = CRGB(SensorLeft,       0, 0);                         //Setting values for the Sensor to the LEDs
    LEDs[1] = CRGB(SensorFrontLeft,  0, 0);                         //^^
    LEDs[2] = CRGB(SensorFrontRight, 0, 0);                         //^^
    LEDs[3] = CRGB(SensorRight,      0, 0);                         //^^
    LEDs[4] = CRGB(SensorBack,       0, 0);                         //^^
    UpdateLEDs = true;                                              //Updating the update update updater updating stuff something
  } else if (LEDDebugWasOn) {                                       //If the LED now has turned of
    LEDDebugWasOn = false;                                          //Reset flag so this will trigger only when it happens, not when its off
    fill_solid(&(LEDs[0]), 5, CRGB(0, 0, 0));                       //Clear those LEDs
    UpdateLEDs = true;                                              //Update
  }
  if (OverWrite) {                                                  //If the Program is overwritten by an pc (so manual control)
    for (int i = 0; i < (TotalLEDs / 5); i++) {                     //At the moment this is a program that will mark al locations of corners and with this enabLED it will be easier to measure different parts of the strip [TODO FIXME LOW]
      int vogels1 = i * 5;
      if (vogels1 < TotalLEDs) {
        LEDs[vogels1 - 1] = CRGB(0, 0, 255);
      }
    }
    for (int i = 0; i < (TotalLEDs / 20); i++) {
      int vogels2 = i * 20;
      if (vogels2 < TotalLEDs) {
        LEDs[vogels2 - 1] = CRGB(255, 0, 0);
      }
    }
    fill_solid(&(LEDs[23]), 2, CRGB(0, 255, 0));                    //Set corners to be lid
    fill_solid(&(LEDs[59]), 2, CRGB(0, 255, 0));                    //^^
    fill_solid(&(LEDs[107]), 2, CRGB(0, 255, 0));                   //^^
    fill_solid(&(LEDs[143]), 2, CRGB(0, 255, 0));                   //^^
    LEDs[167] = CRGB(0, 255, 0);                                    //Set last LED to be lid
    UpdateLEDs = true;                                              //Update
  }
  if (Retrieved[3] == 42) {                                         //If we need an Retrieved[3] (42 = '*')
    LEDDisco = true;
    static byte gHue;                                               //Create a new variable
    EVERY_N_MILLISECONDS(DiscoSpeed) {                              //Do if 20 ms have pasted
      gHue++;                                                       //Slowly cycle the "base colour" through the rainbow
    }
    fill_rainbow(LEDs, TotalLEDs, gHue, 7);                         //Do some funny stuff
    UpdateLEDs = true;                                              //Update
  } else if (LEDDisco) {                                            //If the LED now has turned of
    LEDDisco = false;                                               //Reset flag so this will trigger only when it happens, not when its off
    fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 0));               //Clear those LEDs
    UpdateLEDs = true;                                              //Update
  }
  if (LED_Emergency) {                                              //Emergency
    LEDEmergencyWasOn = true;                                       //Flag that the LED is (was) on, so we can turn it off when its going off
    int EmergencyOffset = TotalLEDs / EmergencyAmount;              //Calculation for calculating offset from first Position
    int poss[EmergencyAmount];                                      //Array for saving the positions of the sections
    fill_solid(&(LEDs[0]), TotalLEDs,             CRGB(0, 0, 0));   //Completely erasing all LED data so they will al be off (this will overwrite all other programs)
    for (int i = 0; i < EmergencyAmount; i++) {                     //Beginning of the loop which will send each position and length
      poss[i] = CounterEmergency + (EmergencyOffset * i);           //This will calculate each position by adding the offset times the position number to the first position
      int posX;                                                     //This is the variable which will be used for sending position start
      if (poss[i] >= TotalLEDs) {                                   //To see if the position is to bigger than the total amount
        posX = poss[i] - TotalLEDs;                                 //Subtract the total amount of LEDs from the position number
      } else {                                                      //Otherwise it will just use the position data without modifying it
        posX = poss[i];                                             //Just use the position number
      }
      if (posX <= (TotalLEDs - EmergencySize)) {                    //If the whole section ends before the total amount is reached it wil just us the normal way of setting the LEDs
        fill_solid( &(LEDs[posX]), EmergencySize, CRGB(255, 0, 0)); //With the standard fill solid command from FastLED, LEDs[posX] PosX stands for beginning position, EmergencySize will stand for the size of the sections and the last one is the colour
      } else if ((posX >= (TotalLEDs - EmergencySize)) && (posX <= TotalLEDs)) {//This will check if the thing is beyond the total amount of LEDs
        int calc1 = (TotalLEDs - (posX + EmergencySize)) * -1;      //Calculates the amount of LEDs which need to be set from the beginning
        int calc2 = EmergencySize - calc1;                          //Calculates the amount of LEDs which need to be set at the last so the total will be reached but wont be bigger than the total
        fill_solid(&(LEDs[posX]), calc2,          CRGB(255, 0, 0)); //Fills the LEDs at the beginning of the strip
        fill_solid(&(LEDs[0])   , calc1,          CRGB(255, 0, 0)); //Fills the last LEDs at the end of the strip
      }
    }
    if (CounterEmergency >= TotalLEDs) {                            //Will check if the main position is bigger than the total
      CounterEmergency = 0;                                         //If that is the case than it will reset it to 0
    } else {
      EVERY_N_MILLISECONDS(DelayEmergency) {                        //Do every 'DelayanimationBlink' ms
        CounterEmergency++;                                         //It will just set it to 0
      }
    }
    UpdateLEDs = true;                                              //Update
  } else if (LEDEmergencyWasOn) {                                   //If the LED now has turned of
    LEDEmergencyWasOn = false;                                      //Reset flag so this will trigger only when it happens, not when its off
    fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 0));               //Clear those LEDs
    UpdateLEDs = true;                                              //Update
  }
  if (UpdateLEDs) {                                                 //If we need an update
    FastLED.show();                                                 //Apply LED changes
    UpdateLEDs = false;                                             //Flag update done
  }
}
//this is the end, hope you had fun
