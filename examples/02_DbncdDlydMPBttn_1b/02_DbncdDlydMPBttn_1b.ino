/**
  ******************************************************************************
  * @file	: 02_DbncdDlydMPBttn_1b.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library DbncdDlydMPBttn class
  *
  *   Framework: Arduino
  *   Platform: ESP32
  * 
  * The example instantiates a DbncdDlydMPBttn object using:
  * 	- 1 push button between GND and ddmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and ddmpbLoadPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbIsDisabledPin
  *
  * ### This example doesn't create extra Tasks:
  * ### This example creates a software timer
  *
  * This simple example instantiates the DbncdDlydMPBttn object in the setup(),
  * and uses the default `loop ()` (loop() is the loopTask() disguised
  * in the Ardu-ESP), in it and checks it's attribute flags locally through the 
  * getters methods.
  * 
  * When a change in the object's outputs attribute flags values is detected, it
  * manages the loads and resources that the switch turns On and Off, in this
  * example case are the output of some GPIO pins.
  *
  * A software timer is created so that it periodically toggles the isEnabled attribute flag
  * value, showing the behavior of the instantiated object when enabled and when disabled.
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

//===============================>> User Functions Prototypes BEGIN
void swpEnableCb(TimerHandle_t pvParam);
void Error_Handler();
//===============================>> User Functions Prototypes END

const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t dmpbIsDisabledPin{GPIO_NUM_18};

TimerHandle_t enableSwpTmrHndl{NULL};
BaseType_t tmrModRslt;

DbncdDlydMPBttn dmpbBttn (dmpbSwitchPin, true, true, 50, 350);
DbncdMPBttn* dmpbBttnPtr {&dmpbBttn};

void setup() {
  pinMode(dmpbLoadPin, OUTPUT);
  pinMode(dmpbIsDisabledPin, OUTPUT);

  enableSwpTmrHndl = xTimerCreate(
    "isEnabledSwapTimer",
    10000,
    pdTRUE,
    dmpbBttnPtr,
    swpEnableCb
  );

	dmpbBttn.setIsOnDisabled(true);
  dmpbBttn.begin();

  if (enableSwpTmrHndl != NULL){
      tmrModRslt = xTimerStart(enableSwpTmrHndl, portMAX_DELAY);
   }
	if(tmrModRslt == pdFAIL){
	    Error_Handler();
	}
}

void loop() {
  if(dmpbBttn.getOutputsChange()){
    /* The following commented out section is replaced by the single line of code following, use whichever code you're more used to
    if (dmpbBttn.getIsOn())
      digitalWrite(dmpbLoadPin, HIGH);
    else
      digitalWrite(dmpbLoadPin, LOW);
    */
    digitalWrite(dmpbLoadPin, (dmpbBttn.getIsOn())?HIGH:LOW);
    /* The following commented out section is replaced by the single line of code following, use whichever code you're more used to
    if (dmpbBttn.getIsEnabled()())
      digitalWrite(dmpbIsDisabledPin, HIGH);
    else
      digitalWrite(dmpbIsDisabledPin, LOW);
    */
    digitalWrite(dmpbIsDisabledPin, (dmpbBttn.getIsEnabled())?LOW:HIGH);

    dmpbBttn.setOutputsChange(false);
  }
}  

//===============================>> User Timers Implementations BEGIN
/**
 * @brief Timer callback function
 * 
 * @param pvParam The callback function argument.
 * In this case is a pointer to the MPB object to be enabled and disabled periodically. 
 * It's a DbncdMPBttn* as enable() and disable() are dynamic polymorphic so they can be invoked through a base class pointer call.
 */
void swpEnableCb(TimerHandle_t pvParam){
  DbncdMPBttn* dbncdMPBLocPtr = (DbncdMPBttn*) pvTimerGetTimerID(pvParam);
  // bool mpbttnIsEnbldStts{dbncdMPBLocPtr->getIsEnabled()};

  if (dbncdMPBLocPtr->getIsEnabled())
    dbncdMPBLocPtr->disable();
  else
    dbncdMPBLocPtr->enable();

  return;
}
//===============================>> User Timers Implementations END

//===============================>> User Functions Implementations BEGIN
/**
 * @brief Error Handling function
 * 
 */
void Error_Handler(){
  for(;;)
  {    
  }
  
  return;
}
//===============================>> User Functions Implementations END
