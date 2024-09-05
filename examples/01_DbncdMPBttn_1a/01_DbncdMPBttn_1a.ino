//01_DbncdMPBttn_1a
/**
  ******************************************************************************
  * @file	: 01_DbncdMPBttn_1a.ino
  * @brief  : Example for the ButtonToSwitch for ESP32 library DbncdMPBttn class
  *
  * The example instantiates a DbncdMPBttn object using:
  * 	- The Nucleo board user pushbutton attached to GPIO_B00
  * 	- The Nucleo board user LED attached to GPIO_A05 to visualize the isOn attribute flag status
  *
  * ### This example creates one Task:
  *
  * This simple example creates a single Task, instantiates the DbncdMPBttn object
  * in it and checks it's attribute flags locally through the getters methods.
  * When a change in the object's outputs attribute flags values is detected, it manages the
  * loads and resources that the switch turns On and Off, in this example case are
  * the output of some GPIO pins.
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
  ******************************************************************************
  */

/*
  mpbToSwitch Library for
    Framework: Arduino
    Platform: ESP32

  1DbncdDlyd_1Ltch_InTasks.ino
  Created by Gabriel D. Goldman, August, 2023.
  Updated by Gabriel D. Goldman, August, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.

  Example file to demonstrate DbncdDlydMPBttn and LtchMPBttn classes, required hardware and connections:
  _ 1 push button between GND and dbncdSwitchPin
  _ 1 push button between GND and ltchSwitchPin
  _ 1 led with it's corresponding resistor between GND and dbncdLoad
  _ 1 led with it's corresponding resistor between GND and ltchdLoad

  Pressing the push button connected to dbncdSwitchPin will turn on the led after the 250 milliseconds delay setted and keep it lit while it's being pressed.
  Pressing the push button connected to ltchdSwitchPin will turn the led on and keep it lit until it is released, and until the same push button is 
  pressed once again, effectively making a latch button switch out of a push button.

  The input signals comming from the push buttons are processed by tasks, so the loop() function is unnecesary to keep the loads status updated. For demonstration purposes
   the loop() is removed from the task list the scheduler must provide processing time at all (using vTaskDelete())
*/

#include <Arduino.h>
#include <ButtonToSwitch_ESP32.h>

// put Types definitions here:
struct bttnAsArg{
  DbncdMPBttn* bttnArg;
  uint8_t outLoadPinArg;
};

// put function declarations here:
static void updLEDStruc(void* argp);

// put Global declarations here: 
const uint8_t ltchSwitchPin{GPIO_NUM_25};
const uint8_t ltchLoadPin{GPIO_NUM_21};
const uint8_t dbncdSwitchPin{GPIO_NUM_26};
const uint8_t dbncdLoadPin{GPIO_NUM_19};

TgglLtchMPBttn latchTestBttn (ltchSwitchPin);   //Create a latched button (toggle button) by instantiating the class
DbncdDlydMPBttn ddTestBttn (dbncdSwitchPin, true, true, 0, 250);  //Create a debounced delayed button by instantiating the class

bttnAsArg latchTestBttnArg {&latchTestBttn, ltchLoadPin}; //Struct that will be passed as an argument to the task that keeps the led status (on/off) updated
bttnAsArg ddTestBttnArg {&ddTestBttn, dbncdLoadPin};    //Struct that will be passed as an argument to the task that keeps the led status (on/off) updated

void setup() {
  int app_cpu = xPortGetCoreID();
  BaseType_t rc;
  
  pinMode(ltchLoadPin, OUTPUT);
  TaskHandle_t latchTestBttnHndl = nullptr;
  latchTestBttn.begin();

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDStruc,  //function to be called
          "UpdltchLoadPin",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &latchTestBttnArg,  //Pointer to the parameters for the function to work with
          1,      //Priority level given to the task
          &latchTestBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(latchTestBttnHndl);
  
  pinMode(dbncdLoadPin, OUTPUT);
  ddTestBttn.begin();
  TaskHandle_t dbncdBttnHndl = nullptr;

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDStruc,  //function to be called
          "UpddbncdLoadPin",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &ddTestBttnArg,  //Pointer to the parameters for the function to work with
          1,      //Priority level given to the task
          &dbncdBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(dbncdBttnHndl);
}

void loop() {
  //Now unneeded as all runs as independent tasks! Delete the loop() task
  vTaskDelete(nullptr);
}

// put function definitions here:
static void updLEDStruc(void* argp){
  bttnAsArg *myMPB = (bttnAsArg*)argp;
  for (;;){
    if (myMPB->bttnArg->getIsOn()){
      digitalWrite(myMPB->outLoadPinArg, HIGH);
    }
    else{
      digitalWrite(myMPB->outLoadPinArg, LOW);
    }
  }
}
