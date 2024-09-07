/**
  ******************************************************************************
  * @file	: 09_TmVdblMPBttn_1a.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library TmVdblMPBttn class
  *
  *   Framework: Arduino
  *   Platform: ESP32
  * 
  * The example instantiates a TmVdblMPBttn object using:
  * 	- 1 push button between GND and dmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
  *   - 1 led with it's corresponding resistor between GND and tvlmpbLoadPin
  *
  * ### This example doesn't create extra Tasks:
  *
  * This simple example instantiates the TmVdblMPBttn object in the setup(),
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

const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t tvLoadPin{GPIO_NUM_19};

TmVdblMPBttn dmpbBttn (dmpbSwitchPin, 3000, true, true, 50, 50, false);

bool mpbttnLstStts{false};
bool mpbttnLstVdd{false};

void setup() {
  pinMode(dmpbLoadPin, OUTPUT);
  pinMode(tvLoadPin, OUTPUT);

  dmpbBttn.begin();
}

void loop() {
  if(dmpbBttn.getOutputsChange()){
    if(mpbttnLstStts != dmpbBttn.getIsOn()){
      mpbttnLstStts = dmpbBttn.getIsOn();
      if (mpbttnLstStts){
        digitalWrite(dmpbLoadPin, HIGH);
      }
      else{
        digitalWrite(dmpbLoadPin, LOW);
      }
    }
    
    if(mpbttnLstVdd != dmpbBttn.getIsVoided()){
      mpbttnLstVdd = dmpbBttn.getIsVoided();
      if (mpbttnLstVdd){
        digitalWrite(tvLoadPin, HIGH);
      }
      else{
        digitalWrite(tvLoadPin, LOW);
      }
    }
    dmpbBttn.setOutputsChange(false);
  }  
}