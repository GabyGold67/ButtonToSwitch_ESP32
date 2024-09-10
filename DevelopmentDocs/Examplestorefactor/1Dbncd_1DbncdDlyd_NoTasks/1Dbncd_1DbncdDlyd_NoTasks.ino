#include <Arduino.h>
#include <ButtonToSwitch_ESP32.h>

/*
  mpbToSwitch Library for
    Framework: Arduino
    Platform: ESP32

  1Dbncd_1DbncdDlyd_NoTasks.ino
  Created by Gabriel D. Goldman, August, 2023.
  Updated by Gabriel D. Goldman, August 17, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.

  Example file to demonstrate DbncdMPBttn and DbncdDlydMPBttn classes, required hardware and connections:
  _ 1 push button between GND and dmpbSwitchPin
  _ 1 push button between GND and ddmpbSwitchPin
  _ 1 led with it's corresponding resistor between GND and dmpbLoadPin
  _ 1 led with it's corresponding resistor between GND and ddmpbLoadPin

  Pressing the push button connected to dmpbSwitchPin will turn the led immediately on and keep it lit while it's being pressed. After 5 times the led is lit the 
  button status checking is paused, until the other push button accumulates 5 pushes. The pushes don't need to be consecutive, the other button might be pushed between this 
  button pushes, when this one accumulates 5 the other must also accumulate 5 to resume this button activities.
  Pressing the push button connected to ddmpbSwitchPin will turn the led on after a 500 miliseconds  delay and keep it lit while it's being pressed

  The input signals coming from the push buttons are not processed by tasks, but in the loop() routine as is usual in Arduino style sketches, just to show there's no need to
   build specific tasks to process the class objects signals, although the "superloop" disadvantages will be back (and yes, loop() is the loopTask() disguised in the Ardu-ESP)
*/

const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t ddmpbLoadPin{GPIO_NUM_19};
const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t ddmpbSwitchPin{GPIO_NUM_26};

DbncdDlydMPBttn dmpbBttn (dmpbSwitchPin, true, true, 20, 500);
DbncdMPBttn ddmpbBttn (ddmpbSwitchPin);

const unsigned int toggleCount {5};

unsigned int ddmpbCount {0};
bool ddmpbFlips{false};
unsigned int dmpbAloneCount {0};
bool dmpbFlips{false};

void setup() {
  pinMode(ddmpbLoadPin, OUTPUT);
  pinMode(dmpbLoadPin, OUTPUT);

  dmpbBttn.begin();
  ddmpbBttn.begin();
}

void loop() {
  if (dmpbBttn.getIsOn()){
    digitalWrite(dmpbLoadPin, HIGH);
    if (!dmpbFlips){
      if(ddmpbCount == toggleCount){
        dmpbFlips = true;
        dmpbAloneCount ++;
        if(dmpbAloneCount == toggleCount){
          ddmpbCount = 0;
          ddmpbFlips = false;
          dmpbAloneCount = 0;
          ddmpbBttn.resume();
        }
      }
    }
  }
  else{
    digitalWrite(dmpbLoadPin, LOW);
    if(dmpbFlips){
      dmpbFlips = false;
    }      
  }

  if (ddmpbBttn.getIsOn()){
    digitalWrite(ddmpbLoadPin, HIGH);
    if (!ddmpbFlips){
      ddmpbFlips = true;
      ddmpbCount++;
    }
  }
  else {
    digitalWrite(ddmpbLoadPin, LOW);
    if(ddmpbFlips){
      ddmpbFlips = false;  
      if (ddmpbCount == toggleCount){
        ddmpbBttn.pause();
      }
    }      
  }
}  
