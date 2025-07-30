/**
  ******************************************************************************
  * @file	: 09_TmVdblMPBttn_1a.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library TmVdblMPBttn class
  *
  * Repository: https://github.com/GabyGold67/ButtonToSwitch_ESP32
  * WOKWI simulation URL: https://wokwi.com/projects/437896610753650689
  * 
  * Framework: Arduino
  * Platform: ESP32
  * 
  * @details The example instantiates a TmVdblMPBttn object using:
  * 	- 1 push button between GND and dmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
  *   - 1 led with it's corresponding resistor between GND and tvlmpbLoadPin
  *
  * - This example doesn't create extra Tasks:
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
  * @author	: Gabriel D. Goldman
  * mail <gdgoldman67@hotmail.com>
  * Github <https://github.com/GabyGold67>
  *
  * @date	: 	01/08/2023 First release
  * 			   15/09/2024 Last update
  *
  ******************************************************************************
  * @warning **Use of this library is under your own responsibility**
  * 
  * @warning The use of this library falls in the category described by The Alan 
  * Parsons Project (c) 1980 "Games People play" disclaimer:  
  * Games people play, you take it or you leave it  
  * Things that they say aren't alright  
  * If I promised you the moon and the stars, would you believe it?  
  * 
  * Released into the public domain in accordance with "GPL-3.0-or-later" license terms.
  ******************************************************************************
  */
#include <Arduino.h>
#include <ButtonToSwitch_ESP32.h>

const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t tvLoadPin{GPIO_NUM_18};

TmVdblMPBttn dmpbBttn (dmpbSwitchPin, 3000, true, true, 50, 50, false);

void setup() {
  pinMode(dmpbLoadPin, OUTPUT);
  pinMode(tvLoadPin, OUTPUT);
  dmpbBttn.begin(5);
}

void loop() {
   if(dmpbBttn.getOutputsChange()){
      /* The following commented out section is replaced by the single line following. Use whichever code you're more used to
      if (dmpbBttn.getIsOn())
         digitalWrite(dmpbLoadPin, HIGH);
      else
         digitalWrite(dmpbLoadPin, LOW);
      */
      digitalWrite(dmpbLoadPin, (dmpbBttn.getIsOn())?HIGH:LOW);

      /* The following commented out section is replaced by the single line following. Use whichever code you're more used to
      if (dmpbBttn.getIsVoided())
         digitalWrite(tvLoadPin, HIGH);
      else
         digitalWrite(tvLoadPin, LOW);
      */
      digitalWrite(tvLoadPin, (dmpbBttn.getIsVoided())?HIGH:LOW);
      
      dmpbBttn.setOutputsChange(false);
   }
}  
