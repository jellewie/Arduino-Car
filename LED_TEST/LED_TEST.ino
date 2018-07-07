/*
  Program written by Jelle Wietsma as a school project for the self driving car


  =====TODO=====
  Test engine numbers (printline and send some values so we can see if its curving)
  Change pin numbers to match Eplan
  Change steering, we have a potmeter, and it's now a PWM engine

  [MID] LED Overright (full manual control)
  [LOW] LED Retrieved[3] mode


*/
#include "FastLED/FastLED.h"

//constants won't change. P =Pin. D=digital, A=analog, W=PWM. I=Input, O=Output
const int PWO_LED = 5;

//Just some configuable things
const int DelayLoop = 15;                                     //The amount of times to wait (arduinoloop)ms [15*1=15ms = 66Hz]

const int TotalLEDs = 168;                                    //The total amount of LEDS in the strip
CRGB LEDs[TotalLEDs];                                         //This is an array of LEDs.  One item for each LED in your strip.
uint8_t gHue = 1;                                             //Rotating "base color" used by many of the patterns
bool UpdateLEDs = true;
float thistimer;

const byte PAI_SensorFrontLeft = A0;
const byte PAI_SensorFrontRight = A1;
const byte PAI_SensorRight = A2;
const byte PAI_SensorLeft = A3;
const byte PAI_SensorBack = A4;
const byte PDO_LEDBlink = 13;

int sensorVal[5];

bool LED_Backwards = false;
bool LED_Left = false;
bool LED_Right = false;
bool LED_Forwards = false;
bool LED_Emergency = false;
bool PcEverConnected = false;
bool PcActivity = false;
bool LED_SensorDebug = false;
bool OverWrite = false;
int Retrieved[3] = {0, 0, 5};

byte SensorFrontLeft = 0;
byte SensorFrontRight = 60;
byte SensorRight = 120;
byte SensorLeft = 180;
byte SensorBack = 255;

byte TempDataRetrieved;

void setup() {                                                      //This code runs once on start-up
  pinMode(PWO_LED, OUTPUT);
  FastLED.addLeds<WS2812B, PWO_LED, GRB>(LEDs, TotalLEDs);
  FastLED.setBrightness(255);                                       //Scale brightness
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 255));               //Set the whole LED strip to be blue (startup animation)
  FastLED.show();                                                   //Update
  unsigned int TimeDelay = 1000;                                    //Delay in ms for the animation
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
  Serial.begin(9600);                                               //Opens serial port (to pc), sets data rate to 9600 bps
  if (Serial) {                                                     //If there is a Serial connection availible to us
    //PcEverConnected = true;                                         //We have found an PC, so give feedback about states from now on
    Serial.println("[!E0]");                                        //Send a 'we did not understand' so the PC will know we are here and see them
  }
}

void loop() {                                                       //Keep looping the next code
  static unsigned long TimeLastTime;                                //The last time we run this code
  const static unsigned int TimeDelay = 1;                          //Delay in ms for the animation (1000/X then X = in Hz)
  unsigned long TimeCurrent = millis();                             //Get currenttime
  if (TimeCurrent - TimeLastTime >= TimeDelay) {                    //If to much time has passed
    TimeLastTime = TimeCurrent;                                     //Set last time executed as now (since we are doing it right now, and no not fucking, we are talking about code here)
    digitalWrite(PDO_LEDBlink, !digitalRead(PDO_LEDBlink));         //Let the LED blink so we know the program is running
  }

  SensorLeft = analogRead(PAI_SensorFrontLeft);
  SensorFrontRight = analogRead(PAI_SensorFrontRight);
  SensorRight = analogRead(PAI_SensorRight);
  SensorLeft = analogRead(PAI_SensorLeft);
  SensorBack = analogRead(PAI_SensorBack);

  if (Serial.available()) {
    Retrieved[0] = Serial.read();
    PcActivity = true;
    delay(1);
    switch (Retrieved[0]) {
      case 43:                                                      //+
        TempDataRetrieved = 0;
        PcEverConnected = false;
        PcActivity = false;
        LED_Backwards = false;
        LED_Left = false;
        LED_Emergency = false;
        LED_Right = false;
        LED_SensorDebug = false;
        LED_Forwards = false;
        OverWrite = false;
        break;
      case 46:                                                      //.
        if (TempDataRetrieved == 42) {
          TempDataRetrieved = 0;
        } else {
          TempDataRetrieved = 42;
        }
        Serial.println("Retrieved[3] " + String(TempDataRetrieved));
        break;
      case 49:                                                      //1
        PcEverConnected = !PcEverConnected;
        Serial.println("PcEverConnected " + String(PcEverConnected));
        break;
      case 50:                                                      //2 (numpad down)
        LED_Backwards = !LED_Backwards;
        Serial.println("LED_Backwards " + String(LED_Backwards));
        break;
      case 51:                                                      //3
        PcEverConnected = true;
        PcActivity = true;
        Serial.println("PcActivity (+PcEverConnected)" + String(PcActivity));
        break;
      case 52:                                                      //4 (numpad left)
        LED_Left = !LED_Left;
        Serial.println("LED_Left " + String(LED_Left));
        break;
      case 53:                                                      //5
        LED_Emergency = !LED_Emergency;
        Serial.println("LED_Emergency " + String(LED_Emergency));
        break;
      case 54:                                                      //6
        LED_Right = !LED_Right;
        Serial.println("LED_Right " + String(LED_Right));
        break;
      case 55:                                                      //7
        LED_SensorDebug = !LED_SensorDebug;
        Serial.println("LED_SensorDebug " + String(LED_SensorDebug));
        break;
      case 56:                                                      //8 (numpad up)
        LED_Forwards = !LED_Forwards;
        Serial.println("LED_Forwards " + String(LED_Forwards));
        break;
      case 57:                                                      //9
        OverWrite = !OverWrite;
        Serial.println("OverWrite " + String(OverWrite));
        break;
      default:
        Serial.println("Not sure" + String(Retrieved[0]));
    }
  }
  Retrieved[3] = TempDataRetrieved;
  LEDControl();
}
//this is the end, hope you had fun
