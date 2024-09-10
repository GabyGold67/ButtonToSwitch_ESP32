#include <Arduino.h>
#include <ButtonToSwitch_ESP32.h>

//1DbncdDlyd_1TmVdblMPBttn_InTasks.ino
/*
  mpbToSwitch Library for
    Framework: Arduino
    Platform: ESP32

  1DbncdDlyd_1TmVdblMPBttn_InTasks.ino
  Created by Gabriel D. Goldman, August, 2023.
  Updated by Gabriel D. Goldman, November, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.

  Example file to demonstrate DbncdMPBttn and TmVdblMPBttn classes, required hardware and connections:
  _ 1 push button between GND and dbncdSwitchPin
  _ 1 push button between GND and tvlmpbSwitchPin
  _ 1 led with it's corresponding resistor between GND and dbncdLoad
  _ 1 led with it's corresponding resistor between GND and tvlmpbLoadPin

  Pressing the push button connected to dbncdSwitchPin will turn the led on after a 150 milliseconds delay and keep it lit while it's being pressed.
  Pressing the push button connected to tvlmpbSwitchPin will turn the led on immediately and keep it lit until:
  _ the setted limit time of 3000 milliseconds is reached,
  _ until it's released, 
  _ until de DbncdMPBttn is pressed,
  whichever happens first. This push button must be released to reset the timer that limits the time it sends an On signal while being pushed.
  This kind of switch avoids the tampering usually suffered by push buttons used to control limited resources, like water valves, heating devices, etc.

  The input signals comming from the push buttons are processed by tasks, so the loop() function is unnecesary to keep the loads status updated. For demonstration purposes
   the loop() is removed from the task list the scheduler must provide processing time at all (using vTaskDelete())
*/

// put Types definitions here:
struct bttnAsArg{
  DbncdMPBttn* bttnArg;
  uint8_t outLoadPinArg;
};

// put function declarations here:
static void updLEDStruc(void* argp);

// put Global declarations here: 
const uint8_t tvmpbSwitchPin{GPIO_NUM_25};
const uint8_t tvLoadPin{GPIO_NUM_21};
const uint8_t dbncdSwitchPin{GPIO_NUM_26};
const uint8_t dbncdLoadPin{GPIO_NUM_19};

TmVdblMPBttn tvTestBttn (tvmpbSwitchPin, 3000, true, true,0, 0, false);
DbncdDlydMPBttn ddTestBttn (dbncdSwitchPin, true, true, 0, 150);

bttnAsArg tvBttnArg {&tvTestBttn, tvLoadPin};
bttnAsArg ddBttnArg {&ddTestBttn, dbncdLoadPin};

void setup() {
  int app_cpu = xPortGetCoreID();
  BaseType_t rc;
  
  pinMode(tvLoadPin, OUTPUT);
  TaskHandle_t tvBttnHndl = nullptr;
  tvTestBttn.begin();

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDStruc,  //function to be called
          "UpdtvLoadPin",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &tvBttnArg,  //Pointer to the parameters for the function to work with
          1,      //Priority level given to the task
          &tvBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(tvBttnHndl);
  
  pinMode(dbncdLoadPin, OUTPUT);
  ddTestBttn.begin();
  TaskHandle_t ddBttnHndl = nullptr;

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDStruc,  //function to be called
          "UpddbncdLoadPin",  //Name of the task
          2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
          &ddBttnArg,  //Pointer to the parameters for the function to work with
          1,      //Priority level given to the task
          &ddBttnHndl, //Task handle
          app_cpu //Run in one core for demo purposes (ESP32 only)
  );
  assert(rc == pdPASS);
  assert(ddBttnHndl);
}

void loop() {
  //Do some modifications to the usual operation of the switches to try the methods

  if (ddTestBttn.getIsOn())
    tvTestBttn.setIsVoided();
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
