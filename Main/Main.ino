/*
  Program written by Jelle Wietsma as a school project for the self driving car


  =====TODO=====
  Test engine numbers (printline and send some values so we can see if its curving)
  Change pin numbers to match Eplan
  Change steering, we have a potmeter, and it's now a PWM engine



*/
#include "FastLED/FastLED.h"
#include "interrupt.h"

//constants won't change. P =Pin. D=digital, A=analog, W=PWM. I=Input, O=Output
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
const int DelayLoop = 15;                                           //The amount of times to wait (arduinoloop)ms [15*1=15ms = 66Hz]

//Just some numbers we need to transfer around
byte SensorFrontLeft = 0;                                           //The consitence value of the sensor though the loop
byte SensorFrontRight = 0;                                          //^^
byte SensorRight = 0;                                               //^^
byte SensorLeft = 0;                                                //^^
byte SensorBack = 0;                                                //^^
bool Emergency = 0;                                          //The emergency button state
int LoopCounter = 0;                                                //After this many Arduino loops, loop the main code (delay without delay)
int MSGLoopCounter = 0;                                             //After this many Arduino loops, loop the msg code (delay without delay)
int Engine = 0;                                                     //Engine status currently
byte EngineFrom = 0;                                                //Engine status start position
byte EngineGoTo = 0;                                                //Engine status stop position
const int MaxValuePWM = 255;                                        //Max number we can send to the Engine frequency generator
int EngineGoInSteps = 0;                                            //The amount to steps to execute the move in
int EngineCurrentStep = 0;                                          //The current step we are in
int Rotation = 0;                                                   //The rotation where we are
int RotationSpeed = 0;                                              //
int RotationGoTo = 45;                                              //Where to Rotate to
byte ReadIncomming_1 = 0;                                           //This is th emergency state of the PC(start up with the thought that we are on fire)
byte Disco = 0;                                                     //If the disco is open or closed
String LastPCStateRotation = "";                                    //Last steering position sended to the PC, so this is what the PC knows
String LastPCStateEngine = "";                                      //Last engine position sended to the PC, so this is what the PC knows
bool OverWrite = false;                                             //If we need to overwrite the self thinking
bool PcEverConnected = false;                                       //If we ever found the PC (and need to send the messages)

const int TotalLeds = 100;                                          //The total amount of LEDS in the strip
CRGB leds[TotalLeds];                                               //This is an array of leds.  One item for each led in your strip.
bool LED_Backwards = false;                                         //If the backwards LEDS needs to be on (if not it forwards)
bool LED_Left = false;                                              //If the left LEDS needs to be on
bool LED_Right = false;                                             //If the right LEDS needs to be on
bool LED_Driving = false;                                           //If the driving LEDS needs to be on (if not we are braking)
bool LED_Emergency = false;                                         //If the emergency LEDS needs to be on
uint8_t gHue = 0;                                                   //Rotating "base color" used by many of the patterns
bool UpdateLEDs = true;                                             //If the LED color needs to be updated
bool PcActivity = false;                                            //If we have pc com activity
bool LED_SensorDebug = false;                                       //If we need to debug the sensors (show sensor data in the LEDs
//Jelle's head
int Z = 0;
const byte SensorLowLimit = 100;                                    //A
const byte MiniumDifference = 5;                                    //B
const byte MiniumStepsBackwards = 100;                              //C
const float DividerSteering = 10;                                   //D
//end jelle's head

//new led values----------------------------------------------
int sensorVal[5];
//--------------------------------------------------------



