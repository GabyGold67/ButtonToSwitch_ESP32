#include <Arduino.h>
#include <ButtonToSwitch_ESP32.h>

/*
  mpbToSwitch Library for
    Framework: Arduino
    Platform: ESP32

  1DbncdDlyd_1HntdTmLtchMPBttn_InTasks.ino
  Created by Gabriel D. Goldman, August, 2023.
  Updated by Gabriel D. Goldman, August 20, 2023.
  Updated by Gabriel D. Goldman, August 27, 2023.
  Released into the public domain in accordance with "GPL-3.0-or-later" license terms.

  Example file to demonstrate DbncdMPBttn and TmLtchMPBttn classes, required hardware and connections:
  _ 1 push button between GND and dbncdSwitchPin
  _ 1 push button between GND and tlmpbSwitchPin
  _ 1 led with it's corresponding resistor between GND and dbncdLoad
  _ 1 led with it's corresponding resistor between GND and tlmpLoadPin
  _ 1 led with it's corresponding resistor between GND and tlmpbPilotPin

  Pressing the push button connected to dbncdSwitchPin will turn the led immediately on and keep it lit while it's being pressed.
  Pressing the push button connected to tlmpbSwitchPin will turn the led on after a 50 miliseconds delay and keep it lit until the setted time is reached,
  the time will be reset if the mpb is pressed while the timer hasn't expired. In this sample 4000 milliseconds is the set time to keep the led on, and a 
  25% parameter is given as a warning setup: when the timer has only 25% of the time left a second pin will be rised to alert the time is running out.
  There is another optional pin output to manage a "Pilot Light signal", this pin will be rised when the pilotOn flag of the object is true, and is equivalent
  to the pilot lights (pilot hint) of some switches that iluminate the switch plate or through the button to make it easy to find them in darkness or make it easy to
  walk in the darkness of the hallway when the controlled light is off. The flag is raised every time the flag _isOn is false, and shows once again the flexibility of the software
  solution over the hardware solution: it would be easy and affordable to make the "pilot hint" output as a negated "On" output, but once implemented there would be no easy way
  of changing it without tampering the hardware. Here's just a matter of a parameter change.
  Also the physical implementation might set the warning and the pilot output pin to be the same without issues.
  Please note that because of that second pin needed for warning -and a third to the pilot- a different struct is defined to be passed to the corresponding task.
  In this example just the second (warning) pin is included in the struct, the third pin state is modified from outside the struct, just for showing different options.

  The input signals comming from the push buttons are processed by tasks, so the loop() function is unnecesary to keep the loads status updated. For demonstration purposes
   the loop() is removed from the task list the scheduler must provide processing time at all (using vTaskDelete())
*/

// put Types definitions here:
struct bttnAsArg{
  DbncdDlydMPBttn* bttnArg;
  uint8_t outLoadPinArg;
};

struct bttnNWarnAsArg{
  HntdTmLtchMPBttn* bttnArg;
  uint8_t outLoadPinArg;
  uint8_t outWarnPinArg;
};
// put function declarations here:
static void updLEDStruc(void* argp);
static void updLEDnWrnStruc(void* argp);

// put Global declarations here: 
const uint8_t tlmpbSwitchPin{GPIO_NUM_25};
const uint8_t tlmpLoadPin{GPIO_NUM_21};
const uint8_t dmpbSwitchPin{GPIO_NUM_26};
const uint8_t dmpbLoadPin{GPIO_NUM_19};
const uint8_t tlmpbWnngPin{GPIO_NUM_17};
const uint8_t tlmpbPilotPin{GPIO_NUM_18};

HntdTmLtchMPBttn htlBttn (tlmpbSwitchPin, 4000, 25, true, true, 20, 50);
DbncdDlydMPBttn dBttn (dmpbSwitchPin);

bttnNWarnAsArg tlBttnArg {&htlBttn, tlmpLoadPin, tlmpbWnngPin};
bttnAsArg dBttnArg {&dBttn, dmpbLoadPin};

void setup() {
  // put your setup code here, to run once:
  int app_cpu = xPortGetCoreID();
  BaseType_t rc;

  pinMode(tlmpLoadPin, OUTPUT);
  pinMode(tlmpbWnngPin, OUTPUT);
  pinMode(tlmpbPilotPin, OUTPUT);
  htlBttn.setKeepPilot(true);

  TaskHandle_t tlBttnHndl;
  htlBttn.begin();

//Task to run forever
  rc = xTaskCreatePinnedToCore(
          updLEDnWrnStruc,  //function to be called
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

//Create a Task to run forever
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

static void updLEDnWrnStruc(void* argp){
  bttnNWarnAsArg *myMPB = (bttnNWarnAsArg*)argp;
  bool prvWrnngStts {false};
  bool curWrnngStts {false};
  bool prvPltStts {false};
  bool curPltStts {true};

  for (;;){
    if (myMPB->bttnArg->getIsOn()){
      //Turn on the Load
      digitalWrite(myMPB->outLoadPinArg, HIGH);
    }
    else{
        //Turn off the Load
        digitalWrite(myMPB->outLoadPinArg, LOW);
    }
    
    curWrnngStts = myMPB->bttnArg->getWrnngOn();
    if (curWrnngStts != prvWrnngStts){
      if (curWrnngStts){
        //turn WrnngPin on
        digitalWrite(myMPB->outWarnPinArg, HIGH);
      }
      else{
        //turn WrnngPin off
        digitalWrite(myMPB->outWarnPinArg, LOW);
      }
    }
    prvWrnngStts = curWrnngStts;
    
    curPltStts = myMPB->bttnArg->getPilotOn();
    if(curPltStts != prvPltStts){
      if (curPltStts){
        //turn PilotPin on
        digitalWrite(tlmpbPilotPin, HIGH);
      }
      else{
        //turn PilotPin off
        digitalWrite(tlmpbPilotPin, LOW);
      }
    }
    prvPltStts = curPltStts;
  }
}
