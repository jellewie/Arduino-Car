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
bool LED_Driving = false;
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
        LED_Driving = false;
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
        LED_Driving = !LED_Driving;
        Serial.println("LED_Driving " + String(LED_Driving));
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
    LEDs[0] = CRGB(map(SensorLeft,       0, 672, 0, 255), 0, 0);    //Setting values for the Sensor to the LEDs
    LEDs[1] = CRGB(map(SensorFrontLeft,  0, 672, 0, 255), 0, 0);    //^^
    LEDs[2] = CRGB(map(SensorFrontRight, 0, 672, 0, 255), 0, 0);    //^^
    LEDs[3] = CRGB(map(SensorRight,      0, 672, 0, 255), 0, 0);    //^^
    LEDs[4] = CRGB(map(SensorBack,       0, 672, 0, 255), 0, 0);    //^^
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
    UpdateLEDs = true;                                            //Update
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
