/*
  Program written by Jelle Wietsma as a school project for the self driving car


  =====TODO=====
  Test engine numbers (printline and send some values so we can see if its curving)
  Change pin numbers to match Eplan
  Change steering, we have a potmeter, and it's now a PWM engine



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
bool LED_Driving = false;
bool LED_Emergency = false;
bool PcEverConnected = false;
bool PcActivity = false;
bool LED_SensorDebug = false;
bool OverWrite = false;
int Disco = 0;


void setup() {                                                //This code runs once on start-up
  delay(1000);                                                //Just some delay to give some room for error programming
  pinMode(PWO_LED, OUTPUT);
  FastLED.addLeds<WS2812B, PWO_LED, GRB>(leds, TotalLeds);
  FastLED.setBrightness(10);                                  //Scale brightness
  fill_solid(&(leds[0]), TotalLeds, CRGB(0, 0, 0));                 //Completly reset the strip
  Serial.begin(9600);                                         //Opens serial port (to pc), sets data rate to 9600 bps
}


void loop() {                                                 //Keep looping the next code
  EVERY_N_MILLISECONDS(20) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }

  sensorVal[0] = analogRead(PAI_SensorFrontLeft);
  sensorVal[1] = analogRead(PAI_SensorFrontRight);
  sensorVal[2] = analogRead(PAI_SensorRight);
  sensorVal[3] = analogRead(PAI_SensorLeft);
  sensorVal[4] = analogRead(PAI_SensorBack);




  delay(1);                                                   //Wait some time so the Arduino has free time
}

void LEDControl() {
  //60 LEDs/M
  //0.8m x 0.6m = 2.8M omtrek (en 0.48 m² oppervlakte)
  //3M x 60LEDS/M = 180LEDs totaal * (3x20ma) = 10800ma (11A) Powerbank is 26800 dus we kunnen een paar uur op full power!
  //pin voor LED = PWO_LED
  //https://github.com/FastLED/FastLED/wiki/Pixel-reference
  //WS2812 led data takes 30µs per pixel. If you have 100 pixels, then that means interrupts will be disabled for 3000µs, or 3ms.
  //48 x 36 LEDS = 24 <Corner> 36 <> 48 <> 36 <> 24
  static int pos1;
  static int pos2;
  static int pos3;
  static int countLed1;
  static int countLed2;

  static byte TimerLED_Left;                                        //Create a timer (so we can make a delay in the animation
  if (LED_Left) {                                                   //Turning left
    TimerLED_Left ++;                                               //Add 1 to the timer
    if (TimerLED_Left > 19) {                                       //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
      TimerLED_Left = 0;                                            //Reset timer so this code will only be called after another X loops (Aka delay)
      int posLV  = 32;                                              //Value for start of front left turning light
      int posLVL = 25;                                              //Value for length of front left turning Light
      int posLB  = 136;                                             //Value for start of back left turning light
      int posLBL = 25;                                              //Value for length of back left turning light
      int calc1  = posLV - posLVL;                                      //This is calculating something! (actually a value that is used for the section that is reversed)
      fill_solid(&(leds[calc1]), posLVL,                CRGB(0, 0, 0)); //Setting the front section to black
      fill_solid(&(leds[posLV - 2]), 2,                 CRGB(255, 128, 0));  //Will set the first two LEDs at the start of the front section to on so they will always be on if this function is called
      fill_solid(&(leds[posLV - countLed2]), countLed2, CRGB(255, 128, 0));  //Will set the increasing front left section
      fill_solid(&(leds[posLB]), posLBL,    CRGB(0, 0, 0));             //Setting the back section to black
      fill_solid(&(leds[posLB]), 2,         CRGB(255, 128, 0));         //Will set the first two LEDs at the start of the back section to on so they will always be on if this function is called
      fill_solid(&(leds[posLB]), countLed1, CRGB(255, 128, 0));         //Will set the increasing back left section
      countLed1++;                                                      //This will make the front LED section length bigger
      countLed2++;                                                      //This will make the back LED section length bigger
      if ((countLed1 > posLVL) && (countLed2 > posLBL)) {               //If both sections are as big as they can go,
        countLed1 = 0;                                                  //It will reset the front counter
        countLed2 = 0;                                                  //And it will reset the back counter
      } else if (countLed1 > posLVL) {                                  //If only the front section is bigger,
        countLed1 = posLVL;                                             //It will just set the front section to maximum length
      } else if (countLed2 > posLBL) {                                  //Same here as last two lines but then for the back
        countLed2 = posLBL;                                             //It will just set the back section to maximum length
      }
      UpdateLEDs = true;                                                //Enabling the send LED data
    }
  } else {
    TimerLED_Left = 0;
  }
  static byte TimerLED_Right;                                           //Create a timer (so we can make a delay in the animation
  if (LED_Right) {                                                      //Turning right
    TimerLED_Right ++;                                                  //Add 1 to the timer
    if (TimerLED_Right > 19) {                                          //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
      TimerLED_Right = 0;                                               //Reset timer so this code will only be called after another X loops (Aka delay)
      int posRV  = 52;                                                  //Value for start of front right turning light
      int posRVL = 25;                                                  //Value for length of front right turning Light
      int posRB  = 116;                                                       //Value for start of back right turning light
      int posRBL = 25;                                                        //Value for length of back right turning light
      int calc1  = posRB - posRBL;                                            //This is calculating something AGAIN! (actually, also a value that is used for the section that is reversed)
      fill_solid(&(leds[posRV]), posRVL,    CRGB(0, 0, 0));                  //Setting the front section to black
      fill_solid(&(leds[posRV]), 2,         CRGB(255, 128, 0));              //Will set the first two LEDs at the start of the front section to on so they will always be on if this function is called
      fill_solid(&(leds[posRV]), countLed1, CRGB(255, 128, 0));              //Will set the increasing front right section
      fill_solid(&(leds[calc1]), posRBL,                CRGB(0, 0, 0));      //Setting the back section to black
      fill_solid(&(leds[posRB - 2]), 2,                 CRGB(255, 128, 0));  //Will set the first two LEDs at the start of the back section to on so they will always be on if this function is called
      fill_solid(&(leds[posRB - countLed2]), countLed2, CRGB(255, 128, 0));  //Will set the increasing back right section
      countLed1++;                                                            //This will make the front LED section length bigger
      countLed2++;                                                            //This will make the back LED section length bigger
      if ((countLed1 > posRVL) && (countLed2 > posRBL)) {                     //If both sections are as big as they can go,
        countLed1 = 0;                                                        //It will reset the front counter
        countLed2 = 0;                                                        //And it will reset the back counter
      } else if (countLed1 > posRVL) {                                        //If only the front section is bigger,
        countLed1 = posRVL;                                                   //It will just set the front section to maximum length
      } else if (countLed2 > posRBL) {                                        //Same here as last two lines but then for the back
        countLed2 = posRBL;                                                   //It will just set the back section to maximum length
      }
      UpdateLEDs = true;                                                      //Enabling the send LED data
    }
  } else {
    TimerLED_Right = 0;
  }
  static byte TimerLED_Driving;                                         //Create a timer (so we can make a delay in the animation
  if (LED_Driving) {                                                    //Drive Forwards
    TimerLED_Driving ++;                                                //Add 1 to the timer
    if (TimerLED_Driving > 19) {                                        //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
      TimerLED_Driving = 0;                                             //Reset timer so this code will only be called after another X loops (Aka delay)
      int frontMiddle = 42;                                                                                   //Middle of the front section (middle is between two so it has an offset of 0,5 of course)
      int frontLength = 10;                                                                                   //Half of the length
      int frontMiddleStatic = 1;                                                                              //half of how many LEDs will always be on
      int calc1 = frontMiddle - pos3;                                                                         //Calculation for calculating the position from where to start the fill_solid from
      fill_solid(&(leds[frontMiddle - frontLength]), (frontLength * 2), CRGB(0, 0, 0));                       //Setting the section to black
      fill_solid(&(leds[frontMiddle - (frontMiddleStatic)]), (frontMiddleStatic * 2), CRGB(0, 255, 0));       //Setting the static LEDs
      fill_solid(&(leds[calc1]), (pos3 * 2), CRGB(0, 255, 0));                                                //Setting the moving LEDs
      pos3++;                                                                                                 //increasing the position
      if (pos3 > frontLength) {                                                                               //If the section is bigger thant the maximum,
        pos3 = 0;                                                                                             //It will reset the position
      }
      UpdateLEDs = true;                                                                                      //Enabling LED data send
    }
  } else {
    TimerLED_Driving = 0;                                               //Rest timer so we will start at the beginning again if we need tha animation
  }
  static byte TimerLED_Backwards;                                       //Create a timer (so we can make a delay in the animation
  if (LED_Backwards) {                                                  //Drive Backwards (and check later if we are standing still)
    TimerLED_Backwards ++;                                              //Add 1 to the timer
    if (TimerLED_Backwards > 19) {                                      //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
      TimerLED_Backwards = 0;                                           //Reset timer so this code will only be called after another X loops (Aka delay)
      int backMiddle = 126;                                                                                   //Middle of the LED section at the back
      int backLength = 10;                                                                                    //Half of the length
      int backMiddleStatic = 1;                                                                               //Half of the length of the LEDs that will always be on
      int calc1 = backMiddle - pos2;                                                                          //Calculation for the position from which to start the fill_solid from
      fill_solid(&(leds[backMiddle - backLength]), (backLength * 2), CRGB(0, 0, 0));                         //Reseting the back strip
      fill_solid(&(leds[backMiddle - (backMiddleStatic)]), (backMiddleStatic * 2), CRGB(255, 255, 255));     //Setting the static LEDs
      fill_solid(&(leds[calc1]), (pos2 * 2), CRGB(255, 255, 255));                                           //Setting the moving LEDs
      pos2++;                                                                                                 //Increasing the position value
      if (pos2 > backLength) {                                                                                //If the position value is bigger than the maximum,
        pos2 = 0;                                                                                             //Reset it to 0
      }                                                                                                       //Ending that check
      UpdateLEDs = true;                                                                                      //This wil enable the data to be send
    }
  } else {
    TimerLED_Backwards = 0;                                             //Rest timer so we will start at the beginning again if we need tha animation
    if ((!LED_Driving)) {                                               //If not moving at all
      fill_solid( &(leds[108]), 36, CRGB(255, 0, 0));                   //Enable brake lights
      UpdateLEDs = true;                                                //And enabling LED data to be send
    }
  }
  if (LED_Emergency) {                                                  //Emergency
    int emerSSize = 10;                                                 //Length of the sections
    int emerNValue = 4;                                                 //Quantity of the sections
    int poss[emerNValue];                                               //Array for saving the positions of the sections
    int emerOffset = TotalLeds / emerNValue;                            //Calculation for calculating offset from first Position
    fill_solid( &(leds[0]), TotalLeds, CRGB(0, 0, 0));                  //Completly erasing all LED data so they wil al be off
    for (int i = 0; i < emerNValue; i++) {                              //Beginning of the loop which will send each position and length
      poss[i] = pos1 + (emerOffset * i);                                //This will calculate each position by adding the offset times the position number to the first position
      int posX;                                                         //This is the variable which will be used for sending position start
      if (poss[i] >= TotalLeds) {                                       //To see if the position is to bigger than the total amount
        posX = poss[i] - TotalLeds;                                     //If that is true then it wil subtract the total amount of leds from the position number
      } else {                                                          //Otherwise it wil just use the position data without modifying it
        posX = poss[i];                                                 //
      }
      if (posX <= (TotalLeds - emerSSize)) {                            //If the whole section ends before the total amount is reached it wil just us the normal way of setting the leds
        fill_solid( &(leds[posX]), emerSSize, CRGB(255, 0, 0));         //With the standard fill_solid command from FastLED, leds[posX] PosX stands for beginning position, emerSSize will stand for the size of the sections and the last one is the color
      } else if ((posX >= (TotalLeds - emerSSize)) && (posX <= TotalLeds)) {//This will check if the thing is beyond the total amount of leds
        int calc1 = (TotalLeds - (posX + emerSSize)) * (-1);            //Calculates the amount of LEDs which need to be set from the beginning
        int calc2 = emerSSize - calc1;                                  //Calculates the amount of LEDs which need to be set at the last so the total wil be reached but wont be bigger than the total
        fill_solid(&(leds[posX]), calc2, CRGB(255, 0, 0));              //Fills the LEDs at the beginning of the strip
        fill_solid(&(leds[0]), calc1, CRGB(255, 0, 0));                 //Fills the last LEDs at the end of the strip
      }
    }
    if (pos1 >= TotalLeds) {                                            //Will check if the main position is bigger than the total
      pos1 = 0;                                                         //If that is the case than it will reset it to 0
    } else {                                                            //Otherwise,
      pos1++;                                                           //It will just set it to 0
    }                                                                   //And end the check
    UpdateLEDs = true;                                                  //Enabling the send LED data
  }
  if (PcEverConnected) {                                                //Pc ever connected
    leds[5] = CRGB(0, 0, 255);                                          //Status indication LED if pc is connected
    UpdateLEDs = true;                                                  //Updating LED data send
  }                                                                     //Ending check
  if (PcActivity) {                                                     //Pc activity
    PcActivity = false;                                                 //Reseting PC activity so it can be set again if it has activity
    leds[5] = CRGB(0, 255, 0);                                          //Making status indication LED different color
    UpdateLEDs = true;                                                  //Updating Update value for updating data for the LEDs so they will update at the end of this loop
  }                                                                     //Ending
  if (LED_SensorDebug) {                                                //Sensor debug through status indication LEDs (brightness change at the moment)
    for (int i = 0; i < 5; i++) {                                       //Loop for going through the different sensor values
      leds[i] = CRGB(sensorVal[i], 0, 0);                               //Setting values for each Sensor to the LEDs
    }                                                                   //And ending this again
    UpdateLEDs = true;                                                  //Updating the update update updater updating stuff something
  }                                                                     //And ending again
  if (OverWrite) {                                                      //If the Program is overwritten by an pc (so manual control)
    for (int i = 0; i < (TotalLeds / 5); i++) {                         //At the moment this is a program that will mark al locations of corners and with this enabled it will be easier to measure different parts of the strip
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
    UpdateLEDs = true;                                                  //Updating the LEDs
  }                                                                     //Ending
  if (Disco == 42) {                                                    //If we need an disco (42 = '*') needs to be written someday, is not very important...
  }
  if (UpdateLEDs) {                                                     //If we need an update
    Serial.println("a");
    leds[167] = CRGB(0, 255, 0);        //Temp! TODO FIXME
    FastLED.show();                                                     //Apply LED changes
    UpdateLEDs = false;                                                 //Flag update done
  }
}
