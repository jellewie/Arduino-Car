/*
  Program written by JelleWho as a school project for the self driving car


  =====TODO=====
  Test engine numbers (printline and send some values so we can see if its curving)
  Change pin numbers to match Eplan
  Change steering, we have a potmeter, and it's now a PWM engine

  TODO FIXME [MID] Add a nice engine PWM down curve
  TODO FIXME [HIGH] PDO_Emergency Needs a pulldown resistor for in case it's disconnected???
  TODO FIXME [HIGH] ADD PWM CONTROL TO STEERING
  TODO FIXME [LOW] Update LED only if the array changed? Would this be faster?
  TODO FIXME [LOW] Change LED overwrite (it should display manual control)
  TODO FIXME [MID] I think the map of the Emergency time led stuff needs to have the MAP fuction tweaked. Since it can be we skip the last step and some LEDS keep being on
*/
#include "FastLED/FastLED.h"                                        //Include the FastLED library to control the LEDs in a simple fashion
#include "interrupt.h"                                              //Include the interrupt file so we can use interupts

//const (read only) █ <variable type> https://www.arduino.cc/reference/en/#variables █ <Pin ║ Digital, Analog, pWn ║ Input, Output║ name>
const byte PDI_SensorLeft = 2;
const byte PDI_SensorBack = 3;
const byte PDI_SensorRight = 4;
const byte PWO_LED = 5;
const byte PDO_LEDBlink = 13;
const byte PDO_Emergency = 23;
const byte PDO_Steering_Read = 25;
const byte PDO_SteeringReversePoles = 27;
const byte PDO_Steering = 29;
const byte PDO_MotorBrake = 31;
const byte PDO_MotorReversePoles = 33;
const byte PDO_Motor = 35;
const byte PAI_SensorFrontLeft = A0;
const byte PAI_SensorFrontRight = A1;
const byte PAI_SensorRight = A2;
const byte PAI_SensorLeft = A3;
const byte PAI_SensorBack = A4;
const byte PAI_SensorPotmeterStuur = 2;

//Just some configuable things
const int DelayPole = 50;                                           //The delay after/for the reversement of poles (reverse engine moment)
const int DelayAncher = 10;                                         //The delay after/for the ancher is turned on
const int MaxValuePWM = 255;                                        //Max number we can send to the Engine frequency generator
const int TotalLEDs = (48 + 36) * 2;                                //The total amount of LEDS in the strip

//Just some numbers we need to transfer around..
CRGB LEDs[TotalLEDs];                                               //This is an array of LEDs. One item for each LED in your strip.
byte Retrieved[3];                                                  //The array where we put the com data in
byte SensorFrontLeft;                                               //The consitence value of the sensor though the loop
byte SensorFrontRight;                                              //^^
byte SensorRight;                                                   //^^
byte SensorLeft;                                                    //^^
byte SensorBack;                                                    //^^

byte Engine;                                                        //Engine PWM value
int EngineFrom;                                                     //Engine status start position
int EngineGoTo;                                                     //Engine status stop position
byte Steering;                                                      //Steering PWM value
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
bool LED_Driving;                                                   //If the driving LEDS needs to be on (if not we are braking)
bool LED_Emergency;                                                 //If the emergency LEDS needs to be on

