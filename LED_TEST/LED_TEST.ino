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
const int PWO_LED = 3;

//Just some configuable things
const int DelayLoop = 15;                                     //The amount of times to wait (arduinoloop)ms [15*1=15ms = 66Hz]

const int TotalLeds = 168;                                    //The total amount of LEDS in the strip
CRGB leds[TotalLeds];                                         //This is an array of leds.  One item for each led in your strip.
uint8_t gHue = 1;                                             // rotating "base color" used by many of the patterns
bool UpdateLEDs = true;
float thistimer;


const int PAI_SensorFrontLeft = A0;
const int PAI_SensorFrontRight = A1;
const int PAI_SensorRight = A2;
const int PAI_SensorLeft = A3;
const int PAI_SensorBack = A4;

int sensorVal[5];

bool LED_Backwards = true;
bool LED_Left = true;
bool LED_Right = true;
bool LED_Driving = true;
bool LED_Emergency = false;
bool PcEverConnected = false;
bool PcActivity = false;
bool LED_SensorDebug = true;
bool OverWrite = false;
byte Retrieved[3] = {0,0,5};

byte SensorFrontLeft = 0;
byte SensorFrontRight = 60;
byte SensorRight = 120;
byte SensorLeft = 180;
byte SensorBack = 255;

void setup() {                                                      //This code runs once on start-up
  delay(1000);                                                      //Just some delay to give some room for error programming
  pinMode(PWO_LED, OUTPUT);
  FastLED.addLeds<WS2812B, PWO_LED, GRB>(leds, TotalLeds);
  FastLED.setBrightness(10);                                        //Scale brightness
  fill_solid(&(leds[0]), TotalLeds, CRGB(0, 0, 0));                 //Completly reset the strip
  Serial.begin(9600);                                               //Opens serial port (to pc), sets data rate to 9600 bps
  Serial.println("Booted");
}

void loop() {                                                       //Keep looping the next code
  Serial.println("L");
  sensorVal[0] = analogRead(PAI_SensorFrontLeft);
  sensorVal[1] = analogRead(PAI_SensorFrontRight);
  sensorVal[2] = analogRead(PAI_SensorRight);
  sensorVal[3] = analogRead(PAI_SensorLeft);
  sensorVal[4] = analogRead(PAI_SensorBack);
  LEDControl();
  delay(10);                                                        //Wait some time so the Arduino has free time
}