void setup() {                                                      //This code runs once on start-up
  delay(1000);                                                      //Just some delay to give some room for error programming
  pinMode(PDI_SensorLeft, INPUT);                                   //Sometimes the Arduino needs to know what pins are OUTPUT and what are INPUT, since it could get confused and create an error. So it's set manual here
  pinMode(PDI_SensorBack, INPUT);
  pinMode(PDI_SensorRight, INPUT);
  pinMode(PWO_LED, OUTPUT);
  pinMode(PDO_Emergency, INPUT_PULLUP);
  pinMode(PDO_Steering_Read, INPUT);
  pinMode(PDO_Steering, OUTPUT);
  pinMode(PDO_MotorBrake, OUTPUT);
  pinMode(PDO_MotorReversePoles, OUTPUT);
  pinMode(PDO_Motor, OUTPUT);
  pinMode(PDO_LEDBlink, OUTPUT);
  pinMode(PAI_SensorFrontLeft, OUTPUT);
  pinMode(PAI_SensorFrontRight, OUTPUT);
  FastLED.addLeds<WS2812B, PWO_LED, GRB>(leds, TotalLeds);
  FastLED.setBrightness(100);                                       //Scale brightness
  fill_solid(&(leds[0]), TotalLeds, CRGB(0, 0, 0));                 //Set the whole LED strip to be off
  FastLED.show();                                                   //Update the LEDs
  Serial.begin(9600);                                               //Opens serial port (to pc), sets data rate to 9600 bps
  ReadIncomming_1 = 126;                                            //Fake the emergency button from the PC, (just once on boot so when you connect the PC the PC takes this over)
  digitalWrite(PDO_LEDBlink, HIGH);                                 //Let the led blink so we know the program has started
  attachInterrupt(digitalPinToInterrupt(PDO_Emergency), EmergencyReleased, FALLING);  //If the emergency button is pressed, turn motor off (this is checked 16.000.000 times / second or so
}