void setup() {                                                      //This code runs once on start-up
  pinMode(PDI_SensorLeft, INPUT);                                   //Sometimes the Arduino needs to know what pins are OUTPUT and what are INPUT, since it could get confused and create an error. So it's set manual here
  pinMode(PDI_SensorBack, INPUT);                                   //^^
  pinMode(PDI_SensorRight, INPUT);                                  //TODO FIXME [LOW] Should we set the sensors to be PULLUP??? does this even work?
  pinMode(PAI_SensorFrontLeft, INPUT);                              //^^
  pinMode(PAI_SensorFrontRight, INPUT);                             //^^
  pinMode(PDO_Emergency, INPUT);                                    //TODO FIXME [HIGH] PDO_Emergency Needs a pulldown resistor for in case it's disconnected???
  pinMode(PDO_Steering_Read, INPUT);                                //^^
  pinMode(PWO_LED, OUTPUT);                                         //^^
  pinMode(PDO_Steering, OUTPUT);                                    //^^
  pinMode(PDO_MotorBrake, OUTPUT);                                  //^^
  pinMode(PDO_MotorReversePoles, OUTPUT);                           //^^
  pinMode(PDO_Motor, OUTPUT);                                       //^^
  pinMode(PDO_LEDBlink, OUTPUT);                                    //^^
  Serial.begin(9600);                                               //Opens serial port (to pc), sets data rate to 9600 bps
  FastLED.addLeds<WS2812B, PWO_LED, GRB>(LEDs, TotalLEDs);          //Set the LED type and such
  FastLED.setBrightness(255);                                       //Scale brightness
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 255));               //Set the whole LED strip to be blue (startup animation)
  FastLED.show();                                                   //Update
  unsigned int TimeDelay = 2000;                                    //Delay in ms for the animation
  unsigned long TimeStart = millis();                               //Set the StartTime as currenttime
  unsigned long TimeCurrent = 0;                                    //No time has passed since start time
  while (TimeCurrent < TimeDelay) {                                 //While we still need to show an animation
    int x = map(TimeCurrent, 0, TimeDelay, 0, TotalLEDs);           //Remap value
    TimeCurrent = millis() - TimeStart;                             //Recalculate current time
    fill_solid(&(LEDs[0]), x, CRGB(0, 0, 0));                       //Set X LEDs to be off
    FastLED.show();                                                 //Update
    delay(1);                                                       //Just some delay (Humans wont know this change, but the arduino likes it)
  }
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 0));                 //Set All LEDs to be off (just to be save
  FastLED.show();                                                   //Update
  Retrieved[0] = 126;                                               //Fake the emergency button from the PC, (just once on boot so when you connect the PC the PC takes this over)
  digitalWrite(PDO_LEDBlink, HIGH);                                 //Let the LED blink so we know the program has started
  attachInterrupt(digitalPinToInterrupt(PDO_Emergency), EmergencyReleased, FALLING); //If the emergency button is pressed, turn motor off (this is checked 16.000.000 times / second or so
  Serial.println("[!E0]");                                          //Send a 'we did not understand' so the PC will know we are here and see them
}

