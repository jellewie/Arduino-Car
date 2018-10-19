//LED test doesnt work here








/*
  Program written by JelleWho as a school project for the self driving car
*/
#include "FastLED/FastLED.h"                                        //Include the FastLED library to control the LEDs in a simple fashion
#include "interrupt.h"                                              //Include the interrupt file so we can use interrupts
//Progam includes all .INO files into this one, so we include "LEDControl.INO" automaticly

//static const (read only) █ <variable type> https://www.arduino.cc/reference/en/#variables █ <Pin ║ Digital, Analog, pWn ║ Input, Output║ name>
static const byte PWO_LED = 5;                                      //Where the <LED strip> is connected to
static const byte PWO_Motor = 6;                                    //Frequency controller motor relay
static const byte PWO_Steering = 7;                                 //Frequency controller steering relay
static const byte PDO_LEDBlink = 13;                                //LED that’s blinks each loop relay
static const byte PDO_MotorOnOff = 23;                    //K1      //Steering on/off relay
static const byte PDO_MotorReversePoles = 25;             //K2      //Reverse polarity motor relay
static const byte PDO_MotorBrakeAnchor = 27;              //K3      //Motor brake anchor relay (short the motor)
static const byte PDO_SteeringOnOff = 29;                 //K4      //Steering on/off relay
static const byte PDO_SteeringReversePoles = 31;          //K5      //Reverse polarity steering relay
//static const byte PDO_SpareRelay = 33;                  //K6      //(no pinMode set)
static const byte PDO_MotorReversePoles2 = 35;            //K7      //Reverse polarity motor relay
static const byte PDO_SteeringReversePoles2 = 37;         //K8      //Reverse polarity steering relay
static const byte PDO_Emergency = 53;                               //Emergency button feedback
static const byte PAI_SensorFrontLeft = 0;
static const byte PAI_SensorFrontRight = 1;
static const byte PAI_SensorRight = 2;
static const byte PAI_SensorBack = 3;
static const byte PAI_SensorLeft = 4;
static const byte PAI_SensorPotmeterStuur = 5;

//Just some configurable things
static const int DelayPole = 50;                                    //The delay after/for the reverse-ment of poles (reverse engine moment)
static const int DelayAncher = 10;                                  //The delay after/for the anchor is turned on
static const int MaxValuePWM = 255 / 2;                             //Max number we can send to the Engine frequency generator
static const int TotalLEDs = (48 + 36) * 2;                         //The total amount of LEDS in the strip
static const byte SteeringMinimum = 25;                             //Below this diffrence we won't steer
static const unsigned int AnimationTimeEmergency = 1000;            //Delay in ms for the animation of the reinitialize of the program (Emergency has been lifted)
static const unsigned int AnimationTimeBooting = 2000;              //Delay in ms for the animation on start

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




int Step;
String MSG;
bool MSGShown = false;

int MSGValue;
int MSGValueOLD;

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
  //attachInterrupt(digitalPinToInterrupt(PDO_Emergency), EmergencyPressed, FALLING); //If the emergency button is pressed, turn motor off (this is checked 16.000.000 times / second or so
  Serial.println("[!E0]");                                          //Send a 'we did not understand' so the PC will know we are here and see them
  Serial.setTimeout(2);                                             //Set the timeout time of data read (ms)
  PcEverConnected = true;                                           //TEMP TODO We can remove this line, so it will default to not sending data (will take less CPU)
  MSG = " ";
}