void loop() {                                                       //Keep looping the next cod
  LoopCounter--;                                                    //Remove one from the LoopCounter
  if (LoopCounter <= 0) {                                           //If we need an update (loops every 'DelayLoop' in time, so like every 60 arduino loops (1ms/a peace)
    LoopCounter = DelayLoop;                                        //Reset LoopCounter
    digitalWrite(PDO_LEDBlink, !digitalRead(PDO_LEDBlink));         //Let the led blink so we know the program is running
    Emergency = digitalRead(PDO_Emergency);                         //Get emergency button state (we save this so this state is contstand in this loop)
    SensorFrontLeft = analogRead(PAI_SensorFrontLeft);              //Get the sensor data (so ir would be consistance though this loop
    SensorFrontRight = analogRead(PAI_SensorFrontRight);            //^^
    SensorRight = analogRead(PAI_SensorRight);                      //^^
    SensorLeft =  analogRead(PAI_SensorLeft);                       //^^
    SensorBack = analogRead(PAI_SensorBack);                        //^^
    if (Serial.available() > 0) {                                   //https://www.arduino.cc/en/Reference/ASCIIchart to see the asci chart to know what numbers are what
      PcActivity = true;
      PcEverConnected = true;                                       //We have found an PC, so give feedback about states from now on
      ReadIncomming_1 = Serial.read();                              //Get Emergency info (1 = it's fine, !1 WE ARE ON FIRE!)
      delay(1);                                                     //Some delay so we are sure we retrieved the data
      int ReadIncomming_2 = Serial.read();                          //Read next data
      delay(1);                                                     //Some delay so we are sure we retrieved the data
      int ReadIncomming_3 = Serial.parseInt();                      //Read next data (its an int)
      Disco = Serial.read();                                        //Just useless, but tells the end of the int
      //Serial.println("1=" + String(ReadIncomming_1) + "." + "\n" + "2=" + String(ReadIncomming_2) + "." + "\n" + "3=" + String(ReadIncomming_3) + ".");//This line is just for debug, to let the Arduino tells us what it recieved
      if (ReadIncomming_2 == 82) {                                  //If "R" retrieved (rotate)
        OverWrite = true;                                           //Set the OverWrite to true to OverWrite the thinking of the Arduino
        RotationGoTo = ReadIncomming_3;                             //Set where to rotate to
      } else if (ReadIncomming_2 == 77) {                           //If "M" retrieved (Motor)
        OverWrite = true;                                           //Set the OverWrite to true to OverWrite the thinking of the Arduino
        EngineGoTo = ReadIncomming_3;                               //Set the EngineGoTo state
      }
      if (ReadIncomming_3 == 68) {                                  //If "D" retrieved (Debig)
        LED_SensorDebug = !LED_SensorDebug;                         //Toggle Debug mode
      }
      LastPCStateEngine = "";                                       //Fuck up LastPCStateEngine so it will resend it
      LastPCStateRotation = "";                                     //Fuck up LastPCStateRotation so it will resend it
      //We can add more code here to retrieve more commands from the pc
    }
    if (ReadIncomming_1 != 126 || Emergency == 0) {                 //Are we on fire? (1 = it's fine, 0 = WE ARE ON FIRE!)
      //CALL THE FIREDEPARMENT AND STOP WHAT WE ARE DOING WE ARE ON FIRE!
      LED_Emergency = true;                                         //Set the emergency LED on
      RotationGoTo = Rotation;                                      //Stop with rotating, keep where ever it is at this moment
      EngineGoTo = 0;                                               //Set EngineGoTo to 0
    } else {
      LED_Emergency = false;                                        //Set the emergency LED off
      if (OverWrite) {                                              //If the Arduino doesn't need to think (user input overwrite)
        //OverWrite = false;                                        //reset OverWrite state (disabled so it's player input only from now on)
      } else {
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
        //Use 'RotationGoTo = X;'  to rotate to X

        //Rotation will happen in increasements of 1 step per loop
        //Engine will be set directly (with appropriate delay)
        HeadJelle();                                                //Call jelle's head code (change if we want to use a diffrent head
        //--------------------End Head--------------------
      }
    }
    //--------------------Engine control--------------------
    LED_Backwards = false;                                          //Set engine backwards LED to be off (will be turned on before it would notice it if it needs to be on)
    if (EngineGoTo == 0) {                                          //If EngineGoTo is off
      LED_Driving = false;                                          //Set engine LED to be off
      if (Engine != EngineGoTo) {                                   //If we are not yet updated
        digitalWrite(PDO_Motor, LOW);                               //Turn engine off
        delay(DelayAncher);                                         //Wait some time to make sure engine is off
        Engine = EngineGoTo;                                        //Update the engine state
        EngineFrom = Engine;                                        //Set current engine state to be the new state to start engine fom
      }
      digitalWrite(PDO_MotorBrake, HIGH);                           //Brake
    } else {
      LED_Driving = true;                                           //Set engine LED to be on
      if (Engine != EngineGoTo) {                                   //If we are not yet done
        if (EngineGoInSteps == 0) {                                 //If no step amount is given
          EngineGoInSteps = 1000 ;                                  //Set the step to do amount
          EngineCurrentStep = EngineGoInSteps * 2;                  //Reset current step
        }
        if (EngineGoInSteps > 0) {                                  //If there still steps ToDo
          EngineGoInSteps--;                                        //Remove one from the list to do (sice we are doing one now)
          if (digitalRead(PDO_MotorBrake) == HIGH ) {               //If the brake is on
            digitalWrite(PDO_MotorBrake, LOW);                      //Don't brake
            delay(DelayAncher);                                     //Wait some time to make sure engine is off
          }
          if (EngineGoTo > 0) {                                     //If we need to move forward
            //Go to https://www.desmos.com/calculator and past this:
            //y=\left\{x>0\right\}\left\{x<a\right\}\left\{x<\frac{a}{2}:\frac{\frac{b}{2}}{\left(\frac{a}{2}\cdot\ \frac{a}{2}\right)}x^2,-2ba^{-2}\left(x-a\right)^2+b\right\}
            if (digitalRead(PDO_MotorReversePoles) == HIGH) {       //If pin is currently high
              digitalWrite(PDO_MotorReversePoles, LOW);             //Set pin low
              delay(DelayPole);                                     //Wait some time to make sure engine is off
            }
            if (EngineGoTo == 0) {                                  //If engine needs to be off
              Engine = 0;                                           //Set engine to be off
            } else if (Engine < EngineGoTo) {                       //If we need to speed up
              if (EngineCurrentStep < EngineGoInSteps / 2) {        //If we are starting up
                Engine = MaxValuePWM / 2 / (EngineGoInSteps * EngineGoInSteps / 4) * EngineCurrentStep * EngineCurrentStep;
              } else {
                Engine = -2 * MaxValuePWM * pow(EngineGoInSteps, -2) * (EngineCurrentStep - EngineGoInSteps) * (EngineCurrentStep - EngineGoInSteps) + MaxValuePWM;
              }
            } else if (Engine > EngineGoTo) {                       //If we need to speed down
              Engine--;                                             //remove 1 to the engine speed
            }
            digitalWrite(PDO_Motor, Engine);                        //Write the value to the engine
          } else {                                                  //it can not be off [0] and it was not forward, so it's backwards
            LED_Backwards = true;                                   //Set backwards driving LED to be on
            if (digitalRead(PDO_MotorReversePoles) == LOW) {        //If pin is currently low
              digitalWrite(PDO_MotorReversePoles, HIGH);            //Set pin high
              delay(DelayPole);                                     //Wait some time to make sure engine is off
            }
          }
        }
      }
    }
    //--------------------Steering control--------------------
    if (RotationGoTo < 0) {                                         //If we are going to rotate to the left
      LED_Left = true;                                              //Set left LED to be on
    } else if (RotationGoTo > 0) {                                  //If we are going to rotate to the right
      LED_Right = true;                                             //Set right LED to be on
    } else {
      LED_Left = false;                                             //Set left LED bool to off
      LED_Right = false;                                            //Set right LED bool to off
    }
    if (Rotation != RotationGoTo) {                                 //If we need to steer
      if (Rotation < RotationGoTo) {                                //If we are going [left] / [right]
        if (digitalRead(PDO_SteeringReversePoles) == LOW) {         //If pin is currently low
          digitalWrite(PDO_SteeringReversePoles, HIGH);             //Set pin high
          delay(DelayAncher);                                       //Wait some time to make sure engine is off
        }
        Rotation = Rotation + 1;                                    //Set the power to rotate
        analogWrite(PDO_Steering, HIGH);                            //Write the steering to the steer-pin (will be reseted after this loop, so it would look like a pulse)
      } else {
        if (digitalRead(PDO_SteeringReversePoles) == HIGH) {        //If pin is currently low
          digitalWrite(PDO_SteeringReversePoles, LOW);              //Set pin high
          delay(DelayAncher);                                       //Wait some time to make sure engine is off
        }
        Rotation = Rotation - 1;                                    //Set the power to rotate
        analogWrite(PDO_Steering, HIGH);                            //Write the steering to the steer-pin (will be reseted after this loop, so it would look like a pulse)
      }
    }
    //--------------------PC communication--------------------
    if (PcEverConnected) {                                          //If a PC has ever been connected
      String EmergencyButtonState = "!";                            //Create a string (and set it to warning, will be overwriten if its save)
      if (Emergency == 1) {                                         //Check if the emergency button isn't pressed
        EmergencyButtonState = "~";                                 //Set status of emergency button to save
      }
      String NewPCState = "[" + EmergencyButtonState + String("M") + String(EngineGoTo) + "]"; //Create a new state where the pc should be in right now
      if (LastPCStateEngine != NewPCState) {                        //If the PC state is not what it should be (and the PC needs an update)
        Serial.print(NewPCState);                                   //Write the info to the PC
        LastPCStateEngine = NewPCState;                             //Update what the PC should have
      }
      NewPCState = "[" + EmergencyButtonState + String("S") + String(RotationGoTo) + "]"; //Create a new state where the pc should be in right now
      if (LastPCStateRotation != NewPCState) {                      //If the PC state is not what it should be (and the PC needs an update)
        Serial.print(NewPCState);                                   //Write the info to the PC
        LastPCStateRotation = NewPCState;                           //Update what the PC should have
      }
    }
    //--------------------LED Control--------------------
    LEDControl();
  }
  delay(1);                                                         //Wait some time so the Arduino has free time (and we set arbitrary delays
}