void loop() {                                                       //Keep looping the next code
  //Just some things we only need in this loop
  static int EngineGoInSteps;                                       //The amount to steps to execute the move in
  static int EngineCurrentStep;                                     //The current step we are in
  static int SteeringGoInSteps;                                     //The amount to steps to execute the move in
  static int SteeringCurrentStep;                                   //The current step we are in
  static String LastPCStateSteering = "";                           //Last steering position sended to the PC, so this is what the PC knows
  static String LastPCStateEngine = "";                             //Last engine position sended to the PC, so this is what the PC knows
  static unsigned long TimeLastTime;                                //The last time we run this code
  const static unsigned int TimeDelay = 1;                          //Delay in ms for the blink, When a osciloscoop is attacked to pin 13, the real loop delay can be
  unsigned long TimeCurrent = millis();                             //Get currenttime
  unsigned long EmergencyEndTime = 1;                               //A timer to keep the Emergency active for some time (just to remove hickups in the contact) if not 0 we need to wait this amount of ms
  unsigned int EmergencyTimeDelay = 1000;                           //Delay in ms for the animation

  if (TimeCurrent - TimeLastTime >= TimeDelay) {                    //If to much time has passed
    TimeLastTime = TimeCurrent;                                     //Set last time executed as now (since we are doing it right now, and no not fucking, we are talking about code here)
    digitalWrite(PDO_LEDBlink, !digitalRead(PDO_LEDBlink));         //Let the LED blink so we know the program is running
  }
  Emergency = digitalRead(PDO_Emergency);                           //Get emergency button state (we save this so this state is contstand in this loop)
  SensorFrontLeft  = map(analogRead(PAI_SensorFrontLeft),  0, 672, 0, 255), 0, 0); //Get the sensor data (so it would be consistance though this loop) (There being remapped to the max of a byte range)
  SensorFrontRight = map(analogRead(PAI_SensorFrontRight), 0, 672, 0, 255), 0, 0); //^^
  SensorRight      = map(analogRead(PAI_SensorRight),      0, 672, 0, 255), 0, 0); //^^
  SensorLeft       = map(analogRead(PAI_SensorLeft),       0, 672, 0, 255), 0, 0); //^^
  SensorBack       = map(analogRead(PAI_SensorBack),       0, 672, 0, 255), 0, 0); //^^
  if (Serial.available() > 0) {                                     //https://www.arduino.cc/en/Reference/ASCIIchart to see the asci chart to know what numbers are what
  PcActivity = true;                                                //Set the PcActivity
  PcEverConnected = true;                                           //We have found an PC, so give feedback about states from now on
  Retrieved[0] = Serial.read();                                     //Get Emergency info (1 = it's fine, !1 WE ARE ON FIRE!)
    delay(1);                                                       //Some delay so we are sure we retrieved the data
    Retrieved[1] = Serial.read();                                   //Read next data
    delay(1);                                                       //Some delay so we are sure we retrieved the data
    Retrieved[2] = Serial.parseInt();                               //Read next data (its an int)
    delay(1);                                                       //Some delay so we are sure we retrieved the data
    Retrieved[3] = Serial.read();                                   //Just useless, but tells the end of the int
    delay(1);                                                       //Some delay so we are sure we retrieved all the data
    while (Serial.available() > 0) {                                //If there is still data bunched up (or you accidentally send a line enter "/n/r")
      Serial.print("[?" + String(Serial.read()) + "]");             //Get the data and trow it in the trash
      delay(1);                                                     //Some delay so we are sure we retrieved all the data
    }
    //Serial.print("RX=0:" + String(Retrieved[0]) + "_1:" + String(Retrieved[1]) + "_2:" + String(Retrieved[2]) + "_3:" + String(Retrieved[3]));
    if (Retrieved[1] == 82) {                                       //If "R" retrieved (Rotate)
      OverWrite = true;                                             //Set the OverWrite to true to OverWrite the thinking of the Arduino
      SteeringGoTo = Retrieved[2];                                  //Set where to rotate to
    } else if (Retrieved[1] == 77) {                                //If "M" retrieved (Motor)
      OverWrite = true;                                             //Set the OverWrite to true to OverWrite the thinking of the Arduino
      EngineGoTo = Retrieved[2];                                    //Set the EngineGoTo state
    } else if (Retrieved[1] == 65) {                                //If "A" retrieved (Auto mode)
      OverWrite = false;                                            //reset OverWrite state (Arduino things by itzelf again)
    } else if (Retrieved[1] == 68) {                                //If "D" retrieved (Debug)
      LED_SensorDebug = !LED_SensorDebug;                           //Toggle Debug mode
    }
    LastPCStateEngine = "";                                         //Fuck up LastPCStateEngine so it will resend it
    LastPCStateSteering = "";                                       //Fuck up LastPCStateSteering so it will resend it
  }
  if (Retrieved[0] != 126 || Emergency == 0) {                      //Are we on fire? (1 = it's fine, 0 = WE ARE ON FIRE!)
  //CALL THE FIREDEPARMENT AND STOP WHAT WE ARE DOING WE ARE ON FIRE!
  LED_Emergency = true;                                           //Set the emergency LED on
  SteeringGoTo = Steering;                                        //Stop with rotating, keep where ever it is at this moment
  EngineGoTo = 0;                                                 //Set EngineGoTo to 0
  EmergencyEndTime = 0;
} else {
  if (EmergencyEndTime == 0) {                                           //If we havn't yet began
      EmergencyEndTime = millis() + EmergencyTimeDelay;             //Set timer so we will wait X ms for the Emergency button to be unpressed (just to make sure we are ready to be turned on again, and it's not a hickup)
      fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 255, 255));         //Set the whole LED strip to be yellow (Emergency released animation)
      FastLED.show();                                               //Update
    }
    TimeCurrent = millis();                                         //No time has passed since start time
    if (TimeCurrent < EmergencyEndTime) {                           //If we still need to wait a bit (just to make sure we are ready to be turned on again, and it's not a hickup)
      int x = map(TimeCurrent, 0, EmergencyTimeDelay, 0, TotalLEDs);//Remap value
      fill_solid(&(LEDs[0]), x, CRGB(0, 0, 0));                     //Set X LEDs to be off
      FastLED.show();                                               //Update
    }
    else {
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
        HeadJelle();                                                  //Call jelle's head code (change if we want to use a diffrent head
        //--------------------End Head--------------------
      }
    }
  }
  //--------------------Engine control--------------------
  LED_Backwards = false;                                            //Set engine backwards LED to be off (will be turned on before it would notice it if it needs to be on)
  if (EngineGoTo == 0) {                                            //If EngineGoTo is off
  LED_Driving = false;                                            //Set engine LED to be off
  if (Engine != EngineGoTo) {                                     //If we are not yet updated
      digitalWrite(PDO_Motor, LOW);                                 //Turn engine off
      delay(DelayAncher);                                           //Wait some time to make sure engine is off
      Engine = EngineGoTo;                                          //Update the engine state
      EngineFrom = Engine;                                          //Set current engine state to be the new state to start engine fom
    }
    digitalWrite(PDO_MotorBrake, HIGH);                             //Brake
  } else {
    if (Engine != EngineGoTo) {                                     //If we are not yet done
      if (EngineGoInSteps == 0) {                                   //If no step amount is given
        EngineGoInSteps = 1000 ;                                    //Set the step to do amount
        EngineCurrentStep = EngineGoInSteps * 2;                    //Reset current step
      }
      if (EngineGoInSteps > 0) {                                    //If there still steps To Do
        EngineGoInSteps--;                                          //Remove one from the list to do (sice we are doing one now)
        if (digitalRead(PDO_MotorBrake) == HIGH ) {                 //If the brake is on
          digitalWrite(PDO_MotorBrake, LOW);                        //Don't brake
          delay(DelayAncher);                                       //Wait some time to make sure engine is off
        }
        if (EngineGoTo > 0) {                                       //If we need to move forward
          LED_Driving = true;                                       //Set forwards driving LED on
          LED_Backwards = false;                                    //Set backwards driving LED off
          if (digitalRead(PDO_MotorReversePoles) == HIGH) {         //If pin is currently high
            digitalWrite(PDO_MotorReversePoles, LOW);               //Set pin low
            delay(DelayPole);                                       //Wait some time to make sure engine is off
          }
        } else {                                                    //If we need to move backwards
          LED_Backwards = true;                                     //Set backwards driving LED on
          LED_Driving = false;                                      //Set forwards driving LED off
          if (digitalRead(PDO_MotorReversePoles) == LOW) {          //If pin is currently low6++9+++
            digitalWrite(PDO_MotorReversePoles, HIGH);              //Set pin high
            delay(DelayPole);                                       //Wait some time to make sure engine is off
          }
        }
        if (abs(Engine) < abs(EngineGoTo)) {                        //If we need to speed up
          if (EngineCurrentStep < EngineGoInSteps / 2) {            //If we are starting up
            //Go to https://www.desmos.com/calculator and past this:
            //y=\left\{x>0\right\}\left\{x<a\right\}\left\{x<\frac{a}{2}:\frac{\frac{b}{2}}{\left(\frac{a}{2}\cdot\ \frac{a}{2}\right)}x^2,-2ba^{-2}\left(x-a\right)^2+b\right\}
            Engine = MaxValuePWM / 2 / (EngineGoInSteps * EngineGoInSteps / 4) * EngineCurrentStep * EngineCurrentStep;
          } else {
            Engine = -2 * MaxValuePWM * pow(EngineGoInSteps, -2) * (EngineCurrentStep - EngineGoInSteps) * (EngineCurrentStep - EngineGoInSteps) + MaxValuePWM;
          }
        } else if (Engine > EngineGoTo) {                           //If we need to speed down
          Engine--;                                                 //remove 1 to the engine speed
          //TODO FIXME [HIGH] Add a nice engine PWM down curve
        }
        digitalWrite(PDO_Motor, Engine);                            //Write the value to the engine
      }
    }
  }
  //--------------------Steering control--------------------
  if (SteeringGoTo < 0) {                                           //If we are going to rotate to the left
  LED_Left = true;                                                //Set left LED to be on
} else if (SteeringGoTo > 0) {                                    //If we are going to rotate to the right
  LED_Right = true;                                               //Set right LED to be on
} else {
  LED_Left = false;                                               //Set left LED bool to off
  LED_Right = false;                                              //Set right LED bool to off
}
if (SteeringGoTo == SteeringReadNow()) {
  //Stop stearing
} else {
  if (SteeringGoTo > SteeringReadNow()) {                         //If we go right
      if (digitalRead(PDO_SteeringReversePoles) == HIGH) {          //If pin is currently HIGH
        digitalWrite(PDO_SteeringReversePoles, LOW);                //Set pin L
        delay(DelayAncher);                                         //Wait some time to make sure engine is off
      }
    } else {                                                        //If we go left (Not right, not straight; so left)
      if (digitalRead(PDO_SteeringReversePoles) == LOW) {           //If pin is currently LOW
        digitalWrite(PDO_SteeringReversePoles, HIGH);               //Set pin HIGH
        delay(DelayAncher);                                         //Wait some time to make sure engine is off
      }
    }
    //Set speed
    int a = SensorFrontLeft + SensorFrontRight;
  }







  //--------------------PC communication--------------------
  if (PcEverConnected) {                                            //If a PC has ever been connected
  String EmergencyButtonState = "!";                              //Create a string (and set it to warning, will be overwriten if its save)
  if (Emergency == 1) {                                           //Check if the emergency button isn't pressed
      EmergencyButtonState = "~";                                   //Set status of emergency button to save
    }
    String NewPCState = "[" + EmergencyButtonState + String("M") + String(EngineGoTo) + "]"; //Create a new state where the pc should be in right now
    if (LastPCStateEngine != NewPCState) {                          //If the PC state is not what it should be (and the PC needs an update)
      Serial.print(NewPCState);                                     //Write the info to the PC
      LastPCStateEngine = NewPCState;                               //Update what the PC should have
    }
    NewPCState = "[" + EmergencyButtonState + String("S") + String(SteeringGoTo) + "]"; //Create a new state where the pc should be in right now
    if (LastPCStateSteering != NewPCState) {                        //If the PC state is not what it should be (and the PC needs an update)
      Serial.print(NewPCState);                                     //Write the info to the PC
      LastPCStateSteering = NewPCState;                             //Update what the PC should have
    }
  }
  //--------------------LED Control--------------------
  LEDControl();
}
int (SteeringReadNow) {
  return map(analogRead(PDO_Steering_Read), 0, 255, -127, 127);     //Read and remap potmeter, and send it back to the caller
}