void loop() {
  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      Serial.read(); //Remove all send data
    }
    Step++;                           //Goto next step
    Serial.println("Next step") + Step;
    MSG = " ";
  }
  //==============================================
  //ANALOG INPUTS (Sensors)
  //==============================================
  if (Step == 0) {                                                          //Next test
    MSGValue = digitalRead(PDO_Emergency);
    if (MSG == " ") {
      MSG = "Button state Emergency: ";
      Serial.println("Press and unpress the button");
    }
  } else if (Step == 1) {                                                   //Next test
    MSGValue = analogRead(PAI_SensorFrontLeft);
    if (MSG == " ") {
      MSG = "Sensor Front Left state:";
      Serial.println("Move infront of the sensor");
    }
  } else if (Step == 2) {                                                   //Next test
    MSGValue = analogRead(PAI_SensorFrontRight);
    if (MSG == " ") {
      MSG = "Sensor Front Right state:";
      Serial.println("Move infront of the sensor");
    }
  } else if (Step == 3) {                                                   //Next test
    MSGValue = analogRead(PAI_SensorRight);
    if (MSG == " ") {
      MSG = "Sensor Righ state:";
      Serial.println("Move infront of the sensor");
    }
  } else if (Step == 4) {                                                   //Next test
    MSGValue = analogRead(PAI_SensorBack);
    if (MSG == " ") {
      MSG = "Sensor Back state:";
      Serial.println("Move infront of the sensor");
    }
  } else if (Step == 5) {                                                   //Next test
    MSGValue = analogRead(PAI_SensorLeft);
    if (MSG == " ") {
      MSG = "Sensor Left state:";
      Serial.println("Move infront of the sensor");
    }
  } else if (Step == 6) {                                                   //Next test
    MSGValue = analogRead(PAI_SensorPotmeterStuur);
    if (MSG == " ") {
      MSG = "Stuur Sensor PotmeterStuur state:";
      Serial.println("Move infront of the sensor");
    }
    //==============================================
    //RELAYS
    //==============================================
    analogWrite(PWO_Motor, 0);
    analogWrite(PWO_Steering, 0);
  } else if (Step == 7) {                                                   //Next test
    digitalWrite(PDO_MotorOnOff, !digitalRead(PDO_MotorOnOff));
    if (MSG == " ") {
      MSG = "K1 blink - MotorOnOff (Turns on K9 [if !Emergency])";
      Serial.println("Check if relai(s) are turning on/off");
    }
  } else if (Step == 8) {                                                   //Next test
    digitalWrite(PDO_MotorOnOff, LOW);
    digitalWrite(PDO_MotorReversePoles, !digitalRead(PDO_MotorReversePoles));
    if (MSG == " ") {
      MSG = "K2 blink - MotorReversePoles";
      Serial.println("Check if relai(s) are turning on/off");
    }
  } else if (Step == 9) {                                                   //Next test
    digitalWrite(PDO_MotorReversePoles, LOW);
    digitalWrite(PDO_MotorBrakeAnchor, !digitalRead(PDO_MotorBrakeAnchor));
    if (MSG == " ") {
      MSG = "K3 blink - MotorBrakeAnchor";
      Serial.println("Check if relai(s) are turning on/off");
    }
  } else if (Step == 10) {                                                   //Next test
    digitalWrite(PDO_MotorBrakeAnchor, LOW);
    digitalWrite(PDO_SteeringOnOff, !digitalRead(PDO_SteeringOnOff));
    if (MSG == " ") {
      MSG = "K4 blink - SteeringOnOff (Turns on K10)";
      Serial.println("Check if relai(s) are turning on/off");
    }
  } else if (Step == 11) {                                                   //Next test
    digitalWrite(PDO_SteeringOnOff, LOW);
    digitalWrite(PDO_SteeringReversePoles, !digitalRead(PDO_SteeringReversePoles));
    if (MSG == " ") {
      MSG = "K5 blink - SteeringReversePoles";
      Serial.println("Check if relai(s) are turning on/off");
    }
    //  }  else if (Step == AA) {                                                   //Next test
    //    digitalWrite(AAAA, LOW)
    //    digitalWrite(PDO_SpareRelay, !digitalRead(PDO_SpareRelay));
    //    if (MSG == " ") {
    //      MSG = "K6 blink - ?";
    //    }
  } else if (Step == 12) {                                                   //Next test
    digitalWrite(PDO_SteeringReversePoles, LOW);
    digitalWrite(PDO_MotorReversePoles2, !digitalRead(PDO_MotorReversePoles2));
    if (MSG == " ") {
      MSG = "K7 blink - MotorReversePoles2";
      Serial.println("Check if relai(s) are turning on/off");
    }
  } else if (Step == 13) {                                                   //Next test
    digitalWrite(PDO_MotorReversePoles2, LOW);
    digitalWrite(PDO_SteeringReversePoles2, !digitalRead(PDO_SteeringReversePoles2));
    if (MSG == " ") {
      MSG = "K8 blink - MotorReversePoles2";
      Serial.println("Check if relai(s) are turning on/off");
    }
  }
  //==============================================
  //PWM
  //==============================================
  else if (Step == 14) {                                                   //Next test
    analogWrite(PWO_Motor, 64);
    if (MSG == " ") {
      MSG = "PWM Main engine 64/255";
      Serial.println("Measure the PWM signal with an osciloscoop");
    }
  }
  else if (Step == 15) {                                                   //Next test
    analogWrite(PWO_Motor, 0);
    analogWrite(PWO_Steering, 64);
    if (MSG == " ") {
      MSG = "PWM Steering engine 64/255";
      Serial.println("Measure the PWM signal with an osciloscoop");
    }
  }
  //==============================================
  //LEDS
  //==============================================
  else if (Step == 16) {                                                   //Next test
    analogWrite(PWO_Steering, 0);
    LED_Emergency = true;
    if (MSG == " ") {
      MSG = "LED Fucntion Emergency";
      Serial.println("Check if the LEDs are correct");
    }
  } else if (Step == 17) {                                                   //Next test
    LED_Emergency = false;
    LED_Forwards = true;
    if (MSG == " ") {
      MSG = "LED Fucntion Forwards";
      Serial.println("Check if the LEDs are correct");
    }
  } else if (Step == 18) {                                                   //Next test
    LED_Forwards = false;
    LED_Right = true;
    if (MSG == " ") {
      MSG = "LED Fucntion Right";
      Serial.println("Check if the LEDs are correct");
    }
  } else if (Step == 19) {                                                   //Next test
    LED_Right = false;
    LED_Left = true;
    if (MSG == " ") {
      MSG = "LED Fucntion Left";
      Serial.println("Check if the LEDs are correct");
    }
  } else if (Step == 20) {                                                   //Next test
    LED_Left = false;
    LED_Backwards = true;
    if (MSG == " ") {
      MSG = "LED Fucntion Backwards";
      Serial.println("Check if the LEDs are correct");
    }
  } else if (Step == 21) {                                                   //Next test
    LED_Left = false;
    LED_SensorDebug = true;
    PcEverConnected = true;
    if (MSG == " ") {
      MSG = "LED Fucntion Debug";
      Serial.println("Check if the LEDs are correct");
    }
  } else if (Step == 21) {                                                   //Next test
    LED_SensorDebug = false;
    PcEverConnected = false;
    if (MSG == " ") {
      MSG = "DONE!";
    }
  }

  if (MSG == ""){} else {
    Serial.println(MSG);
    MSG = "";
  }
  if (MSGValue != MSGValueOLD) {
    Serial.println(MSGValue);
    MSGValueOLD = MSGValue;
  }
  
  delay(500);
  FastLED.show();
}//this is the end, hope you had fun