void EmergencyReleased() {                                          //If the emergency button is pressed (checked 111111/sec?)
  digitalWrite(PDO_Motor, LOW);                                     //Turn engine off
  Emergency = 0;                                                    //Set the emergency pin to be low (this will tell the rest of the loop this has happened)
  RotationGoTo = Rotation;                                          //Stop with rotating, keep where ever it is at this moment
  EngineGoTo = 0;                                                   //Set EngineGoTo to 0 (so it doesn't turn on)
  if (Engine != EngineGoTo) {                                       //If we are not yet updated (the engine isn't in set state)
    delay(DelayAncher);                                             //Wait some time to make sure engine is off
    Engine = EngineGoTo;                                            //Update the engine state
    EngineFrom = Engine;                                            //Set current engine state to be the new state to start engine fom
  }
  digitalWrite(PDO_MotorBrake, HIGH);                               //Brake
  LED_Emergency = true;                                             //Set the emergency LED on
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
  bool LED_Left = false;                                              //
  int DelayAnimationDriving = 30;                                     //Delay in ms for the animation (excluding the write time delay!)
  int DelayanimationBlink = 15;                                       //Delay in ms for the animation (excluding the write time delay!)
  static byte TimerLED_Left;                                          //Create a timer (so we can make a delay in the animation
  if (LED_Left) {                                                     //Turning left
    TimerLED_Left ++;                                               //Add 1 to the timer
    if (TimerLED_Left >= DelayanimationBlink) {                                       //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
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
  static byte TimerLED_Right;                                         //Create a timer (so we can make a delay in the animation
  if (LED_Right) {                                                    //Turning right
    TimerLED_Right ++;                                                  //Add 1 to the timer
    if (TimerLED_Right >= DelayanimationBlink) {                                          //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
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
    if (TimerLED_Driving >= DelayAnimationDriving) {                                        //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
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
    if (TimerLED_Backwards >= DelayAnimationDriving) {                                      //If we looped 19+ times (aka 1ms/loop = 20ms) (to slow it down so it wont give everyone a pannic attack)
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
    leds[167] = CRGB(0, 255, 0);        //Temp! TODO FIXME
    FastLED.show();                                                     //Apply LED changes
    UpdateLEDs = false;                                                 //Flag update done
  }
}

void HeadJelle() {
  //--------------------Jelle's head--------------------
  if (Z > 0) {                                                          //If we need to move backwards
    Z--;                                                                //Remove one from Z (Z = amounts of steps to do backwards)
    EngineGoTo = -1;                                                    //Set engine state, Will be verwritten when false
    if (PDI_SensorBack > SensorLowLimit) {                              //If there is nothing behind us
      if (PDI_SensorRight > SensorLowLimit) {                           //If there is nothing right of us
        RotationGoTo = 0;                                               //Steer left
      } else {
        if (PDI_SensorLeft > SensorLowLimit) {                          //If there is nothing left of us
          RotationGoTo = 180;                                           //Steer right
        } else {
          EngineGoTo = 0;                                               //Turn engine off
        }
      }
    } else {
      EngineGoTo = 0;                                                   //Turn engine off (we can't move backwards
    }
  } else if (PDI_SensorLeft > SensorLowLimit and PDI_SensorRight > SensorLowLimit) {  //If there is nothing in front of us
    int X = PAI_SensorFrontLeft - PAI_SensorFrontRight;                 //Calculate diffrence in sensoren
    if (abs(X) > MiniumDifference) {                                    //If the change is not to small
      if ((X > 0 and PDI_SensorLeft > SensorLowLimit) or (X < 0 and PDI_SensorRight > SensorLowLimit)) {  //If there is nothing on that side of us
        RotationGoTo = Rotation + (X / DividerSteering);                //Steer to that side (with the intensity of 'X/DividerSteering'
      } else {
        RotationGoTo = Rotation;                                        //Stop steering
      }
      EngineGoTo = 1;                                                   //Turn engine on
    } else {
      RotationGoTo = Rotation;                                          //Stop steering
      EngineGoTo = 1;                                                   //Turn engine on
    }
  } else {
    Z = MiniumStepsBackwards;                                           //Tell the code that we need to go backwards from now on
  }


  //  EngineGoInSteps = 1000;                                       //Set the step to do amount
  //  EngineCurrentStep = EngineGoInSteps;                          //Reset current step

  map(EngineGoTo, -1, 1, -MaxValuePWM, MaxValuePWM);    //Remap to PWM
}