void EmergencyReleased() {                                          //If the emergency button is pressed (checked 111111/sec?)
  digitalWrite(PDO_Motor, LOW);                                     //Turn engine off
  Emergency = 0;                                                    //Set the emergency pin to be low (this will tell the rest of the loop this has happened)
  SteeringGoTo = SteeringReadNow();                                 //Stop with rotating, keep where ever it is at this moment
  EngineGoTo = 0;                                                   //Set EngineGoTo to 0 (so it doesn't turn on)
  if (Engine != EngineGoTo) {                                       //If we are not yet updated (the engine isn't in set state)
    delay(DelayAncher);                                             //Wait some time to make sure engine is off
    Engine = EngineGoTo;                                            //Update the engine state
    EngineFrom = Engine;                                            //Set current engine state to be the new state to start engine fom
  }
  digitalWrite(PDO_MotorBrake, HIGH);                               //Brake
  LED_Emergency = true;                                             //Set the emergency LED on
  if (PcEverConnected) {                                            //If a PC has ever been connected
    Serial.println("[!E0]");                                        //Tell the PC an idiot has pressed the button
  }
}

void HeadJelle() {
  //--------------------Jelle's head--------------------
  static int Z = 0;
  static byte SensorLowLimit = 100;                                 //A
  static byte MiniumDifference = 5;                                 //B
  static byte MiniumStepsBackwards = 100;                           //C
  static float DividerSteering = 10;                                //D
  if (Z > 0) {                                                      //If we need to move backwards
    Z--;                                                            //Remove one from Z (Z = amounts of steps to do backwards)
    EngineGoTo = -1;                                                //Set engine state, Will be verwritten when false
    if (PDI_SensorBack > SensorLowLimit) {                          //If there is nothing behind us
      if (PDI_SensorRight > SensorLowLimit) {                       //If there is nothing right of us
        SteeringGoTo = 0;                                           //Steer left+
      } else {
        if (PDI_SensorLeft > SensorLowLimit) {                      //If there is nothing left of us
          SteeringGoTo = 180;                                       //Steer right
        } else {
          EngineGoTo = 0;                                           //Turn engine off
        }
      }
    } else {
      EngineGoTo = 0;                                               //Turn engine off (we can't move backwards
    }
  } else if (PDI_SensorLeft > SensorLowLimit and PDI_SensorRight > SensorLowLimit) { //If there is nothing in front of us
    int X = PAI_SensorFrontLeft - PAI_SensorFrontRight;             //Calculate diffrence in sensoren
    if (abs(X) > MiniumDifference) {                                //If the change is not to small
      if ((X > 0 and PDI_SensorLeft > SensorLowLimit) or (X < 0 and PDI_SensorRight > SensorLowLimit)) {  //If there is nothing on that side of us
        SteeringGoTo = Steering + (X / DividerSteering);            //Steer to that side (with the intensity of 'X/DividerSteering'
      } else {
        SteeringGoTo = Steering;                                    //Stop steering
      }
      EngineGoTo = 1;                                               //Turn engine on
    } else {
      SteeringGoTo = Steering;                                      //Stop steering
      EngineGoTo = 1;                                               //Turn engine on
    }
  } else {
    Z = MiniumStepsBackwards;                                       //Tell the code that we need to go backwards from now on
  }


  //EngineGoInSteps = 1000;                                         //Set the step to do amount
  //EngineCurrentStep = EngineGoInSteps;                            //Reset current step
  //TODO FIXME [HIGH] ADD PWM CONTROL TO STEERING
  map(EngineGoTo, -1, 1, -MaxValuePWM, MaxValuePWM);                //Remap to PWM
}

