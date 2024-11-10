/**
  ******************************************************************************
  * @file	: 13_SnglSrvcVdblMPBttn_2a.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library SnglSrvcVdblMPBttn class
  *
  *   Framework: Arduino
  *   Platform: ESP32
  * 
  * The example instantiates a SnglSrvcVdblMPBttn object using:
  * 	- 1 push button between GND and dmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
  *   - 1 led with it's corresponding resistor between GND and voidedPin
  *   - 1 led with it's corresponding resistor between GND and fnOnmpbLoadPin
  *
  * ### This example doesn't create extra Tasks (by using the default `loopTask` Task):
  *
  * This simple example instantiates the DbncdMPBttn object in the setup(),
  * and uses the default `loop ()` (loop() is the loopTask() disguised
  * in the Ardu-ESP), in it and checks it's attribute flags locally through the 
  * getters methods.
  * 
  * When a change in the object's outputs attribute flags values is detected, it
  * manages the loads and resources that the switch turns On and Off, in this
  * example case are the output of some GPIO pins.
  *
  * 	@author	: Gabriel D. Goldman
  *
  * 	@date	: 	01/08/2023 First release
  * 				    05/09/2024 Last update
  *
  ******************************************************************************
  * @attention	This file is part of the examples folder for the ButtonToSwitch_ESP32
  * library. All files needed are provided as part of the source code for the library.
  * 
  * Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
  *
  ******************************************************************************
  */
#include <Arduino.h>
#include <ButtonToSwitch_ESP32.h>

void otptSwp(); // Function prototype for the execution when the SnglSrvcVdblMPBttn enters the On State

const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t voidedPin{GPIO_NUM_19};
const uint8_t fnOnmpbLoadPin{GPIO_NUM_17};

SnglSrvcVdblMPBttn dmpbBttn (dmpbSwitchPin, true, true, 50, 50);

// bool mpbttnLstStts{false};
// bool mpbttnLstVdd{false};

void setup() {
   pinMode(dmpbLoadPin, OUTPUT);
   pinMode(voidedPin, OUTPUT);
   pinMode(fnOnmpbLoadPin, OUTPUT);

   dmpbBttn.setFnWhnTrnOnPtr(&otptSwp);
   dmpbBttn.begin();
}

void loop() {
   if(dmpbBttn.getOutputsChange()){
      /* The following commented out section is replaced by the pair of lines of code following, use whichever code you're more comfortable with
      // Keep in mind you'll also need to comment out the variables definition for the variables used in this block, if there are any
      if(mpbttnLstVdd != dmpbBttn.getIsVoided()){
         mpbttnLstVdd = dmpbBttn.getIsVoided();
         digitalWrite(vddLoadPin, mpbttnLstVdd?HIGH:LOW);
      }
      */
      digitalWrite(dmpbLoadPin, (dmpbBttn.getIsOn())?HIGH:LOW);
      digitalWrite(voidedPin, (dmpbBttn.getIsVoided())?HIGH:LOW);

      dmpbBttn.setOutputsChange(false);
   }  
}

//===============================>> User Functions Implementations BEGIN
void otptSwp(){
   /* The following commented out section is replaced by the single line of code following, use whichever code you're more comfortable with
   // Keep in mind you'll also need to comment out the variables definition for the variables used in this block, if there are any
   bool isOnFnLed = digitalRead(fnOnmpbLoadPin);
   digitalWrite(fnOnmpbLoadPin, (!isOnFnLed)?HIGH:LOW);
   */
   digitalWrite(fnOnmpbLoadPin, (!digitalRead(fnOnmpbLoadPin))?HIGH:LOW);

   return;
}
//===============================>> User Functions Implementations END