void LEDControl() {
  //60 LEDs/M
  //0.8m x 0.6m = 2.8M omtrek (en 0.48 m² oppervlakte)
  //3M x 60LEDS/M = 180LEDs totaal * (3x20ma) = 10800ma (11A) Powerbank is 26800 dus we kunnen een paar uur op full power!
  //pin voor LED = PWO_LED
  //https://github.com/FastLED/FastLED/wiki/Pixel-reference
  //WS2812 led data takes 30µs per pixel. If you have 100 pixels, then that means interrupts will be disabled for 3000µs, or 3ms.
  //48 x 36 LEDS = 24 <Corner> 36 <> 48 <> 36 <> 24
  static int CounterEmergency;
  static byte CounterBack;
  static byte CounterFront;
  static byte CounterLeft;
  static byte CounterRight;

  static byte TimerLED_Left;                                        //Create a timer (so we can make a delay in the animation
  static byte TimerLED_Right;                                       //Create a timer (so we can make a delay in the animation
  static byte TimerLED_Driving;                                     //Create a timer (so we can make a delay in the animation
  static byte TimerLED_Backwards;                                   //Create a timer (so we can make a delay in the animation
  byte DelayAnimationDriving = 30;                                  //Delay in ms for the animation (excluding the write time delay!)
  byte DelayanimationBlink = 30;                                    //Delay in ms for the animation (excluding the write time delay!)
  if (LED_Left) {                                                   //Turning left
    TimerLED_Left ++;                                               //Add 1 to the timer
    if (TimerLED_Left >= DelayanimationBlink) {                     //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
      TimerLED_Left = 0;                                            //Reset timer so this code will only be called after another X loops (Aka delay)
      byte PositionLeftFront = 32;                                  //Start of front left turning light
      byte PositionLeftBack = 136;                                  //Start of back left turning light
      byte LeftLength = 25;                                         //Total length
      fill_solid(&(leds[PositionLeftFront - LeftLength]), LeftLength,  CRGB(0, 0, 0));     //Turn front off
      fill_solid(&(leds[PositionLeftFront - 2]), 2,                    CRGB(255, 128, 0)); //Will set the first two LEDs at the start of the front section to on so they will always be on if this function is called
      fill_solid(&(leds[PositionLeftFront - CounterLeft]), CounterLeft, CRGB(255, 128, 0)); //Will set the increasing front left section
      fill_solid(&(leds[PositionLeftBack]), LeftLength,                CRGB(0, 0, 0));     //Turn back off
      fill_solid(&(leds[PositionLeftBack]), 2,                         CRGB(255, 128, 0)); //Will set the first two LEDs at the start of the back section to on so they will always be on if this function is called
      fill_solid(&(leds[PositionLeftBack]), CounterLeft,               CRGB(255, 128, 0)); //Will set the increasing back left section
      CounterLeft++;                                                //This will make the front LED section length bigger
      if ((CounterLeft > LeftLength)) {                             //If we are at max lenth
        CounterLeft = 0;                                            //Reset counter
      }
      UpdateLEDs = true;                                            //The leds need an update
    }
  } else {
    TimerLED_Left = 0;
  }
  if (LED_Right) {                                                  //Turning right
    TimerLED_Right ++;                                              //Add 1 to the timer
    if (TimerLED_Right >= DelayanimationBlink) {                    //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
      TimerLED_Right = 0;                                           //Reset timer so this code will only be called after another X loops (Aka delay)
      byte PositionRightFront = 52;                                 //Start of front right turning light
      byte PositionRightBack = 116;                                 //Start of back right turning light
      byte RightLength = 25;                                        //Total length
      fill_solid(&(leds[PositionRightFront]), RightLength,                CRGB(0, 0, 0));     //Setting the front section to black
      fill_solid(&(leds[PositionRightFront]), 2,                          CRGB(255, 128, 0)); //Will set the first two LEDs at the start of the front section to on so they will always be on if this function is called
      fill_solid(&(leds[PositionRightFront]), CounterRight,               CRGB(255, 128, 0)); //Will set the increasing front right section
      fill_solid(&(leds[PositionRightBack - RightLength]), RightLength,   CRGB(0, 0, 0));     //Setting the back section to black
      fill_solid(&(leds[PositionRightBack - 2]), 2,                       CRGB(255, 128, 0)); //Will set the first two LEDs at the start of the back section to on so they will always be on if this function is called
      fill_solid(&(leds[PositionRightBack - CounterRight]), CounterRight, CRGB(255, 128, 0)); //Will set the increasing back right section
      CounterRight++;                                               //This will make the back LED section length bigger
      if ((CounterRight > RightLength)) {                           //If both sections are as big as they can go,
        CounterRight = 0;                                           //And it will reset the back counter
      }
      UpdateLEDs = true;                                            //The leds need an update
    }
  } else {
    TimerLED_Right = 0;
  }
  if (LED_Driving) {                                                //Drive Forwards
    TimerLED_Driving ++;                                            //Add 1 to the timer
    if (TimerLED_Driving >= DelayAnimationDriving) {                //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
      TimerLED_Driving = 0;                                         //Reset timer so this code will only be called after another X loops (Aka delay)
      byte PositionFrontMiddle = 42;                                //Middle of the front section (middle is between two so it has an offset of 0,5 of course)
      byte FrontLength = 10;                                        //Half of the length
      byte PositionFrontMiddleStatic = 1;                           //half of how many LEDs will always be on
      fill_solid(&(leds[PositionFrontMiddle - FrontLength]), (FrontLength * 2),                               CRGB(0, 0, 0));   //Setting the section to black
      fill_solid(&(leds[PositionFrontMiddle - (PositionFrontMiddleStatic)]), (PositionFrontMiddleStatic * 2), CRGB(0, 255, 0)); //Setting the static LEDs
      fill_solid(&(leds[PositionFrontMiddle - CounterFront]), (CounterFront * 2),                             CRGB(0, 255, 0)); //Setting the moving LEDs
      CounterFront++;                                               //increasing the position
      if (CounterFront > FrontLength) {                             //If the section is bigger thant the maximum,
        CounterFront = 0;                                           //It will reset the position
      }
      UpdateLEDs = true;                                            //The leds need an update
    }
  } else {
    TimerLED_Driving = 0;                                           //Rest timer so we will start at the beginning again if we need tha animation
  }
  if (LED_Backwards) {                                              //Drive Backwards (and check later if we are standing still)
    TimerLED_Backwards ++;                                          //Add 1 to the timer
    if (TimerLED_Backwards >= DelayAnimationDriving) {              //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
      TimerLED_Backwards = 0;                                       //Reset timer so this code will only be called after another X loops (Aka delay)
      byte BackMiddle = 126;                                        //Middle of the LED section at the back
      byte BackLength = 10;                                         //Half of the length
      byte BackMiddleStatic = 1;                                    //Half of the length of the LEDs that will always be on
      fill_solid(&(leds[BackMiddle - BackLength]), (BackLength * 2),                CRGB(0, 0, 0));       //Reseting the back strip
      fill_solid(&(leds[BackMiddle - (BackMiddleStatic)]), (BackMiddleStatic * 2),  CRGB(255, 255, 255)); //Setting the static LEDs
      fill_solid(&(leds[BackMiddle - CounterBack]), (CounterBack * 2),              CRGB(255, 255, 255)); //Setting the moving LEDs
      CounterBack++;                                                //Increasing the position value
      if (CounterBack > BackLength) {                               //If the position value is bigger than the maximum,
        CounterBack = 0;                                            //Reset it to 0
      }                                                             //Ending that check
      UpdateLEDs = true;                                            //This wil enable the data to be send
    }
  } else {
    TimerLED_Backwards = 0;                                         //Rest timer so we will start at the beginning again if we need tha animation
    if (!LED_Driving) {                                             //If not moving at all
      fill_solid(&(leds[108]), 36, CRGB(255, 0, 0));                //Enable brake lights
      UpdateLEDs = true;                                            //And enabling LED data to be send
    }
  }
  if (LED_Emergency) {                                              //Emergency
    byte EmergencySize = 10;                                        //Length of the sections
    byte EmergencyAmount = 4;                                       //Quantity of the sections
    int EmergencyOffset = TotalLeds / EmergencyAmount;              //Calculation for calculating offset from first Position
    int poss[EmergencyAmount];                                      //Array for saving the positions of the sections
    fill_solid(&(leds[0]), TotalLeds,             CRGB(0, 0, 0));   //Completly erasing all LED data so they wil al be off (this will overwrite all other programs)
    for (int i = 0; i < EmergencyAmount; i++) {                     //Beginning of the loop which will send each position and length
      poss[i] = CounterEmergency + (EmergencyOffset * i);           //This will calculate each position by adding the offset times the position number to the first position
      int posX;                                                     //This is the variable which will be used for sending position start
      if (poss[i] >= TotalLeds) {                                   //To see if the position is to bigger than the total amount
        posX = poss[i] - TotalLeds;                                 //If that is true then it wil subtract the total amount of leds from the position number
      } else {                                                      //Otherwise it wil just use the position data without modifying it
        posX = poss[i];                                             //
      }
      if (posX <= (TotalLeds - EmergencySize)) {                    //If the whole section ends before the total amount is reached it wil just us the normal way of setting the leds
        fill_solid( &(leds[posX]), EmergencySize, CRGB(255, 0, 0)); //With the standard fill_solid command from FastLED, leds[posX] PosX stands for beginning position, EmergencySize will stand for the size of the sections and the last one is the color
      } else if ((posX >= (TotalLeds - EmergencySize)) && (posX <= TotalLeds)) {//This will check if the thing is beyond the total amount of leds
        int calc1 = (TotalLeds - (posX + EmergencySize)) * -1;      //Calculates the amount of LEDs which need to be set from the beginning
        int calc2 = EmergencySize - calc1;                          //Calculates the amount of LEDs which need to be set at the last so the total wil be reached but wont be bigger than the total
        fill_solid(&(leds[posX]), calc2,          CRGB(255, 0, 0)); //Fills the LEDs at the beginning of the strip
        fill_solid(&(leds[0]), calc1,             CRGB(255, 0, 0)); //Fills the last LEDs at the end of the strip
      }
    }
    if (CounterEmergency >= TotalLeds) {                            //Will check if the main position is bigger than the total
      CounterEmergency = 0;                                         //If that is the case than it will reset it to 0
    } else {                                                        //Otherwise,
      CounterEmergency++;                                                       //It will just set it to 0
    }                                                               //And end the check
    UpdateLEDs = true;                                              //Enabling the send LED data
  }
  if (PcEverConnected) {                                            //Pc ever connected
    leds[5] = CRGB(0, 0, 255);                                      //Status indication LED if pc is connected
    UpdateLEDs = true;                                              //Updating LED data send
  }                                                                 //Ending check
  if (PcActivity) {                                                 //Pc activity
    PcActivity = false;                                             //Reseting PC activity so it can be set again if it has activity
    leds[5] = CRGB(0, 255, 0);                                      //Making status indication LED different color
    UpdateLEDs = true;                                              //Updating Update value for updating data for the LEDs so they will update at the end of this loop
  }                                                                 //Ending
  if (LED_SensorDebug) {                                            //Sensor debug through status indication LEDs (brightness change at the moment)
    leds[1] = CRGB(SensorLeft,       0, 0);                         //Setting values for the Sensor to the LEDs
    leds[2] = CRGB(SensorFrontLeft,  0, 0);                         //^^
    leds[3] = CRGB(SensorFrontRight, 0, 0);                         //^^
    leds[4] = CRGB(SensorRight,      0, 0);                         //^^
    leds[5] = CRGB(SensorBack,       0, 0);                         //^^
    UpdateLEDs = true;                                              //Updating the update update updater updating stuff something
  }                                                                 //And ending again
  if (OverWrite) {                                                  //If the Program is overwritten by an pc (so manual control)
    for (int i = 0; i < (TotalLeds / 5); i++) {                     //At the moment this is a program that will mark al locations of corners and with this enabled it will be easier to measure different parts of the strip [TODO FIXME LOW]
      int vogels1 = i * 5;
      if (vogels1 < TotalLeds) {
        leds[vogels1 - 1] = CRGB(0, 0, 255);
      }
    }
    for (int i = 0; i < (TotalLeds / 20); i++) {
      int vogels2 = i * 20;
      if (vogels2 < TotalLeds) {
        leds[vogels2 - 1] = CRGB(255, 0, 0);
      }
    }
    fill_solid(&(leds[23]), 2, CRGB(0, 255, 0));
    fill_solid(&(leds[59]), 2, CRGB(0, 255, 0));
    fill_solid(&(leds[107]), 2, CRGB(0, 255, 0));
    fill_solid(&(leds[143]), 2, CRGB(0, 255, 0));
    fill_solid(&(leds[167]), 2, CRGB(0, 255, 0));
    leds[167] = CRGB(0, 255, 0);
    UpdateLEDs = true;                                              //Updating the LEDs
  }                                                                 //Ending
  if (Retrieved[3] == 42) {                                                //If we need an Retrieved[3] (42 = '*') needs to be written someday, is not very important...  [TODO FIXME LOW]
  }
  if (UpdateLEDs) {                                                 //If we need an update
    FastLED.show();                                                 //Apply LED changes
    UpdateLEDs = false;                                             //Flag update done
  }
}
