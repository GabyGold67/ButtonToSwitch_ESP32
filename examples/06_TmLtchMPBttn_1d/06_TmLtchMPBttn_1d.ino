/**
  ******************************************************************************
  * @file	: 06_TmLtchMPBttn_1d.ino
  * @brief  : Example for the ButtonToSwitch_ESP32 library TmLtchMPBttn class
  *
  *   Framework: Arduino
  *   Platform: ESP32
  * 
  * The example instantiates a TmLtchMPBttn object using:
  * 	- 1 push button between GND and dmpbSwitchPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbLoadPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbIsDisabledPin
  * 	- 1 led with it's corresponding resistor between GND and dmpbTskWhlOnPin
  *
  * ### This example creates three tasks and deletes the default `loopTask` task
  * ### This example creates a software timer
  *
  * The "Main control task" is created in the regular setup() and then that 
  * arduino standard `loopTask` is deleted in the loop()
  * 
  * In the main control task the TmLtchMPBttn object is instantiated and configured
  * as usual, including a task to be unblocked when the isOn state changes by using
  * `dmpbBttn.setTaskToNotify(dmpsOutputTskHdl)`. That second task is also created
  * in the main control task, as is created the software timer that controls the 
  * isEnabled state.
  * 
  * When a change in the object's outputs attribute flags values is detected, the
  * task that manages the loads and resources that the switch turns On and Off is 
  * unblocked, with the provision of the MPB state passed as a 32-bits argument.
  * The task then takes care of updating the GPIO outputs.
  *
  * - The third task is created, started and blocked, like the second, it's 
  * purpose is to execute while the MPB is in "isOn state". 
  * Please read the library documentation regarding the consequences of executing
  * a task that is resumed and paused externally and without previous notification
  * to let the task to be paused in an orderly manner!!
  * 
  * The software timer created periodically toggles the isEnabled attribute flag
  * value, showing the behavior of the instantiated object when enabled and when
  * disabled.
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

//===============================>> User Functions Prototypes BEGIN
void swpEnableCb(TimerHandle_t pvParam);
void Error_Handler();

void mainCtrlTsk(void *pvParameters);
void dmpsOutputTsk(void *pvParameters);
void dmpsActWhlOnTsk(void *pvParameters);
//===============================>> User Functions Prototypes END

//===============================>> User Tasks & Timers related declarations BEGIN
TaskHandle_t mainCtrlTskHndl {NULL};
TaskHandle_t dmpsOutputTskHdl;
TaskHandle_t dmpsActWhlOnTskHndl;
BaseType_t xReturned;

TimerHandle_t enableSwpTmrHndl{NULL};
//===============================>> User Tasks & Timers related declarations END

void setup() {
  //Create the Main control task to keep, the MPBs outputs updated and set the Callback task function
   xReturned = xTaskCreatePinnedToCore(
      mainCtrlTsk,  //Callback function/task to be called
      "MainControlTask",  //Name of the task
      1716,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  //Pointer to the parameters for the function to work with
      configTIMER_TASK_PRIORITY,  //Priority level given to the task: use the same as the Software Timers priority level      
      &mainCtrlTskHndl, //Task handle
      xPortGetCoreID() //Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
   );
   if(xReturned != pdPASS)
      Error_Handler();
}

void loop() {
   vTaskDelete(NULL);
}  

//===============================>> User Tasks Implementations BEGIN
void mainCtrlTsk(void *pvParameters){
   const uint8_t dmpbSwitchPin{GPIO_NUM_25};

   TmLtchMPBttn dmpbBttn (dmpbSwitchPin, 3000, true, true, 50, 100);
   LtchMPBttn* dmpbBttnPtr {&dmpbBttn};

  //Create the task to keep the GPIO outputs updated to reflect the MPBs states
   xReturned = xTaskCreatePinnedToCore(
      dmpsOutputTsk,  //Callback function/task to be called
      "SwitchOutputsControlTask",  //Name of the task
      1716,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  //Pointer to the parameters for the function to work with
      configTIMER_TASK_PRIORITY,  //Priority level given to the task: use the same as the Software Timers priority level      
      &dmpsOutputTskHdl, //Task handle
      xPortGetCoreID() //Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
   );
   if(xReturned != pdPASS)
      Error_Handler();

  //Create the task to resume every time the the MPB enters the On state and blocks when enters the Off state
   xReturned = xTaskCreatePinnedToCore(
      dmpsActWhlOnTsk,  //Callback function/task to be called
      "ExecWhileOnTask",  //Name of the task
      1024,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
      NULL,  //Pointer to the parameters for the function to work with
      configTIMER_TASK_PRIORITY,  //Priority level given to the task: use the same as the Software Timers priority level      
      &dmpsActWhlOnTskHndl, //Task handle
      xPortGetCoreID() //Run in the App Core if it's a dual core mcu (ESP-FreeRTOS specific)
   );
   if(xReturned != pdPASS)
      Error_Handler();
   vTaskSuspend(dmpsActWhlOnTskHndl);

   //Create the timer to keep the isEnabled state of the MPB changing to see the effects over the MPBs behavior
   enableSwpTmrHndl = xTimerCreate(
      "isEnabledSwapTimer",
      15000,
      pdTRUE,
      dmpbBttnPtr,
      swpEnableCb
   );
   if (enableSwpTmrHndl != NULL)
      xReturned = xTimerStart(enableSwpTmrHndl, portMAX_DELAY);
	if(xReturned == pdFAIL)
      Error_Handler();
   
   dmpbBttn.setIsOnDisabled(true);
   dmpbBttn.setTaskToNotify(dmpsOutputTskHdl);
   dmpbBttn.setTaskWhileOn(dmpsActWhlOnTskHndl);
   dmpbBttn.begin();

   for(;;){
      /* 
      Any  code can be placed here to execute while the MPBs are monitored 
      by the timer, and any output change will be responded by the dmpsOutputTsk
      */
   }
}

