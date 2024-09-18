/**
  ******************************************************************************
  * @file	: 13_SnglSrvcVdblMPBttn_1b.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library SnglSrvcVdblMPBttn class
  *
  *   Framework: Arduino
  *   Platform: ESP32
  * 
  * The example instantiates a SnglSrvcVdblMPBttn object using:
  * 	- 1 push button between GND and dmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
  *   - 1 led with it's corresponding resistor between GND and tvlmpbLoadPin
  *   - 1 led with it's corresponding resistor between GND and fnOnmpbLoadPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbIsDisabledPin
  *
  * ### This example doesn't create extra Tasks:
  * ### This example creates a software timer
  *
  * This simple example instantiates the SnglSrvcVdblMPBttn object in the setup(),
  * and uses the default "loop ()" (and yes, loop() is part of the loopTask()
  * disguised in the Ardu-ESP), in it and checks it's attribute flags locally
  * through the getters methods.
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
  * 				    18/09/2024 Last update
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
void otptSwp(); // Function prototype for the execution when the SnglSrvcVdblMPBttn enters the On State
void swpEnableCb(TimerHandle_t pvParam);
void Error_Handler();
//===============================>> User Functions Prototypes END

const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t vddLoadPin{GPIO_NUM_19};
const uint8_t fnOnmpbLoadPin{GPIO_NUM_17};

const uint8_t dmpbIsDisabledPin{GPIO_NUM_18};

TimerHandle_t enableSwpTmrHndl{NULL};
BaseType_t tmrModRslt;

SnglSrvcVdblMPBttn dmpbBttn (dmpbSwitchPin, true, true, 50, 50);
VdblMPBttn* dmpbBttnPtr {&dmpbBttn};

void setup() {
   pinMode(dmpbLoadPin, OUTPUT);
   pinMode(vddLoadPin, OUTPUT);
   pinMode(fnOnmpbLoadPin, OUTPUT);
   pinMode(dmpbIsDisabledPin, OUTPUT);

   enableSwpTmrHndl = xTimerCreate(
      "isEnabledSwapTimer",
      10000,
      pdTRUE,
      dmpbBttnPtr,
      swpEnableCb
   );
   if (enableSwpTmrHndl != NULL){
      tmrModRslt = xTimerStart(enableSwpTmrHndl, portMAX_DELAY);
   }
	if(tmrModRslt == pdFAIL){
      Error_Handler();
	}

   dmpbBttn.setFnWhnTrnOnPtr(&otptSwp);
   dmpbBttn.begin();
}

void loop() {
   if(dmpbBttn.getOutputsChange()){
      digitalWrite(dmpbLoadPin, (dmpbBttn.getIsOn())?HIGH:LOW);
      digitalWrite(vddLoadPin, (dmpbBttn.getIsVoided())?HIGH:LOW);
      digitalWrite(dmpbIsDisabledPin, (dmpbBttn.getIsEnabled())?LOW:HIGH);

      dmpbBttn.setOutputsChange(false);
   }  
}

//===============================>> User Functions Implementations BEGIN

void otptSwp(){
   /* The following commented out section is replaced by the single line of code following, use whichever code you're more comfortable with
   // Keep in mind you'll also need to comment out the variables definition for the variables used in this block, if there are any
   bool isOnFnLed = digitalRead(fnOnmpbLoadPin);
   digitalWrite(fnOnmpbLoadPin, (!isOnFnLed)?HIGH:LOW);
   */
   digitalWrite(fnOnmpbLoadPin, (!digitalRead(fnOnmpbLoadPin))?HIGH:LOW);

   return;
}

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
