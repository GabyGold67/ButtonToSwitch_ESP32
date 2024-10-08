/**
  ******************************************************************************
  * @file	: 12_DDlydDALtchMPBttn_1a.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library DDlydDALtchMPBttn class
  *
  *   Framework: Arduino
  *   Platform: ESP32
  * 
  * The example instantiates a DDlydDALtchMPBttn object using:
  * 	- 1 push button between GND and dmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
  *   - 1 led with it's corresponding resistor between GND and isOnScndryLoadPin
  *
  * ### This example doesn't create extra Tasks:
  *
  * This simple example instantiates the DDlydDALtchMPBttn object in the setup(),
  * and uses the default "loop ()" (and yes, loop() is part of the loopTask()
  * disguised in the Ardu-ESP), in it and checks it's attribute flags locally
  * through the getters methods.
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

const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t isOnScndryLoadPin{GPIO_NUM_19};

SldrDALtchMPBttn dmpbBttn(dmpbSwitchPin, true, true, 50, 100, 1280);

// bool mpbttnLstStts{false};
// bool mpbttnLstOnScndry{false};

void setup() {
  pinMode(dmpbLoadPin, OUTPUT);
  pinMode(isOnScndryLoadPin, OUTPUT);

  dmpbBttn.setScndModActvDly(2000);
  dmpbBttn.begin();
}

void loop() {
  if(dmpbBttn.getOutputsChange()){
    /* The following commented out section is replaced by the single line of code following, use whichever code you're more comfortable with
    // Keep in mind you'll also need to comment out the variables definition for the variables used in this block, if there are any
    if(mpbttnLstStts != dmpbBttn.getIsOn()){
      mpbttnLstStts = dmpbBttn.getIsOn();
      digitalWrite(dmpbLoadPin, mpbttnLstStts?HIGH:LOW);
    }
    */
    digitalWrite(dmpbLoadPin, (dmpbBttn.getIsOn())?HIGH:LOW);

    /* The following commented out section is replaced by the single line of code following, use whichever code you're more comfortable with
    // Keep in mind you'll also need to comment out the variables definition for the variables used in this block, if there are any
    if(mpbttnLstOnScndry != dmpbBttn.getIsOnScndry()){
      mpbttnLstOnScndry = dmpbBttn.getIsOnScndry();
      digitalWrite(isOnScndryLoadPin, mpbttnLstOnScndry?HIGH:LOW);
    }
    */
    digitalWrite(isOnScndryLoadPin, dmpbBttn.getIsOnScndry()?HIGH:LOW);

    dmpbBttn.setOutputsChange(false);
  }  
}