#include <Arduino.h>
#include <ButtonToSwitch_ESP32.h>

/*
  mpbToSwitch Library for
    Framework: Arduino
    Platform: ESP32

  1DbncdDlyd_1TmLtchMPBttn_InTasks.ino
  Created by Gabriel D. Goldman, August, 2023.
  Updated by Gabriel D. Goldman, August 20, 2023.
  Updated by Gabriel D. Goldman, August 27, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.

  Example file to demonstrate DbncdMPBttn and TmLtchMPBttn classes, required hardware and connections:
  _ 1 push button between GND and dbncdSwitchPin
  _ 1 push button between GND and tlmpbSwitchPin
  _ 1 led with it's corresponding resistor between GND and dbncdLoad
  _ 1 led with it's corresponding resistor between GND and tlmpLoadPin
  
  Pressing the push button connected to dbncdSwitchPin will turn the led immediately on and keep it lit while it's being pressed.
  Pressing the push button connected to tlmpbSwitchPin will turn the led on after a 50 miliseconds delay and keep it lit until the setted time is reached,
  the time will be reset if the mpb is pressed while the timer hasn't expired. In this sample 4000 milliseconds is the set time to keep the led on
  
  The input signals comming from the push buttons are processed by tasks, so the loop() function is unnecesary to keep the loads status updated. For demonstration
  purposes the loop() is removed from the task list the scheduler must provide processing time at all (using vTaskDelete())
*/

// put Types definitions here:
struct bttnAsArg{
  DbncdMPBttn* bttnArg;
  uint8_t outLoadPinArg;
};

// put function declarations here:
static void updLEDStruc(void* argp);
static void updLEDnWrnStruc(void* argp);

// put Global declarations here: 
const uint8_t tlmpbSwitchPin{GPIO_NUM_25};
const uint8_t tlmpLoadPin{GPIO_NUM_21};
const uint8_t dmpbSwitchPin{GPIO_NUM_26};
const uint8_t dmpbLoadPin{GPIO_NUM_19};

TmLtchMPBttn tlBttn (tlmpbSwitchPin, 4000, true, true, 20, 50);
DbncdMPBttn dBttn (dmpbSwitchPin);

bttnAsArg tlBttnArg {&tlBttn, tlmpLoadPin};
bttnAsArg dBttnArg {&dBttn, dmpbLoadPin};

void setup() {
  // put your setup code here, to run once:
  int app_cpu = xPortGetCoreID();
  BaseType_t rc;

  pinMode(tlmpLoadPin, OUTPUT);

  TaskHandle_t tlBttnHndl;
  tlBttn.begin();

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDStruc,  //function to be called
          "UpdTlLed",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &tlBttnArg,  //Pointer to the parameters for the function to work with, change to &tlBttnArg
          1,      //Priority level given to the task
          &tlBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(tlBttnHndl);

  pinMode(dmpbLoadPin, OUTPUT);
  dBttn.begin();
  TaskHandle_t dBttnHndl;

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDStruc,  //function to be called
          "UpdDbncLed",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &dBttnArg,  //Pointer to the parameters for the function to work with
          1,      //Priority level given to the task
          &dBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(dBttnHndl);
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