void dmpsOutputTsk(void *pvParameters){
   const uint8_t dmpbLoadPin{GPIO_NUM_21};
   const uint8_t dmpbIsDisabledPin{GPIO_NUM_18};

   pinMode(dmpbLoadPin, OUTPUT);
   pinMode(dmpbIsDisabledPin, OUTPUT);

   uint32_t mpbSttsRcvd{0};
	MpbOtpts_t mpbCurStateDcdd;

   for(;;){
		xReturned = xTaskNotifyWait(
         0x00,	//uint32_t ulBitsToClearOnEntry
         0xFFFFFFFF,	//uint32_t ulBitsToClearOnExit,
         &mpbSttsRcvd,	// uint32_t *pulNotificationValue,
         portMAX_DELAY//TickType_t xTicksToWait
		);
      if (xReturned != pdPASS)
         Error_Handler();

		mpbCurStateDcdd = otptsSttsUnpkg(mpbSttsRcvd);
      digitalWrite(dmpbLoadPin, (mpbCurStateDcdd.isOn)?HIGH:LOW);
      digitalWrite(dmpbIsDisabledPin, (mpbCurStateDcdd.isEnabled)?LOW:HIGH);
   }
}

void dmpsActWhlOnTsk(void *pvParameters){
   const uint8_t dmpbTskWhlOnPin{GPIO_NUM_16};
   
   pinMode(dmpbTskWhlOnPin, OUTPUT);

	const unsigned long int swapTimeMs{500};
	unsigned long int strtTime{0};
	unsigned long int curTime{0};
	unsigned long int elapTime{0};
	bool blinkOn {false};

	strtTime = xTaskGetTickCount() / portTICK_RATE_MS;
	for(;;){
		curTime = (xTaskGetTickCount() / portTICK_RATE_MS);
		if ((curTime - strtTime) > swapTimeMs){
			blinkOn = !blinkOn;
         digitalWrite(dmpbTskWhlOnPin, blinkOn?HIGH:LOW);
			strtTime = curTime;
		}
	}
}
//===============================>> User Tasks Implementations END

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
//===============================>> User Timers Implementations END

//===============================>> User Functions Implementations BEGIN
/**
 * @brief Error Handling function
 * 
 * Placeholder for a Error Handling function, in case of an error the execution
 * will be trapped in this endless loop
 */
void Error_Handler(){
  for(;;)
  {    
  }
  
  return;
}
//===============================>> User Functions Implementations END