void LEDControl() {
  //60 LEDs/M
  //0.8m x 0.6m = 2.8M omtrek (en 0.48 m² oppervlakte)
  //3M x 60LEDS/M = 180LEDs totaal * (3x20ma) = 10800ma (11A) Powerbank is 26800 dus we kunnen een paar uur op full power!
  //pin voor LED = PWO_LED
  //https://github.com/FastLED/FastLED/wiki/Pixel-reference
  //WS2812 LED data takes 30µs per pixel. If you have 100 pixels, then that means interrupts will be disabLED for 3000µs, or 3ms.
  //48 x 36 LEDS = 24 <Corner> 36 <> 48 <> 36 <> 24

  const static byte DelayAnimationDriving = 100;                    //Delay in ms for the animation (minium time)
  const static byte DelayanimationBlink = 50;                       //^^
  const static byte DiscoSpeed = 1;                                 //Speed of change when in disco mode
  const static byte DelayEmergency = 15;                            //delay in ms between each step in emergency mode

  static int CounterEmergency;                                      //Create a counter for the animation step (for things like the direction indicators current amount of on LEDs)
  static byte CounterBack;                                          //^^
  static byte CounterFront;                                         //^^
  static byte CounterLeft;                                          //^^
  static byte CounterRight;                                         //^^
  static bool UpdateLEDs;                                           //If the LED color needs to be updated
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
      if ((CounterLeft > LeftLength)) {                             //If we are at max lenth
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
      CounterRight++;                                               //This will make the back LED section length bigger
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
  if (LEDBrakeWasOn and (LED_Driving or LED_Backwards)) {           //If the LED now has turned of
    LEDBrakeWasOn = false;                                          //Reset flag so this will trigger only when it happens, not when its off
    fill_solid(&(LEDs[108]), 36, CRGB(0, 0, 0));                    //Clear those LEDs
    UpdateLEDs = true;                                              //Update
  }
  if (LED_Driving) {                                                //Drive Forwards
    LEDDrivingWasOn = true;                                         //Flag that the LED is (was) on, so we can turn it off when its going off
    EVERY_N_MILLISECONDS(DelayAnimationDriving) {                   //Do every 'DelayanimationBlink' ms
      fill_solid(&(LEDs[PositionFrontMiddle - FrontLength]), (FrontLength * 2),                               CRGB(0, 0, 0));   //Setting the section to black
      fill_solid(&(LEDs[PositionFrontMiddle - (PositionFrontMiddleStatic)]), (PositionFrontMiddleStatic * 2), CRGB(0, 255, 0)); //Setting the static LEDs
      fill_solid(&(LEDs[PositionFrontMiddle - CounterFront]), (CounterFront * 2),                             CRGB(0, 255, 0)); //Setting the moving LEDs
      CounterFront++;                                               //increasing the position
      if (CounterFront > FrontLength) {                             //If the section is bigger thant the maximum,
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
      fill_solid(&(LEDs[BackMiddle - BackLength]), (BackLength * 2),                CRGB(0, 0, 0));       //Reseting the back strip
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
    if (!LED_Driving) {                                             //If not moving at all
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
      LEDs[5] = CRGB(0, 255, 0);                                    //Making status indication LED different color
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
    fill_solid(&(LEDs[167]), 2, CRGB(0, 255, 0));                   //^^
    LEDs[167] = CRGB(0, 255, 0);                                    //Set last LED to be lid
    UpdateLEDs = true;                                              //Update
  }
  if (Retrieved[3] == 42) {                                         //If we need an Retrieved[3] (42 = '*')
    LEDDisco = true;
    static byte gHue;                                               //Create a new varabile
    EVERY_N_MILLISECONDS(DiscoSpeed) {                              //Do if 20 ms have pasted
      gHue++;                                                       //Slowly cycle the "base color" through the rainbow
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
    fill_solid(&(LEDs[0]), TotalLEDs,             CRGB(0, 0, 0));   //Completly erasing all LED data so they wil al be off (this will overwrite all other programs)
    for (int i = 0; i < EmergencyAmount; i++) {                     //Beginning of the loop which will send each position and length
      poss[i] = CounterEmergency + (EmergencyOffset * i);           //This will calculate each position by adding the offset times the position number to the first position
      int posX;                                                     //This is the variable which will be used for sending position start
      if (poss[i] >= TotalLEDs) {                                   //To see if the position is to bigger than the total amount
        posX = poss[i] - TotalLEDs;                                 //Subtract the total amount of LEDs from the position number
      } else {                                                      //Otherwise it wil just use the position data without modifying it
        posX = poss[i];                                             //Just use the position number
      }
      if (posX <= (TotalLEDs - EmergencySize)) {                    //If the whole section ends before the total amount is reached it wil just us the normal way of setting the LEDs
        fill_solid( &(LEDs[posX]), EmergencySize, CRGB(255, 0, 0)); //With the standard fill_solid command from FastLED, LEDs[posX] PosX stands for beginning position, EmergencySize will stand for the size of the sections and the last one is the color
      } else if ((posX >= (TotalLEDs - EmergencySize)) && (posX <= TotalLEDs)) {//This will check if the thing is beyond the total amount of LEDs
        int calc1 = (TotalLEDs - (posX + EmergencySize)) * -1;      //Calculates the amount of LEDs which need to be set from the beginning
        int calc2 = EmergencySize - calc1;                          //Calculates the amount of LEDs which need to be set at the last so the total wil be reached but wont be bigger than the total
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
