/**
  ******************************************************************************
  * @file	: 11_SldrDALtchMPBttn_2a.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library SldrDALtchMPBttn class
  *
  *   Framework: Arduino
  *   Platform: ESP32
  * 
  * The example instantiates a SldrDALtchMPBttn object using:
  * 	- 1 push button between GND and dmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
  *   - 1 led with it's corresponding resistor between GND and isOnScndryLoadPin
  *   - 1 led with it's corresponding resistor between GND and curValLoadPin
  *
  * ### This example doesn't create extra Tasks:
  *
  * This simple example instantiates the TmVdblMPBttn object in the setup(),
  * and uses the default `loop ()` (loop() is the loopTask() disguised
  * in the Ardu-ESP), in it and checks it's attribute flags locally through the 
  * getters methods.
  * 
  * When a change in the object's outputs attribute flags values is detected, it
  * manages the loads and resources that the switch turns On and Off, in this
  * example case are the output of some GPIO pins.
  *
  *   * This example shows the use of a SldrDALtchMPBttn to manage three output leds:
  * - The first led to show the state of the isOn attribute flag.
  * - The second led to show the slider ("dimmer") value.
  * - The third led to show the state of the isOnScndry attribute flag.
  * 
  * A short press will turn on the first led and the third led to the setted brightness value
  * - While the those leds are On, pressing the MPB for more than 2 seconds (or holding
  * the MPB while the led is Off for more than 2 seconds after the debounce
  * period) will turn the third led to show the "dimmer mode" is activated and the led brightness will start
  * changing. Please consider that the configuration done in the setup() sets the dimming action to change
  * directions every time the MPB is released and pushed back AND every time the dimming value reaches the 
  * limits set with `setOtptValMin()` and `setOtptValMax()`
  * - Releasing the MPB will stop the dimming
  * - Pressing the MPB back for a long press will start changing the led to a higher
  * brightness.
  * - Pressing the MPB back for a short press will turn it Off
  * - Pressing once again for a short press will turn it On again, at the same brightness
  * level it was before turned Off

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
const uint8_t curValLoadPin{GPIO_NUM_17};

SldrDALtchMPBttn dmpbBttn(dmpbSwitchPin, true, true, 50, 100, 1280);

// bool mpbttnLstStts{false};
// bool mpbttnLstOnScndry{false};
// uint16_t mpbttnLstCurVal{0};

void setup() {
  pinMode(dmpbLoadPin, OUTPUT);
  pinMode(isOnScndryLoadPin, OUTPUT);
  pinMode(curValLoadPin, OUTPUT);

  dmpbBttn.setOtptValMin(0);
  dmpbBttn.setOtptValMax(2550);
  dmpbBttn.setSldrDirDn();
  dmpbBttn.setSwpDirOnPrss(true);
  dmpbBttn.setSwpDirOnEnd(true);
  dmpbBttn.setOtptSldrStpSize(1);
  dmpbBttn.setScndModActvDly(2000);
  dmpbBttn.begin(10);
}

void loop() {
  if(dmpbBttn.getOutputsChange()){
    /* The following commented out section is replaced by the single line of code following, use whichever code you're more comfortable with
    // Keep in mind you'll also need to comment out the variables definition for the variables used in this block, if there are any
    if(mpbttnLstStts != dmpbBttn.getIsOn()){
      mpbttnLstStts = dmpbBttn.getIsOn();
      digitalWrite(dmpbLoadPin, mpbttnLstStts?HIGH:LOW);
      analogWrite(curValLoadPin, mpbttnLstStts?(mpbttnLstCurVal/10):0);
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

    /* The following commented out section is replaced by the single line of code following, use whichever code you're more comfortable with
    // Keep in mind you'll also need to comment out the variables definition for the variables used in this block, if there are any
    if(mpbttnLstCurVal != dmpbBttn.getOtptCurVal()){
      mpbttnLstCurVal = dmpbBttn.getOtptCurVal();
      analogWrite(curValLoadPin, mpbttnLstCurVal/10);
    }
    */
   analogWrite(curValLoadPin, (dmpbBttn.getIsOn())?(dmpbBttn.getOtptCurVal()/10):0);

    dmpbBttn.setOutputsChange(false);
  }  
}