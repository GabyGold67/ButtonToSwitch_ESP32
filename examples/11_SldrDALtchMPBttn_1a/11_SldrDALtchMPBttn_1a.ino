/**
  ******************************************************************************
  * @file	: 11_SldrDALtchMPBttn_1a.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library SldrDALtchMPBttn class
  *
  *   Framework: Arduino
  *   Platform: ESP32
  * 
  * The example instantiates a SldrDALtchMPBttn object using:
  * 	- 1 push button between GND and dmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
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
  * This example shows the use of a SldrDALtchMPBttn to:
  * - Turn On a led
  * - Turn Off a led
  * - While the led is On, pressing the MPB for more than 2 seconds (or holding
  *  the MPB while the led is Off for more than 2 seconds after the debounce
  *  period) the led brightness will start changing to a lower brightness.
  * - Releasing the MPB will stop the dimming
  * - Pressing the MPB back for a long press will start changing the led to a higher
  * brightness.
  * - Pressing the MPB back for a short press will turn it Off
  * - Pressing once again for a short press will turn it On again, at the same brightness
  * level it was before turned Off
  *
  * 	@author	: Gabriel D. Goldman
  *
  * 	@date	: 	01/08/2023 First release
  * 				    16/09/2024 Last update
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

SldrDALtchMPBttn dmpbBttn(dmpbSwitchPin, true, true, 50, 100, 1280);

void setup() {
  pinMode(dmpbLoadPin, OUTPUT);

  dmpbBttn.setOtptValMin(255);   // Set minimum value to 10% of the total range
  dmpbBttn.setOtptValMax(2550);  // Set the maximum value to 100% of the total range
  dmpbBttn.setSwpDirOnEnd(false);   // This sets the SldrDALtchMPBttn dimmer NOT to change the "dimming direction" when reaching the set minimum and maximum values
  dmpbBttn.setSwpDirOnPrss(true);   // This sets the SldrDALtchMPBttn dimmer to change the "dimming direction" every time the MPB is pressed to enter the Secondary behavior
  dmpbBttn.setSldrDirUp(); // This sets the dimming direction to start incrementing its value, BUT as the setSwpDirOnPrss() indicates it must change direction as it is pressed, it will start changing directions to Down, and then start the changing values process, si the first time it will start dimming off the led brightness
  dmpbBttn.setOtptSldrStpSize(1);
  dmpbBttn.setScndModActvDly(2000);
  dmpbBttn.begin(5);
}

void loop() {
   if(dmpbBttn.getOutputsChange()){
      analogWrite(dmpbLoadPin, (dmpbBttn.getIsOn())?(dmpbBttn.getOtptCurVal()/10):0);

      dmpbBttn.setOutputsChange(false);
   }  
}