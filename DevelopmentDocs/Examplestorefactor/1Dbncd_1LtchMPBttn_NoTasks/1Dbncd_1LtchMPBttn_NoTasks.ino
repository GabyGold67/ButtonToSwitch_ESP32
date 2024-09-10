#include <Arduino.h>
#include <ButtonToSwitch_ESP32.h>

/*
  mpbToSwitch Library for
    Framework: Arduino
    Platform: ESP32

  1Dbncd_1LtchMPBttn_NoTasks.ino
  Created by Gabriel D. Goldman, August, 2023.
  Updated by Gabriel D. Goldman, August 20, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.

Example file to demonstrate DbncdDlydMPBttn and LtchMPBttn classes, required hardware and connections:
  _ 1 push button between GND and dbncdSwitchPin
  _ 1 push button between GND and ltchSwitchPin
  _ 1 led with it's corresponding resistor between GND and dbncdLoad
  _ 1 led with it's corresponding resistor between GND and ltchdLoad

  Pressing the push button connected to dbncdSwitchPin will turn the led immediately on and keep it lit while it's being pressed.
  Pressing the push button connected to ltchdSwitchPin will turn the led on after a 250 miliseconds delay and keep it lit until it is released, and until the same push button is 
  pressed once again, effectively making a latch button switch out of a push button.

  The input signals comming from the push buttons are not processed by tasks, but in the loop() routine as is usual in Arduino style sketches
   (and yes, loop() is the loopTask() disguised in the Ardu-ESP), showing the ease of use of the library even without much experience in ESP-FreeRTOS
*/

const uint8_t dbncdLoad{GPIO_NUM_19};
const uint8_t ltchdLoad{GPIO_NUM_21};
const uint8_t dbncdSwitchPin{GPIO_NUM_26};
const uint8_t ltchdSwitchPin{GPIO_NUM_25};

TgglLtchMPBttn ltchdBttn (ltchdSwitchPin, true, true, 20, 250);
DbncdMPBttn dbncdBttn (dbncdSwitchPin);

void setup() {
  // put your setup code here, to run once:
  pinMode(dbncdLoad, OUTPUT);
  pinMode(ltchdLoad, OUTPUT);

  ltchdBttn.begin();
  dbncdBttn.begin();
}

void loop() {
  if (ltchdBttn.getIsOn()){
    digitalWrite(ltchdLoad, HIGH);
  }
  else{
    digitalWrite(ltchdLoad, LOW);
  }

  if (dbncdBttn.getIsOn()){
    digitalWrite(dbncdLoad, HIGH);
  }
  else {
    digitalWrite(dbncdLoad, LOW);
  }      
}
