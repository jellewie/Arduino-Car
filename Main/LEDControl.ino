//60 LEDs/M
//0.8m x 0.6m = 2.8M omtrek (and 0.48 m² surface)
//3M x 60LEDS/M = 180LEDs total * (3x20ma) = 10800ma (11A) Power bank is 26800 so we can do a few hours at full power!
//pin voor LED = PWO_LED
//https://github.com/FastLED/FastLED/wiki/Pixel-reference
//WS2812 LED data takes 30µs per pixel. If you have 100 pixels, then that means interrupts will be disabled for 3000µs, or 3ms.
//48 x 36 LEDS = 24 <Corner> 36 <> 48 <> 36 <> 24

void LEDControl() {                                                 //Code that controls all the LEDs
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
  if (LEDDisco) {                                                   //If we need a disco
    static byte gHue;                                               //Create a new variable
    EVERY_N_MILLISECONDS(DiscoSpeed) {                              //Do if x ms have pasted
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
