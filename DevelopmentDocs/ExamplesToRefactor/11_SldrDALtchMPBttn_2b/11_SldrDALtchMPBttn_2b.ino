/**
  ******************************************************************************
  * @file	: 11_SldrDALtchMPBttn_2b.ino
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
  * 	- 1 led with it's corresponding resistor between GND and dmpbIsDisabledPin
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
  * A software timer is created so that it periodically toggles the isEnabled attribute flag
  * value, showing the behavior of the instantiated object when enabled and when disabled.
  * 
  * This example shows the use of a SldrDALtchMPBttn to manage four output leds:
  * - The first led to show the state of the isOn attribute flag.
  * - The second led to show the slider ("dimmer") value.
  * - The third led to show the state of the isOnScndry attribute flag.
  * - The fourth led to show the state of the isEnabled attribute flag.
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

//===============================>> User Functions Prototypes BEGIN
void swpEnableCb(TimerHandle_t pvParam);
void Error_Handler();
//===============================>> User Functions Prototypes END

const uint8_t dmpbSwitchPin{GPIO_NUM_25};
const uint8_t dmpbLoadPin{GPIO_NUM_21};
const uint8_t isOnScndryLoadPin{GPIO_NUM_19};
const uint8_t curValLoadPin{GPIO_NUM_17};
const uint8_t dmpbIsDisabledPin{GPIO_NUM_18};

TimerHandle_t enableSwpTmrHndl{NULL};
BaseType_t tmrModRslt;

SldrDALtchMPBttn dmpbBttn(dmpbSwitchPin, true, true, 50, 100, 1280);
DblActnLtchMPBttn* dmpbBttnPtr {&dmpbBttn};

void setup() {
   pinMode(dmpbLoadPin, OUTPUT);
   pinMode(isOnScndryLoadPin, OUTPUT);
   pinMode(curValLoadPin, OUTPUT);
   pinMode(dmpbIsDisabledPin, OUTPUT);

   enableSwpTmrHndl = xTimerCreate(
      "isEnabledSwapTimer",
      15000,
      pdTRUE,
      dmpbBttnPtr,
      swpEnableCb
   );
   if (enableSwpTmrHndl != NULL)
      tmrModRslt = xTimerStart(enableSwpTmrHndl, portMAX_DELAY);
	if(tmrModRslt == pdFAIL)
      Error_Handler();

   dmpbBttn.setOtptValMin(0);
   dmpbBttn.setOtptValMax(2550);
   dmpbBttn.setSldrDirDn();
   dmpbBttn.setSwpDirOnPrss(true);
   dmpbBttn.setSwpDirOnEnd(true);
   dmpbBttn.setOtptSldrStpSize(1);
   dmpbBttn.setScndModActvDly(2000);
   dmpbBttn.setIsOnDisabled(false);
   dmpbBttn.begin(10);
}

void loop() {
  if(dmpbBttn.getOutputsChange()){
      digitalWrite(dmpbLoadPin, (dmpbBttn.getIsOn())?HIGH:LOW);
      digitalWrite(isOnScndryLoadPin, dmpbBttn.getIsOnScndry()?HIGH:LOW);
      analogWrite(curValLoadPin, (dmpbBttn.getIsOn())?(dmpbBttn.getOtptCurVal()/10):0);
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
  
  if (dbncdMPBLocPtr->getIsEnabled())
    dbncdMPBLocPtr->disable();
  else
    dbncdMPBLocPtr->enable();

  return;
}
//===============================>> User Timers Implementations End

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
