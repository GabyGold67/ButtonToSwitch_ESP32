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
  * 				    15/09/2024 Last update
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

void setup() {
  /* 
  ! NOTE: The following line ensures that the provided standard "Arduino loopTask" priority is set to a convenient level for the example purpose. 
   */
  vTaskPrioritySet(NULL, configTIMER_TASK_PRIORITY);  

  pinMode(dmpbLoadPin, OUTPUT);
  pinMode(wrnngLoadPin, OUTPUT);
  pinMode(pltLoadPin, OUTPUT);
  
  dmpbBttn.setKeepPilot(true);
  dmpbBttn.begin(20);
}

void loop() {
  if(dmpbBttn.getOutputsChange()){ //FAILS with the use of getOutputsChange!!!
    /* 
    The following commented out section is replaced by the single line following. Use whichever code you're more used to
    if (dmpbBttn.getIsOn())
      digitalWrite(dmpbLoadPin, HIGH);
    else
      digitalWrite(dmpbLoadPin, LOW);
    */
    digitalWrite(dmpbLoadPin, (dmpbBttn.getIsOn())?HIGH:LOW);

    /*
    The following commented out section is replaced by the single line following. Use whichever code you're more used to
    if (dmpbBttn.getWrnngOn())
      digitalWrite(wrnngLoadPin, HIGH);
    else
      digitalWrite(wrnngLoadPin, LOW);
    */
    digitalWrite(wrnngLoadPin, (dmpbBttn.getWrnngOn())?HIGH:LOW);

    /*
    The following commented out section is replaced by the single line following. Use whichever code you're more used to
    if (dmpbBttn.getPilotOn())
      digitalWrite(pltLoadPin, HIGH);
    else
      digitalWrite(pltLoadPin, LOW);
    */
    digitalWrite(pltLoadPin, (dmpbBttn.getPilotOn())?HIGH:LOW);
    
    dmpbBttn.setOutputsChange(false);
  }
}  
