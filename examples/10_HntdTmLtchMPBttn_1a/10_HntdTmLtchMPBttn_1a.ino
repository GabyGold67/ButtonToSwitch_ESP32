/**
  ******************************************************************************
  * @file	: 10_HntdTmLtchMPBttn_1a.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library HntdTmLtchMPBttn class
  *
  *   Framework: Arduino
  *   Platform: ESP32
  * 
  * The example instantiates a HntdTmLtchMPBttn object using:
  * 	- 1 push button between GND and dmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
  *   - 1 led with it's corresponding resistor between GND and wrnngLoadPin
  *   - 1 led with it's corresponding resistor between GND and pltLoadPin
  *
  * ### This example doesn't create extra Tasks:
  *
  * This simple example instantiates the HntdTmLtchMPBttn object in the setup(),
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
const uint8_t wrnngLoadPin{GPIO_NUM_19};
const uint8_t pltLoadPin{GPIO_NUM_17};

HntdTmLtchMPBttn dmpbBttn (dmpbSwitchPin, 4000, 25, true, true, 20, 50);

bool mpbttnLstStts{false};
bool mpbttnLstWrnng{false};
bool mpbttnLstPlt{false};

bool mpbttnCurStts{false};
bool mpbttnCurWrnng{false};
bool mpbttnCurPlt{false};

void setup() {
  pinMode(dmpbLoadPin, OUTPUT);
  pinMode(wrnngLoadPin, OUTPUT);
  pinMode(pltLoadPin, OUTPUT);

  dmpbBttn.setKeepPilot(true);
  dmpbBttn.begin();
}

void loop() {
  mpbttnCurStts = dmpbBttn.getIsOn();
  mpbttnCurWrnng = dmpbBttn.getWrnngOn();
  mpbttnCurPlt = dmpbBttn.getPilotOn();

  if(mpbttnLstStts != mpbttnCurStts){
    mpbttnLstStts = mpbttnCurStts;
    digitalWrite(dmpbLoadPin, mpbttnLstStts?HIGH:LOW);
  }

  if(mpbttnLstWrnng != mpbttnCurWrnng){
    mpbttnLstWrnng = mpbttnCurWrnng;
    digitalWrite(wrnngLoadPin, mpbttnLstWrnng?HIGH:LOW);
  }

  if(mpbttnLstPlt != mpbttnCurPlt){
    mpbttnLstPlt = mpbttnCurPlt;
    digitalWrite(pltLoadPin, mpbttnLstPlt?HIGH:LOW);
  }
}  
