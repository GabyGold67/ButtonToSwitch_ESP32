/**
  ******************************************************************************
  * @file	: ButtonToSwitch_ESP32.cpp
  * @brief	: Source file for the ButtonToSwitch_ESP32 library classes
  *
  * @details The library implements classes that model several switch mechanisms
  * replacements out of simple push buttons or similar equivalent digital signal 
  * inputs.
  * By using just a button (a.k.a. momentary switches or momentary push buttons,
  * _**MPB**_ for short from here on) the classes implemented in this library will 
  * manage, calculate and update several parameters to **generate the embedded 
  * behavior of standard electromechanical switches**.
  *
  * @author	: Gabriel D. Goldman
  * @version v4.0.0
  * @date	: Created on: 06/11/2023
  * 		: Last modification: 28/08/2024
  * @copyright GPL-3.0 license
  *
  ******************************************************************************
  * @attention	This library was developed as part of the refactoring process for
  * an industrial machines security enforcement and productivity control
  * (hardware & firmware update). As such every class included complies **AT LEAST**
  * with the provision of the attributes and methods to make the hardware & firmware
  * replacement transparent to the controlled machines. Generic use attribute and
  * methods were added to extend the usability to other projects and application
  * environments, but no fitness nor completeness of those are given but for the
  * intended refactoring project.
  * 
  * @warning **Use of this library is under your own responsibility**
  ******************************************************************************
  */
#include <ButtonToSwitch_ESP32.h>
//===========================>> BEGIN General use Global variables
static BaseType_t errorFlag {pdFALSE};
//===========================>> END General use Global variables


DbncdMPBttn::DbncdMPBttn()
: _mpbttnPin{_InvalidPinNum}, _pulledUp{true}, _typeNO{true}, _dbncTimeOrigSett{0}
{
}

DbncdMPBttn::DbncdMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett)
: _mpbttnPin{mpbttnPin}, _pulledUp{pulledUp}, _typeNO{typeNO}, _dbncTimeOrigSett{dbncTimeOrigSett}
{

	if(mpbttnPin != _InvalidPinNum){
		String mpbPinNumStr {"00" + String(_mpbttnPin)};
		mpbPinNumStr = mpbPinNumStr.substring(mpbPinNumStr.length() - 2, 2);
		_mpbPollTmrName = "PollMpbPin" + mpbPinNumStr + "_tmr";

		if(_dbncTimeOrigSett < _stdMinDbncTime) //Best practice would impose failing the constructor (throwing an exception or building a "zombie" object)
			_dbncTimeOrigSett = _stdMinDbncTime;    //this tolerant approach taken for developers benefit, but object will be no faithful to the instantiation parameters
		_dbncTimeTempSett = _dbncTimeOrigSett;
		pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT_PULLDOWN);
	}
	else{
		_pulledUp = true;
		_typeNO = true;
		_dbncTimeOrigSett = 0;
	}
}

DbncdMPBttn::~DbncdMPBttn(){
    
    end();  // Stops the software timer associated to the object, deletes it's entry and nullyfies the handle to it befor destructing the object
}

bool DbncdMPBttn::begin(const unsigned long int &pollDelayMs) {
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (pollDelayMs > 0){
        if (!_mpbPollTmrHndl){        
            _mpbPollTmrHndl = xTimerCreate(
                _mpbPollTmrName.c_str(),  //Timer name
                pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
                pdTRUE,     //Auto-reload true
                this,       //TimerID: data passed to the callback function to work
                mpbPollCallback  //DbncdMPBttn::mpbPollCallback  //Callback function
            );
        if (_mpbPollTmrHndl != NULL){
            tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
                result = true;
        }

        }
    }

    return result;
}

void DbncdMPBttn::clrStatus(bool clrIsOn){
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    /*To Resume operations after a pause() without risking generating false "Valid presses" and "On" situations,
    several attributes must be resetted to "Start" values.
    The only important value not reseted is the _mpbFdaState, to do it call resetFda() INSTEAD of this method*/
    
    taskENTER_CRITICAL(&mux);   //ESP-IDF FreeRTOS modifies the taskENTER_CRITICAL of vanilla FreeRTOS to require a mutex as argument to avoid core to core interruptions
	_isPressed = false;
	_validPressPend = false;
	_validReleasePend = false;
	_dbncTimerStrt = 0;
	_dbncRlsTimerStrt = 0;
	if(clrIsOn){
		if(_isOn){
			_turnOff();
		}
	}
    taskEXIT_CRITICAL(&mux);
    
    return;
}

void DbncdMPBttn::clrSttChng(){
	_sttChng = false;

	return;
}

void DbncdMPBttn::disable(){

    return _setIsEnabled(false);
}

void DbncdMPBttn::enable(){

    return _setIsEnabled(true);
}

bool DbncdMPBttn::end(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if (_mpbPollTmrHndl){
   	result = pause();
      if (result){
      	tmrModResult = xTimerDelete(_mpbPollTmrHndl, portMAX_DELAY);
			if (tmrModResult == pdPASS){
				_mpbPollTmrHndl = NULL;
			}
			else{
				result = false;
			}
      }
   }

   return result;
}

const unsigned long int DbncdMPBttn::getCurDbncTime() const{

    return _dbncTimeTempSett;
}

fncPtrType DbncdMPBttn::getFnWhnTrnOff(){

	return _fnWhnTrnOff;
}

fncPtrType DbncdMPBttn::getFnWhnTrnOn(){

	return _fnWhnTrnOn;
}

const bool DbncdMPBttn::getIsEnabled() const{

    return _isEnabled;
}

const bool DbncdMPBttn::getIsOn() const {

	return _isOn;
}

const bool DbncdMPBttn::getIsOnDisabled() const{

    return _isOnDisabled;
}

const bool DbncdMPBttn::getIsPressed() const {

    return _isPressed;
}

const uint32_t DbncdMPBttn::getOtptsSttsPkgd(){

	return _otptsSttsPkg();
}

const bool DbncdMPBttn::getOutputsChange() const{

    return _outputsChange;
}

unsigned long int DbncdMPBttn::getStrtDelay(){

	return _strtDelay;
}

const TaskHandle_t DbncdMPBttn::getTaskToNotify() const{
    
    return _taskToNotifyHndl;
}

const TaskHandle_t DbncdMPBttn::getTaskWhileOn(){

	return _taskWhileOnHndl;
}

bool DbncdMPBttn::init(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett){
    bool result {false};

    if((_mpbttnPin == _InvalidPinNum) && (mpbttnPin != _InvalidPinNum)){
        if (_mpbPollTmrName == ""){
            _mpbttnPin = mpbttnPin;
            _pulledUp = pulledUp;
            _typeNO = typeNO;
            _dbncTimeOrigSett = dbncTimeOrigSett;

            String mpbPinNumStr {"00" + String(_mpbttnPin)};
            mpbPinNumStr = mpbPinNumStr.substring(mpbPinNumStr.length() - 2, 2);
            _mpbPollTmrName = "PollMpbPin" + mpbPinNumStr + "_tmr";

            if(_dbncTimeOrigSett < _stdMinDbncTime) //Best practice would impose failing the constructor (throwing an exception or building a "zombie" object)
                _dbncTimeOrigSett = _stdMinDbncTime;    //this tolerant approach taken for developers benefit, but object will be no faithful to the instantiation parameters
            _dbncTimeTempSett = _dbncTimeOrigSett;
            pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT_PULLDOWN);
            result = true;
        }
        else{
            _pulledUp = true;
            _typeNO = true;
            _dbncTimeOrigSett = 0;
        }
    }
    
    return result;
}

void DbncdMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
	DbncdMPBttn* mpbObj = (DbncdMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
	BaseType_t xReturned;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    
    taskENTER_CRITICAL(&mux);   //ESP-IDF FreeRTOS modifies the taskENTER_CRITICAL of vanilla FreeRTOS to require a mutex as argument to avoid core to core interruptions
	if(mpbObj->getIsEnabled()){
		// Input/Output signals update
		mpbObj->updIsPressed();
		// Flags/Triggers calculation & update
		mpbObj->updValidPressesStatus();
	}
	// State machine status update
	mpbObj->updFdaState();
	taskEXIT_CRITICAL(&mux);

	if (mpbObj->getOutputsChange()){	//Output changes might happen as part of the updFdaState() execution
		if(mpbObj->getTaskToNotify() != NULL){
			xReturned = xTaskNotify(
					mpbObj->getTaskToNotify(),	//TaskHandle_t of the task receiving notification
					static_cast<unsigned long>(mpbObj->getOtptsSttsPkgd()),
					eSetValueWithOverwrite	//In this specific case using eSetBits is also a valid option
					);
			 if (xReturned != pdPASS){
				 errorFlag = pdTRUE;
			 }
			 mpbObj->setOutputsChange(false);	//If the outputsChange triggers a task to treat it, here's  the flag reset, in other cases the mechanism reading the chganges must take care of the flag status
		}
	}

	return;
}

uint32_t DbncdMPBttn::_otptsSttsPkg(uint32_t prevVal){
	if(_isOn){
		prevVal |= ((uint32_t)1) << IsOnBitPos;
	}
	else{
		prevVal &= ~(((uint32_t)1) << IsOnBitPos);
	}
	if(_isEnabled){
		prevVal |= ((uint32_t)1) << IsEnabledBitPos;
	}
	else{
		prevVal &= ~(((uint32_t)1) << IsEnabledBitPos);
	}

	return prevVal;
}

bool DbncdMPBttn::pause(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if (_mpbPollTmrHndl){
      if (xTimerIsTimerActive(_mpbPollTmrHndl)){
         tmrModResult = xTimerStop(_mpbPollTmrHndl, portMAX_DELAY);
         if (tmrModResult == pdPASS)
            result = true;
      }
   }
   else{
      result = true;
   }

   return result;
}

void DbncdMPBttn::resetDbncTime(){
   setDbncTime(_dbncTimeOrigSett);

   return; 
}

void DbncdMPBttn::resetFda(){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	clrStatus();
	setSttChng();
	_mpbFdaState = stOffNotVPP;
	taskEXIT_CRITICAL(&mux);

	return;
}

bool DbncdMPBttn::resume(){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   resetFda();	//To restart in a safe situation the FDA is resetted to have all flags and timers cleaned up
	if (_mpbPollTmrHndl){
		if (xTimerIsTimerActive(_mpbPollTmrHndl) == pdFAIL){	// This enforces the timer to be stopped to let the timer be resumed, makes the method useless just to reset the timer counter
			tmrModResult = xTimerReset( _mpbPollTmrHndl, portMAX_DELAY);
			if (tmrModResult == pdPASS)
				result = true;
		}
	}

    return result;
}

bool DbncdMPBttn::setDbncTime(const unsigned long int &newDbncTime){
    bool result {true};
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

    taskENTER_CRITICAL(&mux);
    if(_dbncTimeTempSett != newDbncTime){
		 if (newDbncTime >= _stdMinDbncTime){
			  _dbncTimeTempSett = newDbncTime;
		 }
		 else{
			  result = false;
		 }
    }
    taskEXIT_CRITICAL(&mux);

    return result;
}

void DbncdMPBttn::setFnWhnTrnOffPtr(void (*newFnWhnTrnOff)()){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOff != newFnWhnTrnOff){
		_fnWhnTrnOff = newFnWhnTrnOff;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void DbncdMPBttn::setFnWhnTrnOnPtr(void (*newFnWhnTrnOn)()){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOn != newFnWhnTrnOn){
		_fnWhnTrnOn = newFnWhnTrnOn;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void DbncdMPBttn::_setIsEnabled(const bool &newEnabledValue){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_isEnabled != newEnabledValue){
		if (newEnabledValue){  //Change to Enabled = true
			_validEnablePend = true;
			if(_validDisablePend)
				_validDisablePend = false;
		}
		else{	//Change to Enabled = false  (i.e. Disabled)
			_validDisablePend = true;
			if(_validEnablePend)
				_validEnablePend = false;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void DbncdMPBttn::setIsOnDisabled(const bool &newIsOnDisabled){
   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_isOnDisabled != newIsOnDisabled){
		_isOnDisabled = newIsOnDisabled;
		if(!_isEnabled){
			if(_isOn != _isOnDisabled){
				if(_isOnDisabled){
					_turnOn();
				}
				else{
					_turnOff();
				}
			}
		}
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void DbncdMPBttn::setOutputsChange(bool newOutputsChange){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_outputsChange != newOutputsChange)
   	_outputsChange = newOutputsChange;
	taskEXIT_CRITICAL(&mux);

   return;
}

void DbncdMPBttn::setSttChng(){
	_sttChng = true;

	return;
}

void DbncdMPBttn::setTaskToNotify(const TaskHandle_t &newTaskHandle){
	eTaskState taskWhileOnStts{};
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

   taskENTER_CRITICAL(&mux);
   if(_taskToNotifyHndl != newTaskHandle){
      if(_taskToNotifyHndl != NULL){
         taskWhileOnStts = eTaskGetState(_taskToNotifyHndl);
         if (taskWhileOnStts != eSuspended){
            if(taskWhileOnStts != eDeleted){
               vTaskSuspend(_taskToNotifyHndl);
               _taskToNotifyHndl = NULL;
            }
         }
      }
      if (newTaskHandle != NULL){
         _taskToNotifyHndl = newTaskHandle;
      }
   }
   taskEXIT_CRITICAL(&mux);

    return;
}

void DbncdMPBttn::setTaskWhileOn(const TaskHandle_t &newTaskHandle){
	eTaskState taskWhileOnStts{};
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_taskWhileOnHndl != newTaskHandle){
		if(_taskWhileOnHndl != NULL){
			taskWhileOnStts = eTaskGetState(_taskWhileOnHndl);
			if (taskWhileOnStts != eSuspended){
				if(taskWhileOnStts != eDeleted){
					vTaskSuspend(_taskWhileOnHndl);
					_taskWhileOnHndl = NULL;
				}
			}
		}
		if (newTaskHandle != NULL){
			_taskWhileOnHndl = newTaskHandle;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void DbncdMPBttn::_turnOff(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	if(_isOn){
		//---------------->> Tasks related actions
		if(_taskWhileOnHndl != NULL){
			eTaskState taskWhileOnStts{eTaskGetState(_taskWhileOnHndl)};
			if (taskWhileOnStts != eSuspended){
				if(taskWhileOnStts != eDeleted){
					vTaskSuspend(_taskWhileOnHndl);
				}
			}
		}
		//---------------->> Functions related actions
		if(_fnWhnTrnOff != nullptr){
			_fnWhnTrnOff();
		}
	}

	taskENTER_CRITICAL(&mux);
	if(_isOn){
		//---------------->> Flags related actions
		_isOn = false;
		_outputsChange = true;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void DbncdMPBttn::_turnOn(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	if(!_isOn){
		//---------------->> Tasks related actions
		if(_taskWhileOnHndl != NULL){
			eTaskState taskWhileOnStts{eTaskGetState(_taskWhileOnHndl)};
			if(taskWhileOnStts != eDeleted){
				if (taskWhileOnStts == eSuspended){
					vTaskResume(_taskWhileOnHndl);
				}
			}
		}
		//---------------->> Functions related actions
		if(_fnWhnTrnOn != nullptr){
			_fnWhnTrnOn();
		}
	}

	taskENTER_CRITICAL(&mux);
	if(!_isOn){
		//---------------->> Flags related actions
		_isOn = true;
		_outputsChange = true;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void DbncdMPBttn::updFdaState(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	switch(_mpbFdaState){
		case stOffNotVPP:
			//In: >>---------------------------------->>
			if(_sttChng){
				clrStatus(true);
				clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validPressPend){
				_mpbFdaState = stOffVPP;
				setSttChng();	//Set flag to execute exiting OUT code
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;
				setSttChng();	//Set flag to execute exiting OUT code
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffVPP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!_isOn){
				_turnOn();
			}
			_validPressPend = false;
			_mpbFdaState = stOn;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOn:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validReleasePend){
				_mpbFdaState = stOnVRP;
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;
				setSttChng();	//Set flag to execute exiting OUT code
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnVRP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_isOn){
				_turnOff();
			}
			_validReleasePend = false;
			_mpbFdaState = stOffNotVPP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stDisabled:
			//In: >>---------------------------------->>
			if(_sttChng){
				if(_isOn != _isOnDisabled){
					if(_isOn){
						_turnOff();
					}
					else{
						_turnOn();
					}
				}
				clrStatus(false);	//Clears all flags and timers, _isOn value will not be affected
				_isEnabled = false;
				if(!_outputsChange)
					_outputsChange = true;
				_validDisablePend = false;
				clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validEnablePend){
				if(_isOn){
					_turnOff();
				}
				_isEnabled = true;
				_validEnablePend = false;
				if(!_outputsChange)
					_outputsChange = true;
			}
			if(_isEnabled && !updIsPressed()){	//The stDisabled status will be kept until the MPB is released for security reasons
				_mpbFdaState = stOffNotVPP;
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){
				clrStatus(true);
			}	// Execute this code only ONCE, when exiting this state
			break;

	default:
		break;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

bool DbncdMPBttn::updIsPressed(){
    /*To be 'pressed' the conditions are:
    1) For NO == true
        a)  _pulledUp == false ==> digitalRead == HIGH
        b)  _pulledUp == true ==> digitalRead == LOW
    2) For NO == false
        a)  _pulledUp == false ==> digitalRead == LOW
        b)  _pulledUp == true ==> digitalRead == HIGH
    */
    bool result {false};
    bool tmpPinLvl {digitalRead(_mpbttnPin)};
    
    if (_typeNO == true){
        //For NO MPBs
        if (_pulledUp == false){
            if (tmpPinLvl == HIGH)
                result = true;
        }
        else{
            if (tmpPinLvl == LOW)
                result = true;
        }
    }
    else{
        //For NC MPBs
        if (_pulledUp == false){
            if (tmpPinLvl == LOW)
                result = true;
        }
        else{
            if (tmpPinLvl == HIGH)
                result = true;
        }
    }    
    _isPressed = result;

    return _isPressed;
}

bool DbncdMPBttn::updValidPressesStatus(){
	if(_isPressed){
		if(_dbncRlsTimerStrt != 0)
			_dbncRlsTimerStrt = 0;
		if(!_prssRlsCcl){
			if(_dbncTimerStrt == 0){    //This is the first detection of the press event
				_dbncTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;	//Started to be pressed
			}
			else{
				if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncTimerStrt) >= (_dbncTimeTempSett + _strtDelay)){
					_validPressPend = true;
					_validReleasePend = false;
					_prssRlsCcl = true;
				}
			}
		}
	}
	else{
		if(_dbncTimerStrt != 0)
			_dbncTimerStrt = 0;
		if(_prssRlsCcl){
			if(_dbncRlsTimerStrt == 0){    //This is the first detection of the release event
				_dbncRlsTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;	//Started to be UNpressed
			}
			else{
				if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncRlsTimerStrt) >= (_dbncRlsTimeTempSett)){
					_validReleasePend = true;
					_prssRlsCcl = false;
				}
			}
		}
	}

	return (_validPressPend||_validReleasePend);
}

//=========================================================================> Class methods delimiter

DbncdDlydMPBttn::DbncdDlydMPBttn()
:DbncdMPBttn()
{
}

DbncdDlydMPBttn::DbncdDlydMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:DbncdMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett)
{
   _strtDelay = strtDelay;
}

bool DbncdDlydMPBttn::init(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay){
    bool result {false};

    result = DbncdMPBttn::init(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett);
    if (result)
        setStrtDelay(strtDelay);

    return result;
}

void DbncdDlydMPBttn::setStrtDelay(const unsigned long int &newStrtDelay){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
   if(_strtDelay != newStrtDelay)
      _strtDelay = newStrtDelay;
   taskEXIT_CRITICAL(&mux);

   return;
}

//=========================================================================> Class methods delimiter

LtchMPBttn::LtchMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:DbncdDlydMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{
}

bool LtchMPBttn::begin(const unsigned long int &pollDelayMs){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if (pollDelayMs > 0){
      if (!_mpbPollTmrHndl){        
         _mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName.c_str(),  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback function to work                
            mpbPollCallback   // LtchMPBttn::mpbPollCallback   //Callback function
         );
         if (_mpbPollTmrHndl != NULL){
            tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
               result = true;
         }
      }
   }

    return result;
}

void LtchMPBttn::clrStatus(bool clrIsOn){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	_isLatched = false;
	_validUnlatchPend = false;
	_validUnlatchRlsPend = false;
	DbncdMPBttn::clrStatus(clrIsOn);
	taskEXIT_CRITICAL(&mux);

	return;
}

const bool LtchMPBttn::getIsLatched() const{

	return _isLatched;
}

bool LtchMPBttn::getTrnOffASAP(){

	return _trnOffASAP;
}

const bool LtchMPBttn::getUnlatchPend() const{

	return _validUnlatchPend;
}

const bool LtchMPBttn::getUnlatchRlsPend() const{

	return _validUnlatchRlsPend;
}

void LtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
    LtchMPBttn* mpbObj = (LtchMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
    portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

    taskENTER_CRITICAL(&mux);
    if(mpbObj->getIsEnabled()){
		// Input/Output signals update
		mpbObj->updIsPressed();
		// Flags/Triggers calculation & update
		mpbObj->updValidPressesStatus();
		mpbObj->updValidUnlatchStatus();
 	}
	// State machine state update
	mpbObj->updFdaState();
	taskEXIT_CRITICAL(&mux);

	//Outputs update based on outputsChange flag
	if (mpbObj->getOutputsChange()){
		if(mpbObj->getTaskToNotify() != NULL){
			xTaskNotify(
					mpbObj->getTaskToNotify(),	//TaskHandle_t of the task receiving notification
					static_cast<unsigned long>(mpbObj->getOtptsSttsPkgd()),
					eSetValueWithOverwrite
			);
			mpbObj->setOutputsChange(false);
		}
	}

	return;
}

void LtchMPBttn::setTrnOffASAP(const bool &newVal){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_trnOffASAP != newVal){
		_trnOffASAP = newVal;
	}
	taskEXIT_CRITICAL(&mux);

   return;
}

void LtchMPBttn::setUnlatchPend(const bool &newVal){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_validUnlatchPend != newVal)
		_validUnlatchPend = newVal;
	taskEXIT_CRITICAL(&mux);

	return;
}

void LtchMPBttn::setUnlatchRlsPend(const bool &newVal){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_validUnlatchRlsPend != newVal)
		_validUnlatchRlsPend = newVal;
	taskEXIT_CRITICAL(&mux);

	return;
}

bool LtchMPBttn::unlatch(){
	bool result{false};
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_isLatched){
		setUnlatchPend(true);
		setUnlatchRlsPend(true);
		result = true;
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

void LtchMPBttn::updFdaState(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	switch(_mpbFdaState){
		case stOffNotVPP:
			//In: >>---------------------------------->>
			if(_sttChng){
				clrStatus(true);
				stOffNotVPP_In();
				clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validPressPend){
				_mpbFdaState = stOffVPP;
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;	// For this stDisabled entry, the only flags that might be affected are _ validPressPend and (unlikely) _validReleasePend
				setSttChng();	//Set flag to execute exiting OUT code
			}
			//Out: >>---------------------------------->>
			if(_sttChng){
				stOffNotVPP_Out();
			}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffVPP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!_isOn){
				_turnOn();
			}
			_validPressPend = false;
			_mpbFdaState = stOnNVRP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){
				stOffVPP_Out();	//This function starts the latch timer here... to be considered if the MPB release must be the starting point Gaby
			}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnNVRP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			stOnNVRP_Do();
			if(_validReleasePend){
				_mpbFdaState = stOnVRP;
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;
				setSttChng();	//Set flag to execute exiting OUT code
			}

			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnVRP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_validReleasePend = false;
			if(!_isLatched)
				_isLatched = true;
			_mpbFdaState = stLtchNVUP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stLtchNVUP:	//From this state on different unlatch sources might make sense
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			stLtchNVUP_Do();
			if(_validUnlatchPend){
				_mpbFdaState = stLtchdVUP;
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;
				setSttChng();	//Set flag to execute exiting OUT code
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stLtchdVUP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_trnOffASAP){
				if(_isOn){
					_turnOff();
				}
			}
			_mpbFdaState = stOffVUP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffVUP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_validUnlatchPend = false;	// This is a placeholder for updValidUnlatchStatus() implemented in each subclass
			_mpbFdaState = stOffNVURP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffNVURP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validUnlatchRlsPend){
				_mpbFdaState = stOffVURP;
				setSttChng();
			}
			stOffNVURP_Do();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffVURP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_validUnlatchRlsPend = false;
			if(_isOn){
				_turnOff();
			}
			if(_isLatched)
				_isLatched = false;
			if(_validPressPend)
				_validPressPend = false;
			if(_validReleasePend)
				_validReleasePend = false;

			_mpbFdaState = stOffNotVPP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){
				stOffVURP_Out();
			}	// Execute this code only ONCE, when exiting this state
			break;

		case stDisabled:
			//In: >>---------------------------------->>
			if(_sttChng){
				if(_isOn != _isOnDisabled){
					if(_isOn){
						_turnOff();
					}
					else{
						_turnOn();
					}
				}
				clrStatus(false);	//Clears all flags and timers, _isOn value will not be affected
				stDisabled_In();
				_validDisablePend = false;
				_isEnabled = false;
				_outputsChange = true;
				clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validEnablePend){
				if(_isOn){
					_turnOff();
				}
				_isEnabled = true;
				_validEnablePend = false;
				_outputsChange = true;
			}
			if(_isEnabled && !updIsPressed()){	//The stDisabled status will be kept until the MPB is released for security reasons
				_mpbFdaState = stOffNotVPP;
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){
				clrStatus(true);
				stDisabled_Out();
			}	// Execute this code only ONCE, when exiting this state
			break;

	default:
		break;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

//=========================================================================> Class methods delimiter

TgglLtchMPBttn::TgglLtchMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{
}

void TgglLtchMPBttn::stOffNVURP_Do(){
	//This method is invoked exclusively from the updFdaState, no need to declare it critical section
	if(_validDisablePend){
		if(_validUnlatchRlsPend)
			_validUnlatchRlsPend = false;
		_mpbFdaState = stDisabled;
		setSttChng();	//Set flag to execute exiting OUT code
	}

	return;
}

void TgglLtchMPBttn::updValidUnlatchStatus(){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_isLatched){
		if(_validPressPend){
			_validUnlatchPend = true;
			_validPressPend = false;
		}
		if(_validReleasePend){
			_validUnlatchRlsPend = true;
			_validReleasePend = false;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

//=========================================================================> Class methods delimiter

TmLtchMPBttn::TmLtchMPBttn(const uint8_t &mpbttnPin, const unsigned long int &actTime, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _srvcTime{actTime}
{
    if(_srvcTime < _MinSrvcTime)    //Best practice would impose failing the constructor (throwing an exception or building a "zombie" object)
        _srvcTime = _MinSrvcTime;    //this tolerant approach taken for developers benefit, but object will be no faithful to the instantiation parameters

}

void TmLtchMPBttn::clrStatus(bool clrIsOn){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	_srvcTimerStrt = 0;
	LtchMPBttn::clrStatus(clrIsOn);
	taskEXIT_CRITICAL(&mux);

	return;
}

const unsigned long int TmLtchMPBttn::getSrvcTime() const{

    return _srvcTime;
}

bool TmLtchMPBttn::setSrvcTime(const unsigned long int &newSrvcTime){
	bool result {true};
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

   taskENTER_CRITICAL(&mux);
	if (_srvcTime != newSrvcTime){
		if (newSrvcTime >= _MinSrvcTime){  //The minimum activation time is _minActTime milliseconds
			_srvcTime = newSrvcTime;
		}
		else{
			result = false;
		}
   }
	taskEXIT_CRITICAL(&mux);

   return result;
}

void TmLtchMPBttn::setTmerRstbl(const bool &newIsRstbl){
   portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

   taskENTER_CRITICAL(&mux);
	if(_tmRstbl != newIsRstbl)
        _tmRstbl = newIsRstbl;
	taskEXIT_CRITICAL(&mux);

    return;
}

void TmLtchMPBttn::stOffNotVPP_Out(){
	_srvcTimerStrt = 0;

	return;
}

void TmLtchMPBttn::stOffVPP_Out(){
	//This method is invoked exclusively from the updFdaState, no need to declare it critical section
	_srvcTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;

	return;
}

void TmLtchMPBttn::updValidUnlatchStatus(){
	if(_isLatched){
		if(_validPressPend){
			if(_tmRstbl)
				_srvcTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
			_validPressPend = false;
		}
		if (((xTaskGetTickCount() / portTICK_RATE_MS) - _srvcTimerStrt) >= _srvcTime){
			_validUnlatchPend = true;
			_validUnlatchRlsPend = true;
		}
	}

	return;
}

//=========================================================================> Class methods delimiter

HntdTmLtchMPBttn::HntdTmLtchMPBttn(const uint8_t &mpbttnPin, const unsigned long int &actTime, const unsigned int &wrnngPrctg, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:TmLtchMPBttn(mpbttnPin, actTime, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _wrnngPrctg{wrnngPrctg}
{
    _wrnngMs = (_srvcTime * _wrnngPrctg) / 100;   
}

bool HntdTmLtchMPBttn::begin(const unsigned long int &pollDelayMs){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

   if (pollDelayMs > 0){
	if (!_mpbPollTmrHndl){
           _mpbPollTmrHndl = xTimerCreate(
              _mpbPollTmrName.c_str(),  //Timer name
              pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
              pdTRUE,     //Autoreload true
              this,       //TimerID: data passed to the callback function to work
              mpbPollCallback //HntdTmLtchMPBttn::mpbPollCallback   //Callback function
           );
           if (_mpbPollTmrHndl != NULL){
               tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
               if (tmrModResult == pdPASS)
                  result = true;
	   }
	}
   }

   return result;
}

void HntdTmLtchMPBttn::clrStatus(bool clrIsOn){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	
	//	Put here class specific sets/resets, including pilot and warning
	taskENTER_CRITICAL(&mux);
	_validWrnngSetPend = false;
	_validWrnngResetPend = false;
	_validPilotSetPend = false;
	_validPilotResetPend = false;
	TmLtchMPBttn::clrStatus(clrIsOn);
	taskEXIT_CRITICAL(&mux);

	return;
}

fncPtrType HntdTmLtchMPBttn::getFnWhnTrnOffPilot(){

	return _fnWhnTrnOffPilot;
}

fncPtrType HntdTmLtchMPBttn::getFnWhnTrnOffWrnng(){

	return _fnWhnTrnOffWrnng;
}

fncPtrType HntdTmLtchMPBttn::getFnWhnTrnOnPilot(){

	return _fnWhnTrnOnPilot;
}

fncPtrType HntdTmLtchMPBttn::getFnWhnTrnOnWrnng(){
	
	return _fnWhnTrnOnWrnng;
}

const bool HntdTmLtchMPBttn::getPilotOn() const{

    return _pilotOn;
}

const bool HntdTmLtchMPBttn::getWrnngOn() const{
    
    return _wrnngOn;
}

void HntdTmLtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
	HntdTmLtchMPBttn* mpbObj = (HntdTmLtchMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(mpbObj->getIsEnabled()){
		// Input/Output signals update
		mpbObj->updIsPressed();
		// Flags/Triggers calculation & update
		mpbObj->updValidPressesStatus();
		mpbObj->updValidUnlatchStatus();
		mpbObj->updWrnngOn();
		mpbObj->updPilotOn();
	}
 	// State machine state update
 	mpbObj->updFdaState();
 	taskEXIT_CRITICAL(&mux);

	if (mpbObj->getOutputsChange()){
		if(mpbObj->getTaskToNotify() != NULL){
			xTaskNotify(
					mpbObj->getTaskToNotify(),	//TaskHandle_t of the task receiving notification
					static_cast<unsigned long>(mpbObj->getOtptsSttsPkgd()),
					eSetValueWithOverwrite
			);
			mpbObj->setOutputsChange(false);
		}
	}

	return;
}

uint32_t HntdTmLtchMPBttn::_otptsSttsPkg(uint32_t prevVal){
	prevVal = DbncdMPBttn::_otptsSttsPkg(prevVal);
	if(_pilotOn){
		prevVal |= ((uint32_t)1) << PilotOnBitPos;
	}
	else{
		prevVal &= ~(((uint32_t)1) << PilotOnBitPos);
	}
	if(_wrnngOn){
		prevVal |= ((uint32_t)1) << WrnngOnBitPos;
	}
	else{
		prevVal &= ~(((uint32_t)1) << WrnngOnBitPos);
	}

	return prevVal;
}

void HntdTmLtchMPBttn::setFnWhnTrnOffPilotPtr(void(*newFnWhnTrnOff)()){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOffPilot != newFnWhnTrnOff){
		_fnWhnTrnOffPilot = newFnWhnTrnOff;
	}
	taskEXIT_CRITICAL(&mux);

	return;	
}

void HntdTmLtchMPBttn::setFnWhnTrnOffWrnngPtr(void(*newFnWhnTrnOff)()){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOffWrnng != newFnWhnTrnOff){
		_fnWhnTrnOffWrnng = newFnWhnTrnOff;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void HntdTmLtchMPBttn::setFnWhnTrnOnPilotPtr(void(*newFnWhnTrnOn)()){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOnPilot != newFnWhnTrnOn){
		_fnWhnTrnOnPilot = newFnWhnTrnOn;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void HntdTmLtchMPBttn::setFnWhnTrnOnWrnngPtr(void(*newFnWhnTrnOn)()){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOnWrnng != newFnWhnTrnOn){
		_fnWhnTrnOnWrnng = newFnWhnTrnOn;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void HntdTmLtchMPBttn::setKeepPilot(const bool &newKeepPilot){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_keepPilot != newKeepPilot)
		_keepPilot = newKeepPilot;
	taskEXIT_CRITICAL(&mux);

	return;
}

bool HntdTmLtchMPBttn::setSrvcTime(const unsigned long int &newSrvcTime){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	bool result {true};

	taskENTER_CRITICAL(&mux);
	if (newSrvcTime != _srvcTime){
		result = TmLtchMPBttn::setSrvcTime(newSrvcTime);
		if (result)
			_wrnngMs = (_srvcTime * _wrnngPrctg) / 100;  //If the _srvcTime was changed, the _wrnngMs must be updated as it's a percentage of the first
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

bool HntdTmLtchMPBttn::setWrnngPrctg (const unsigned int &newWrnngPrctg){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	bool result{false};

	taskENTER_CRITICAL(&mux);
	if(_wrnngPrctg != newWrnngPrctg){
		if(newWrnngPrctg <= 100){
			_wrnngPrctg = newWrnngPrctg;
			_wrnngMs = (_srvcTime * _wrnngPrctg) / 100;
			result = true;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

void HntdTmLtchMPBttn::stDisabled_In(){
	//This method is invoked exclusively from the updFdaState, no need to declare it critical section
	if(_validWrnngSetPend)
		_validWrnngSetPend = false;
	if(_validWrnngResetPend)
		_validWrnngResetPend = false;
	if(_wrnngOn){
		_turnOffWrnng();	// _wrnngOn = false;
		_outputsChange = true;
	}

	if(_validPilotSetPend)
		_validPilotSetPend = false;
	if(_validPilotResetPend)
		_validPilotResetPend = false;
	if(_keepPilot && !_isOnDisabled && !_pilotOn){
		_turnOnPilot();	// _pilotOn = true;
		_outputsChange = true;
	}
	else if(_pilotOn == true)
		_turnOffPilot();	// _pilotOn = false;

	return;
}

void HntdTmLtchMPBttn::stLtchNVUP_Do(){
	//This method is invoked exclusively from the updFdaState, no need to declare it critical section
	if(_validWrnngSetPend){
		_turnOnWrnng();	// _wrnngOn = true;
		_validWrnngSetPend = false;
		_outputsChange = true;
	}
	if(_validWrnngResetPend){
		_turnOffWrnng();	// _wrnngOn = false;
		_validWrnngResetPend = false;
		_outputsChange = true;
	}

	return;
}

void HntdTmLtchMPBttn::stOffNotVPP_In(){
	//This method is invoked exclusively from the updFdaState, no need to declare it critical section
	if(_keepPilot){
		if(!_pilotOn){
			_turnOnPilot();	// _pilotOn = true;
			_outputsChange = true;
		}
	}
	if(_wrnngOn){
		_turnOffWrnng();	// _wrnngOn = false;
		_outputsChange = true;
	}

	return;
}

void HntdTmLtchMPBttn::stOffVPP_Out(){
	//This method is invoked exclusively from the updFdaState, no need to declare it critical section
	TmLtchMPBttn::stOffVPP_Out();
	if(_pilotOn){
		_turnOffPilot();	// _pilotOn = false;
		_outputsChange = true;
	}

	return;
}

void HntdTmLtchMPBttn::stOnNVRP_Do(){
	//This method is invoked exclusively from the updFdaState, no need to declare it critical section
	if(_validWrnngSetPend){
		_turnOnWrnng();	// _wrnngOn = true;
		_validWrnngSetPend = false;
		_outputsChange = true;
	}
	if(_validWrnngResetPend){
		_turnOffWrnng();	// _wrnngOn = false;
		_validWrnngResetPend = false;
		_outputsChange = true;
	}

	return;
}
	
void HntdTmLtchMPBttn::_turnOffPilot(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
   

	if(_pilotOn){
		//---------------->> Tasks related actions
		/*
		* The Task Resume/Task Suspend while an On/Off state is kept has many shortcomings. Implement it here if needed.
		*/
		//---------------->> Functions related actions
		if(_fnWhnTrnOffPilot != nullptr){
			_fnWhnTrnOffPilot();
		}
	}
	taskENTER_CRITICAL(&mux);
	if(_pilotOn){
		//---------------->> Flags related actions
		_pilotOn = false;
		_outputsChange = true;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void HntdTmLtchMPBttn::_turnOffWrnng(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	if(_wrnngOn){
		//---------------->> Tasks related actions
		/*
		* The Task Resume/Task Suspend while an On/Off state is kept has many shortcomings. Implement it here if needed.
		*/
		//---------------->> Functions related actions
		if(_fnWhnTrnOffWrnng != nullptr){
			_fnWhnTrnOffWrnng();
		}
	}
	taskENTER_CRITICAL(&mux);
	if(_wrnngOn){
		//---------------->> Flags related actions
		_wrnngOn = false;
		_outputsChange = true;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void HntdTmLtchMPBttn::_turnOnPilot(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	if(!_pilotOn){
		//---------------->> Tasks related actions
		/*
		 * The Task Resume/Task Suspend while an On/Off state is kept has many shortcomings. Implement it here if needed.
		*/
		//---------------->> Functions related actions
		if(_fnWhnTrnOnPilot != nullptr){
			_fnWhnTrnOnPilot();
		}
	}
	taskENTER_CRITICAL(&mux);
	if(!_pilotOn){
		//---------------->> Flags related actions
		_pilotOn = true;
		_outputsChange = true;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void HntdTmLtchMPBttn::_turnOnWrnng(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	if(!_wrnngOn){
		//---------------->> Tasks related actions
		/*
		 * The Task Resume/Task Suspend while an On/Off state is kept has many shortcomings. Implement it here if needed.
		*/
		//---------------->> Functions related actions
		if(_fnWhnTrnOnWrnng != nullptr){
			_fnWhnTrnOnWrnng();
		}
	}
	taskENTER_CRITICAL(&mux);
	if(!_wrnngOn){
		//---------------->> Flags related actions
		_wrnngOn = true;
		_outputsChange = true;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

bool HntdTmLtchMPBttn::updPilotOn(){
	if (_keepPilot){
		if(!_isOn && !_pilotOn){
			_validPilotSetPend = true;
			_validPilotResetPend = false;
		}
		else if(_isOn && _pilotOn){
			_validPilotResetPend = true;
			_validPilotSetPend = false;
		}
	}
	else{
		if(_pilotOn){
			_validPilotResetPend = true;
			_validPilotSetPend = false;
		}
	}

	return _pilotOn;
}

bool HntdTmLtchMPBttn::updWrnngOn(){
	if(_wrnngPrctg > 0){
		if (_isOn && _isEnabled){	//The _isEnabled evaluation is done to avoid computation of flags that will be ignored if the MPB is disablee
			if (((xTaskGetTickCount() / portTICK_RATE_MS) - _srvcTimerStrt) >= (_srvcTime - _wrnngMs)){
				if(_wrnngOn == false){
					_validWrnngSetPend = true;
					_validWrnngResetPend = false;
				}
			}
			else if(_wrnngOn == true){
				_validWrnngResetPend = true;
				_validWrnngSetPend = false;
			}
		}
		else if(_wrnngOn == true){
			_validWrnngResetPend = true;
			_validWrnngSetPend = false;
		}
	}

	return _wrnngOn;
}

//=========================================================================> Class methods delimiter

XtrnUnltchMPBttn::XtrnUnltchMPBttn(const uint8_t &mpbttnPin,  DbncdDlydMPBttn* unLtchBttn,
        const bool &pulledUp,  const bool &typeNO,  const unsigned long int &dbncTimeOrigSett,  const unsigned long int &strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _unLtchBttn{unLtchBttn}
{
}

XtrnUnltchMPBttn::XtrnUnltchMPBttn(const uint8_t &mpbttnPin,  
        const bool &pulledUp,  const bool &typeNO,  const unsigned long int &dbncTimeOrigSett,  const unsigned long int &strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{
}

bool XtrnUnltchMPBttn::begin(const unsigned long int &pollDelayMs){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if (pollDelayMs > 0){
		if (!_mpbPollTmrHndl){
			_mpbPollTmrHndl = xTimerCreate(
				_mpbPollTmrName.c_str(),  //Timer name
				pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
				pdTRUE,     //Auto-reload true
				this,       //TimerID: data passed to the callback function to work
				mpbPollCallback
			);
		}
		if (_mpbPollTmrHndl != NULL){
			tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
			if (tmrModResult == pdPASS){
				if(_unLtchBttn != nullptr)
					result = _unLtchBttn->begin();
				else
					result = true;
			}
		}
   }

   return result;
}

void XtrnUnltchMPBttn::clrStatus(bool clrIsOn){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	_xtrnUnltchPRlsCcl = false;
	LtchMPBttn::clrStatus(clrIsOn);
	taskEXIT_CRITICAL(&mux);

	return;
}

void XtrnUnltchMPBttn::stOffNVURP_Do(){
	if(_validDisablePend){
		if(_validUnlatchRlsPend)
			_validUnlatchRlsPend = false;
		if(_xtrnUnltchPRlsCcl)
			_xtrnUnltchPRlsCcl = false;
		_mpbFdaState = stDisabled;
		setSttChng();
	}

	return;
}

void XtrnUnltchMPBttn::updValidUnlatchStatus(){
	if(_unLtchBttn != nullptr){
		if(_isLatched){
			if (_unLtchBttn->getIsOn() && !_xtrnUnltchPRlsCcl){
				_validUnlatchPend = true;
				_xtrnUnltchPRlsCcl = true;
			}
			if(!_unLtchBttn->getIsOn() && _xtrnUnltchPRlsCcl){
				_validUnlatchRlsPend = true;
				_xtrnUnltchPRlsCcl = false;
			}
		}
		else{
			if(_xtrnUnltchPRlsCcl)
				_xtrnUnltchPRlsCcl = false;
		}
	}
	
	return;
}

//=========================================================================> Class methods delimiter

DblActnLtchMPBttn::DblActnLtchMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{
}

DblActnLtchMPBttn::~DblActnLtchMPBttn()
{
}

bool DblActnLtchMPBttn::begin(const unsigned long int &pollDelayMs) {
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (pollDelayMs > 0){
        if (!_mpbPollTmrHndl){
            _mpbPollTmrHndl = xTimerCreate(
            	_mpbPollTmrName.c_str(),  //Timer name
               pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
               pdTRUE,     //Auto-reload true
               this,       //TimerID: data passed to the callback function to work
               mpbPollCallback	  //Callback function
				);
            if (_mpbPollTmrHndl != NULL){
               tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
            	if (tmrModResult == pdPASS)
                  result = true;
            }
        }
    }

    return result;
}

void DblActnLtchMPBttn::clrStatus(bool clrIsOn){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	_scndModTmrStrt = 0;
	_validScndModPend = false;
	if(clrIsOn)
		if(_isOnScndry)
			_turnOffScndry();
	LtchMPBttn::clrStatus(clrIsOn);
	taskEXIT_CRITICAL(&mux);

	return;
}

fncPtrType DblActnLtchMPBttn::getFnWhnTrnOffScndry(){

	return _fnWhnTrnOffScndry;
}

fncPtrType DblActnLtchMPBttn::getFnWhnTrnOnScndry(){

	return _fnWhnTrnOnScndry;
}

bool DblActnLtchMPBttn::getIsOnScndry(){

	return _isOnScndry;
}

unsigned long DblActnLtchMPBttn::getScndModActvDly(){

	return _scndModActvDly;
}

const TaskHandle_t DblActnLtchMPBttn::getTaskWhileOnScndry(){

	return _taskWhileOnScndryHndl;
}

void DblActnLtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
	DblActnLtchMPBttn* mpbObj = (DblActnLtchMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(mpbObj->getIsEnabled()){
		// Input/Output signals update
		mpbObj->updIsPressed();
		// Flags/Triggers calculation & update
		mpbObj->updValidPressesStatus();
	}
 	// State machine state update
	mpbObj->updFdaState();
	taskEXIT_CRITICAL(&mux);

	if (mpbObj->getOutputsChange()){
		if(mpbObj->getTaskToNotify() != NULL){
			xTaskNotify(
					mpbObj->getTaskToNotify(),	//TaskHandle_t of the task receiving notification
					static_cast<unsigned long>(mpbObj->getOtptsSttsPkgd()),
					eSetValueWithOverwrite
			);
			mpbObj->setOutputsChange(false);
		}
	}

	return;
}

void DblActnLtchMPBttn::setFnWhnTrnOffScndryPtr(void (*newFnWhnTrnOff)()){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOffScndry != newFnWhnTrnOff){
		_fnWhnTrnOffScndry = newFnWhnTrnOff;
	}
	taskEXIT_CRITICAL(&mux);
	return;
}

void DblActnLtchMPBttn::setFnWhnTrnOnScndryPtr(void (*newFnWhnTrnOn)()){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	
	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOnScndry != newFnWhnTrnOn){
		_fnWhnTrnOnScndry = newFnWhnTrnOn;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

bool DblActnLtchMPBttn::setScndModActvDly(const unsigned long &newVal){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	bool result {true};

	taskENTER_CRITICAL(&mux);
	if(newVal != _scndModActvDly){
		if (newVal >= _MinSrvcTime){  //The minimum activation time is _minActTime
			_scndModActvDly = newVal;
		}
		else{
			result = false;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

void DblActnLtchMPBttn::setTaskWhileOnScndry(const TaskHandle_t &newTaskHandle){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	eTaskState taskWhileOnStts{};

	taskENTER_CRITICAL(&mux);
	if(_taskWhileOnScndryHndl != newTaskHandle){
		if(_taskWhileOnScndryHndl != NULL){
			taskWhileOnStts = eTaskGetState(_taskWhileOnScndryHndl);
			if (taskWhileOnStts != eSuspended){
				if(taskWhileOnStts != eDeleted){
					vTaskSuspend(_taskWhileOnScndryHndl);
					_taskWhileOnScndryHndl = NULL;
				}
			}
		}
		if (newTaskHandle != NULL){
			_taskWhileOnScndryHndl = newTaskHandle;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void DblActnLtchMPBttn::_turnOffScndry(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	if(_isOnScndry){
		taskENTER_CRITICAL(&mux);
		//---------------->> Flags related actions
		if(_isOnScndry){
			_isOnScndry = false;
			_outputsChange = true;
		}
		taskEXIT_CRITICAL(&mux);
		//---------------->> Tasks related actions
		if(_taskWhileOnScndryHndl != NULL){
			eTaskState taskWhileOnScndryStts{eTaskGetState(_taskWhileOnScndryHndl)};
			if (taskWhileOnScndryStts != eSuspended){
				if(taskWhileOnScndryStts != eDeleted){
					vTaskSuspend(_taskWhileOnScndryHndl);
				}
			}
		}
		//---------------->> Functions related actions
		if(_fnWhnTrnOffScndry != nullptr){
			_fnWhnTrnOffScndry();
		}
	}

	return;
}

void DblActnLtchMPBttn::_turnOnScndry(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	if(!_isOnScndry){
		taskENTER_CRITICAL(&mux);
		//---------------->> Flags related actions
		if(!_isOnScndry){
			_isOnScndry = true;
			_outputsChange = true;
		}
		taskEXIT_CRITICAL(&mux);
		//---------------->> Tasks related actions
		if(_taskWhileOnScndryHndl != NULL){
			eTaskState taskWhileOnScndryStts{eTaskGetState(_taskWhileOnScndryHndl)};
			if(taskWhileOnScndryStts != eDeleted){
				if (taskWhileOnScndryStts == eSuspended){
					vTaskResume(_taskWhileOnScndryHndl);
				}
			}
		}
		//---------------->> Functions related actions
		if(_fnWhnTrnOnScndry != nullptr){
			_fnWhnTrnOnScndry();
		}
	}

	return;
}

void DblActnLtchMPBttn::updFdaState(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	switch(_mpbFdaState){
		case stOffNotVPP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validPressPend || _validScndModPend){
				_mpbFdaState = stOffVPP;	//Start pressing timer
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;	//The MPB has been disabled
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffVPP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!_isOn){
				_turnOn();
			}
			if(_validScndModPend){
				_scndModTmrStrt = (xTaskGetTickCount() / portTICK_RATE_MS);
				_mpbFdaState = stOnStrtScndMod;
				setSttChng();
			}
			else if(_validPressPend && _validReleasePend){
				_validPressPend = false;
				_validReleasePend = false;
				_mpbFdaState = stOnMPBRlsd;
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnStrtScndMod:
			//In: >>---------------------------------->>
			if(_sttChng){
				stOnStrtScndMod_In();
				clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_mpbFdaState = stOnScndMod;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnScndMod:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!_validReleasePend){
				//Operating in Second Mode
				stOnScndMod_Do();
			}
			else{
				// MPB released, close Slider mode, move on to next state
				_mpbFdaState = stOnEndScndMod;
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;	//The MPB has been disabled
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnEndScndMod:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_scndModTmrStrt = 0;
			_validScndModPend = false;
			_mpbFdaState = stOnMPBRlsd;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){
				stOnEndScndMod_Out();
			}
			break;

		case stOnMPBRlsd:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validScndModPend){
				_scndModTmrStrt = (xTaskGetTickCount() / portTICK_RATE_MS);
				_mpbFdaState = stOnStrtScndMod;
				setSttChng();
			}
			else if(_validPressPend && _validReleasePend){
				_validPressPend = false;
				_validReleasePend = false;
				_mpbFdaState = stOnTurnOff;
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;	//The MPB has been disabled
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnTurnOff:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_turnOff();
			_mpbFdaState = stOffNotVPP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stDisabled:
			//In: >>---------------------------------->>
			if(_sttChng){
				if(_isOn != _isOnDisabled){
					if(_isOn){
						_turnOff();
					}
					else{
						_turnOn();
					}
				}
				clrStatus(false);	//Clears all flags and timers, _isOn value will not be affected
				stDisabled_In();
				_isEnabled = false;
				_validDisablePend = false;
				setOutputsChange(true);
				clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validEnablePend){
				_isEnabled = true;
				_validEnablePend = false;
				setOutputsChange(true);
			}
			if(_isEnabled && !updIsPressed()){	//The stDisabled status will be kept until the MPB is released for security reasons
				_mpbFdaState = stOffNotVPP;
				setSttChng();
			}

			//Out: >>---------------------------------->>
			if(_sttChng){
				clrStatus(true);
			}	// Execute this code only ONCE, when exiting this state
			break;
	default:
		break;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

bool DblActnLtchMPBttn::updValidPressesStatus(){
	if(_isPressed){
		if(_dbncRlsTimerStrt != 0)
			_dbncRlsTimerStrt = 0;
		if(_dbncTimerStrt == 0){    //It was not previously pressed
			_dbncTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;	//Started to be pressed
		}
		else{
			if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncTimerStrt) >= ((_dbncTimeTempSett + _strtDelay) + _scndModActvDly)){
				_validScndModPend = true;
				_validPressPend = false;
			} else if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncTimerStrt) >= (_dbncTimeTempSett + _strtDelay)){
				_validPressPend = true;
			}
			if(_validPressPend || _validScndModPend){
				_validReleasePend = false;
				_prssRlsCcl = true;
			}
		}
	}
	else{
		if(_dbncTimerStrt != 0)
			_dbncTimerStrt = 0;
		if(!_validReleasePend && _prssRlsCcl){
			if(_dbncRlsTimerStrt == 0){    //It was not previously pressed
				_dbncRlsTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;	//Started to be UNpressed
			}
			else{
				if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncRlsTimerStrt) >= (_dbncRlsTimeTempSett)){
					_validReleasePend = true;
					_prssRlsCcl = false;
				}
			}
		}
	}

	return (_validPressPend || _validScndModPend);
}

void DblActnLtchMPBttn::updValidUnlatchStatus(){
	_validUnlatchPend = true;

	return;
}

//=========================================================================> Class methods delimiter

DDlydDALtchMPBttn::DDlydDALtchMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:DblActnLtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{
}

DDlydDALtchMPBttn::~DDlydDALtchMPBttn()
{
}

void DDlydDALtchMPBttn::clrStatus(bool clrIsOn){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	
	taskENTER_CRITICAL(&mux);
	if(clrIsOn && _isOnScndry){
		_turnOffScndry();
		_outputsChange = true;
	}
	DblActnLtchMPBttn::clrStatus(clrIsOn);
	taskEXIT_CRITICAL(&mux);

	return;
}

uint32_t DDlydDALtchMPBttn::_otptsSttsPkg(uint32_t prevVal){
	prevVal = DbncdMPBttn::_otptsSttsPkg(prevVal);
	if(_isOnScndry)
		prevVal |= ((uint32_t)1) << IsOnScndryBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsOnScndryBitPos);

	return prevVal;
}

void DDlydDALtchMPBttn::stDisabled_In(){	
	if(_isOnScndry != _isOnDisabled){
		if(_isOnDisabled)
			_turnOnScndry();
		else
			_turnOffScndry();
		_outputsChange = true;
	}

	return;
}

void DDlydDALtchMPBttn::stOnEndScndMod_Out(){
	if(_isOnScndry){
		_turnOffScndry();
		_outputsChange = true;
	}

	return;
}

void DDlydDALtchMPBttn::stOnScndMod_Do(){

	return;
}

void DDlydDALtchMPBttn::stOnStrtScndMod_In(){
	if(!_isOnScndry){
		_turnOnScndry();
		_outputsChange = true;
	}

	return;
}

//=========================================================================> Class methods delimiter

SldrDALtchMPBttn::SldrDALtchMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay, const uint16_t initVal)
:DblActnLtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _initOtptCurVal{initVal}
{
	_otptCurVal = _initOtptCurVal;
}

SldrDALtchMPBttn::~SldrDALtchMPBttn()
{
}

void SldrDALtchMPBttn::clrStatus(bool clrIsOn){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	// Might the option to return the _otpCurVal to the initVal be added? To one the extreme values?
	if(clrIsOn && _isOnScndry){
		_turnOffScndry();
		_outputsChange = true;
	}

	DblActnLtchMPBttn::clrStatus(clrIsOn);
	taskEXIT_CRITICAL(&mux);

	return;
}

uint16_t SldrDALtchMPBttn::getOtptCurVal(){

	return _otptCurVal;
}

bool SldrDALtchMPBttn::getOtptCurValIsMax(){

	return (_otptCurVal == _otptValMax);
}

bool SldrDALtchMPBttn::getOtptCurValIsMin(){

	return (_otptCurVal == _otptValMin);
}

unsigned long SldrDALtchMPBttn::getOtptSldrSpd(){

	return _otptSldrSpd;
}

uint16_t SldrDALtchMPBttn::getOtptSldrStpSize(){

	return _otptSldrStpSize;
}

uint16_t SldrDALtchMPBttn::getOtptValMax(){

	return _otptValMax;
}

uint16_t SldrDALtchMPBttn::getOtptValMin(){

	return _otptValMin;
}

bool SldrDALtchMPBttn::getSldrDirUp(){

	return _curSldrDirUp;
}

uint32_t SldrDALtchMPBttn::_otptsSttsPkg(uint32_t prevVal){
	prevVal = DbncdMPBttn::_otptsSttsPkg(prevVal);
	if(_isOnScndry)
		prevVal |= ((uint32_t)1) << IsOnScndryBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsOnScndryBitPos);
	prevVal |= (((uint32_t)_otptCurVal) << OtptCurValBitPos);

	return prevVal;
}

bool SldrDALtchMPBttn::setOtptCurVal(const uint16_t &newVal){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	bool result{true};

	taskENTER_CRITICAL(&mux);
	if(_otptCurVal != newVal){
		if(newVal >= _otptValMin && newVal <= _otptValMax){
			_otptCurVal = newVal;
		}
		else{
			result = false;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

bool SldrDALtchMPBttn::setOtptSldrSpd(const uint16_t &newVal){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	bool result{true};

	taskENTER_CRITICAL(&mux);
	if(newVal != _otptSldrSpd){
		if(newVal > 0){
			_otptSldrSpd = newVal;
		}
		else{
			result = false;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

bool SldrDALtchMPBttn::setOtptSldrStpSize(const uint16_t &newVal){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	bool result{true};

	taskENTER_CRITICAL(&mux);
	if(newVal != _otptSldrStpSize){
		if((newVal > 0) && (newVal <= (_otptValMax - _otptValMin) / _otptSldrSpd)){	//If newVal == (_otptValMax - _otptValMin) the slider will work as kind of an On/Off switch
			_otptSldrStpSize = newVal;
		}
		else{
			result = false;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

bool SldrDALtchMPBttn::setOtptValMax(const uint16_t &newVal){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	bool result{true};

	taskENTER_CRITICAL(&mux);
	if(newVal != _otptValMax){
		if(newVal > _otptValMin){
			_otptValMax = newVal;
			if(_otptCurVal > _otptValMax){
				_otptCurVal = _otptValMax;
				_outputsChange = true;
			}
		}
		else{
			result = false;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

bool SldrDALtchMPBttn::setOtptValMin(const uint16_t &newVal){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	bool result{true};

	taskENTER_CRITICAL(&mux);
	if(newVal != _otptValMin){
		if(newVal < _otptValMax){
			_otptValMin = newVal;
			if(_otptCurVal < _otptValMin){
				_otptCurVal = _otptValMin;
				_outputsChange = true;
			}
		}
		else{
			result = false;
		}
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

bool SldrDALtchMPBttn::_setSldrDir(const bool &newVal){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	bool result{true};

	taskENTER_CRITICAL(&mux);
	if(newVal != _curSldrDirUp){
		if(newVal){	//Try to set new direction Up
			if(_otptCurVal != _otptValMax){
				_curSldrDirUp = true;
			}
		}
		else{		//Try to set new direction down
			if(_otptCurVal != _otptValMin){
				_curSldrDirUp = false;
			}
		}
		if(_curSldrDirUp != newVal)
			result = false;
	}
	taskEXIT_CRITICAL(&mux);

	return result;
}

bool SldrDALtchMPBttn::setSldrDirDn(){

	return _setSldrDir(false);
}

bool SldrDALtchMPBttn::setSldrDirUp(){

	return _setSldrDir(true);
}

void SldrDALtchMPBttn::setSwpDirOnEnd(const bool &newVal){
	if(_autoSwpDirOnEnd != newVal)
		_autoSwpDirOnEnd = newVal;

	return;
}

void SldrDALtchMPBttn::setSwpDirOnPrss(const bool &newVal){
	if(_autoSwpDirOnPrss != newVal)
		_autoSwpDirOnPrss = newVal;

	return;
}

void SldrDALtchMPBttn::stDisabled_In(){
	if(_isOnScndry != _isOnDisabled){
		if(_isOnDisabled)
			_turnOnScndry();
		else
			_turnOffScndry();
		_outputsChange = true;
	}

	return;
}

void SldrDALtchMPBttn::stOnEndScndMod_Out(){
	if(_isOnScndry){
		_turnOffScndry();
		_outputsChange = true;
	}

	return;
}

void SldrDALtchMPBttn::stOnScndMod_Do(){
	// Operating in Slider mode, change the associated value according to the time elapsed since last update
	//and the step size for every time unit elapsed
	uint16_t _otpStpsChng{0};
	unsigned long _sldrTmrNxtStrt{0};
	unsigned long _sldrTmrRemains{0};

	_sldrTmrNxtStrt = (xTaskGetTickCount() / portTICK_RATE_MS);
	_otpStpsChng = (_sldrTmrNxtStrt - _scndModTmrStrt) /_otptSldrSpd;
	_sldrTmrRemains = ((_sldrTmrNxtStrt - _scndModTmrStrt) % _otptSldrSpd) * _otptSldrSpd;
	_sldrTmrNxtStrt -= _sldrTmrRemains;
	_scndModTmrStrt = _sldrTmrNxtStrt;	//This ends the time management section of the state, calculating the time

	if(_curSldrDirUp){
		// The slider is moving up
		if(_otptCurVal != _otptValMax){
			if((_otptValMax - _otptCurVal) >= (_otpStpsChng * _otptSldrStpSize)){
				//The value change is in range
				_otptCurVal += (_otpStpsChng * _otptSldrStpSize);
			}
			else{
				//The value change goes out of range
				_otptCurVal = _otptValMax;
			}
			_outputsChange = true;
		}
		if(_outputsChange){
			if(_otptCurVal == _otptValMax){
				if(_autoSwpDirOnEnd == true){
					_curSldrDirUp = false;
				}
			}
		}
	}
	else{
		// The slider is moving down
		if(_otptCurVal != _otptValMin){
			if((_otptCurVal - _otptValMin) >= (_otpStpsChng * _otptSldrStpSize)){
				//The value change is in range
				_otptCurVal -= (_otpStpsChng * _otptSldrStpSize);
			}
			else{
				//The value change goes out of range
				_otptCurVal = _otptValMin;
			}
			_outputsChange = true;
		}
		if(_outputsChange){
			if(_otptCurVal == _otptValMin){
				if(_autoSwpDirOnEnd == true){
					_curSldrDirUp = true;
				}
			}
		}
	}

	return;
}

void SldrDALtchMPBttn::stOnStrtScndMod_In(){
	if(!_isOnScndry){
		_turnOnScndry();
		_outputsChange = true;
	}
if(_autoSwpDirOnPrss)
		swapSldrDir();

	return;
}

bool SldrDALtchMPBttn::swapSldrDir(){

	return _setSldrDir(!_curSldrDirUp);
}

//=========================================================================> Class methods delimiter

VdblMPBttn::VdblMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay, const bool &isOnDisabled)
:DbncdDlydMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{
	_isOnDisabled = isOnDisabled;
}

VdblMPBttn::~VdblMPBttn()
{
}

void VdblMPBttn::clrStatus(bool clrIsOn){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_isVoided){
		setIsNotVoided();
		_outputsChange = true;
	}
	DbncdMPBttn::clrStatus(clrIsOn);
	taskEXIT_CRITICAL(&mux);

	return;
}

fncPtrType VdblMPBttn::getFnWhnTrnOffVdd(){

	return _fnWhnTrnOffVdd;
}

fncPtrType VdblMPBttn::getFnWhnTrnOnVdd(){
	
	return _fnWhnTrnOnVdd;
}

bool VdblMPBttn::getFrcOtptLvlWhnVdd(){

	return _frcOtptLvlWhnVdd;
}

const bool VdblMPBttn::getIsVoided() const{

    return _isVoided;
}

bool VdblMPBttn::getStOnWhnOtpFrcd(){

	return _stOnWhnOtptFrcd;
}

void VdblMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
	VdblMPBttn* mpbObj = (VdblMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(mpbObj->getIsEnabled()){
		// Input/Output signals update
		mpbObj->updIsPressed();
		// Flags/Triggers calculation & update
		mpbObj->updValidPressesStatus();
		mpbObj->updVoidStatus();
	}
 	// State machine state update
	mpbObj->updFdaState();
	taskEXIT_CRITICAL(&mux);

	if (mpbObj->getOutputsChange()){
		if(mpbObj->getTaskToNotify() != NULL){
			xTaskNotify(
					mpbObj->getTaskToNotify(),	//TaskHandle_t of the task receiving notification
					static_cast<unsigned long>(mpbObj->getOtptsSttsPkgd()),
					eSetValueWithOverwrite
			);
			mpbObj->setOutputsChange(false);
		}
	}

	return;
}

uint32_t VdblMPBttn::_otptsSttsPkg(uint32_t prevVal){
	prevVal = DbncdMPBttn::_otptsSttsPkg(prevVal);

	if(_isVoided)
		prevVal |= ((uint32_t)1) << IsVoidedBitPos;
	else
		prevVal &= ~(((uint32_t)1) << IsVoidedBitPos);

	return prevVal;
}

void VdblMPBttn::setFnWhnTrnOffVddPtr(void(*newFnWhnTrnOff)()){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOffVdd != newFnWhnTrnOff){
		_fnWhnTrnOffVdd = newFnWhnTrnOff;
	}
	taskEXIT_CRITICAL(&mux);

	return;

}

void VdblMPBttn::setFnWhnTrnOnVddPtr(void(*newFnWhnTrnOn)()){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if (_fnWhnTrnOnVdd != newFnWhnTrnOn){
		_fnWhnTrnOnVdd = newFnWhnTrnOn;
	}
	taskEXIT_CRITICAL(&mux);

	return;

}

void VdblMPBttn::setFrcdOtptWhnVdd(const bool &newVal){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_frcOtptLvlWhnVdd != newVal)
		_frcOtptLvlWhnVdd = newVal;
	taskEXIT_CRITICAL(&mux);

	return;
}

bool VdblMPBttn::setIsNotVoided(){

	return setVoided(false);
}

bool VdblMPBttn::setIsVoided(){

	return setVoided(true);
}

void VdblMPBttn::setStOnWhnOtpFrcd(const bool &newVal){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	if(_stOnWhnOtptFrcd != newVal)
		_stOnWhnOtptFrcd = newVal;
	taskEXIT_CRITICAL(&mux);

	return;
}

bool VdblMPBttn::setVoided(const bool &newVoidValue){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	
	taskENTER_CRITICAL(&mux);
	if(_isVoided != newVoidValue){
		_isVoided = newVoidValue;
      _outputsChange = true;
	}
	taskEXIT_CRITICAL(&mux);

	return true;
}

void VdblMPBttn::stDisabled_In(){
	if(_isOn != _isOnDisabled){
		if(_isOn){
			_turnOff();
		}
		else{
			_turnOn();
		}
	}
	clrStatus(false);	//Clears all flags and timers, _isOn value will not be affected

	return;
}

void VdblMPBttn::stDisabled_Out(){
	clrStatus(true);	//Clears all flags and timers, _isOn value will be reset

	return;
}

void VdblMPBttn::_turnOffVdd(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	if(_isVoided){
		//---------------->> Tasks related actions

		//---------------->> Functions related actions
		if(_fnWhnTrnOffVdd != nullptr){
			_fnWhnTrnOffVdd();
		}
	}
	taskENTER_CRITICAL(&mux);
	if(_isVoided){
		//---------------->> Flags related actions
		_isVoided = false;
		_outputsChange = true;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void VdblMPBttn::_turnOnVdd(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	if(!_isVoided){
		//---------------->> Tasks related actions

		//---------------->> Functions related actions
		if(_fnWhnTrnOnVdd != nullptr){
			_fnWhnTrnOnVdd();
		}
	}
	taskENTER_CRITICAL(&mux);
	if(!_isVoided){
		//---------------->> Flags related actions
		_isVoided = true;
		_outputsChange = true;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

void VdblMPBttn::updFdaState(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
	switch(_mpbFdaState){
		case stOffNotVPP:
			//In: >>---------------------------------->>
			if(_sttChng){
				_turnOffVdd();	// _isVoided = false;

				stOffNotVPP_In();
				clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validPressPend){
				_mpbFdaState = stOffVPP;
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;	//The MPB has been disabled
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffVPP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(!_isOn){
				_turnOn();
			}
			_validPressPend = false;
			stOffVPP_Do();	// This provides a setting point for the voiding mechanism to be started
			_mpbFdaState = stOnNVRP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnNVRP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validVoidPend){
				_mpbFdaState = stOnVVP;
				setSttChng();
			}
			if(_validReleasePend){
				_mpbFdaState = stOnVRP;
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;	//The MPB has been disabled
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnVVP:
			if(_sttChng){
				_turnOnVdd();	// _isVoided = true;
				_validVoidPend = false;
				_outputsChange = true;
				clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_mpbFdaState = stOnVddNVUP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnVddNVUP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_turnOff();
			_mpbFdaState = stOffVddNVUP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffVddNVUP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			stOffVddNVUP_Do();
			if(_validUnvoidPend){
				_mpbFdaState = stOffVddVUP;
				setSttChng();
			}
			if(_validDisablePend){
				_mpbFdaState = stDisabled;	//The MPB has been disabled
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffVddVUP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_turnOffVdd(); //_isVoided = false;
			_validUnvoidPend = false;
			_outputsChange = true;
			_mpbFdaState = stOffUnVdd;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOffUnVdd:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_mpbFdaState = stOff;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnVRP:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_validReleasePend = false;
			_mpbFdaState = stOnTurnOff;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOnTurnOff:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_turnOff();
			_mpbFdaState = stOff;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stOff:
			//In: >>---------------------------------->>
			if(_sttChng){clrSttChng();}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			_mpbFdaState = stOffNotVPP;
			setSttChng();
			//Out: >>---------------------------------->>
			if(_sttChng){}	// Execute this code only ONCE, when exiting this state
			break;

		case stDisabled:
			//In: >>---------------------------------->>
			if(_sttChng){
				_validDisablePend = false;
				stDisabled_In();
				_isEnabled = false;
				setOutputsChange(true);
				clrSttChng();
			}	// Execute this code only ONCE, when entering this state
			//Do: >>---------------------------------->>
			if(_validEnablePend){
				_turnOff();
				_isEnabled = true;
				_validEnablePend = false;
				_outputsChange = true;
			}
			if(_isEnabled && !updIsPressed()){	//The stDisabled status will be kept until the MPB is released for security reasons
				_mpbFdaState = stOffNotVPP;
				setSttChng();
			}
			//Out: >>---------------------------------->>
			if(_sttChng){
				stDisabled_Out();
			}	// Execute this code only ONCE, when exiting this state
			break;

	default:
		break;
	}
	taskEXIT_CRITICAL(&mux);

	return;
}

//=========================================================================> Class methods delimiter

TmVdblMPBttn::TmVdblMPBttn(const uint8_t &mpbttnPin, unsigned long int voidTime, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay, const bool &isOnDisabled)
:VdblMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay, isOnDisabled), _voidTime{voidTime}
{
}

TmVdblMPBttn::~TmVdblMPBttn()
{
}

bool TmVdblMPBttn::begin(const unsigned long int &pollDelayMs){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if (!_mpbPollTmrHndl){
		_mpbPollTmrHndl = xTimerCreate(
			_mpbPollTmrName.c_str(),  //Timer name
			pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
			pdTRUE,     //Autoreload true
			this,       //TimerID: data passed to the callback funtion to work
			mpbPollCallback
		);
	}
   if (_mpbPollTmrHndl != NULL){
   	tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
		if (tmrModResult == pdPASS)
			result = true;
	}

   return result;
}

void TmVdblMPBttn::clrStatus(){
	portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;

	taskENTER_CRITICAL(&mux);
   _voidTmrStrt = 0;
   VdblMPBttn::clrStatus();
   taskEXIT_CRITICAL(&mux);

   return;
}

const unsigned long int TmVdblMPBttn::getVoidTime() const{

	return _voidTime;
}

bool TmVdblMPBttn::setVoidTime(const unsigned long int &newVoidTime){
    portMUX_TYPE mux portMUX_INITIALIZER_UNLOCKED;
	 bool result{true};

    taskENTER_CRITICAL(&mux);
    if(newVoidTime != _voidTime){
   	 if(newVoidTime >= _MinSrvcTime)
   		 _voidTime = newVoidTime;
   	 else
   		 result = false;
    }
    taskEXIT_CRITICAL(&mux);

    return result;
}

void TmVdblMPBttn::stOffNotVPP_In(){
	_voidTmrStrt = 0;

	return;
}

void TmVdblMPBttn::stOffVddNVUP_Do(){
	if(_validReleasePend){
		_validReleasePend = false;
		_validUnvoidPend = true;
	}
	return;
}

void TmVdblMPBttn::stOffVPP_Do(){	// This provides a setting point for the voiding mechanism to be started
   _voidTmrStrt = xTaskGetTickCount() / portTICK_RATE_MS;

	return;
}

bool TmVdblMPBttn::updIsPressed(){

    return DbncdDlydMPBttn::updIsPressed();
}

bool TmVdblMPBttn::updVoidStatus(){
   bool result {false};

   if(_voidTmrStrt != 0){
		if (((xTaskGetTickCount() / portTICK_RATE_MS) - _voidTmrStrt) >= (_voidTime)){ // + _dbncTimeTempSett + _strtDelay
			 result = true;
		}
	}
   _validVoidPend = result;

	return _validVoidPend;
}

//=========================================================================> Class methods delimiter

SnglSrvcVdblMPBttn::SnglSrvcVdblMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:VdblMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay, false)
{
	_isOnDisabled = false;
   _frcOtptLvlWhnVdd = true;	//This attribute is subclass inherent characteristic, no setter will be provided for it
   _stOnWhnOtptFrcd = false;	//This attribute is subclass inherent characteristic, no setter will be provided for it
}

SnglSrvcVdblMPBttn::~SnglSrvcVdblMPBttn()
{
}

bool SnglSrvcVdblMPBttn::begin(const unsigned long int &pollDelayMs){
   bool result {false};
   BaseType_t tmrModResult {pdFAIL};

   if (!_mpbPollTmrHndl){
		_mpbPollTmrHndl = xTimerCreate(
			_mpbPollTmrName.c_str(),  //Timer name
			pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
			pdTRUE,     //Autoreload true
			this,       //TimerID: data passed to the callback funtion to work
			mpbPollCallback
		);
	}
   if (_mpbPollTmrHndl != NULL){
   	tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
		if (tmrModResult == pdPASS)
			result = true;
	}

   return result;
}

void SnglSrvcVdblMPBttn::setTaskWhileOn(const TaskHandle_t &newTaskHandle){

	return;
}

void SnglSrvcVdblMPBttn::stOffVddNVUP_Do(){
	if(_validReleasePend){
		_validReleasePend = false;
		_validUnvoidPend = true;
	}

	return;
}

bool SnglSrvcVdblMPBttn::updVoidStatus(){
	bool result {false};

	if(_isOn)
		result = true;
	_validVoidPend = result;

	return _validVoidPend;
}

//=========================================================================> Class methods delimiter

/**
 * @brief Unpackages a 32-bit value into a DbncdMPBttn object status
 * 
 * The 32-bit encoded and packaged is used for inter-task object status comunication, passed as a "notification value" in a xTaskNotify() execution.
 * For each bit value attribute flag represented see DbncdMPBttn::getOtptsSttsPkgd()
 * 
 * @param pkgOtpts A 32-bit value holding a DbncdMPBttn status encoded
 * @return A MpbOtpts_t type element containing the information decoded
 */
MpbOtpts_t otptsSttsUnpkg(uint32_t pkgOtpts){
	MpbOtpts_t mpbCurSttsDcdd {0};

	if(pkgOtpts & (((uint32_t)1) << IsOnBitPos))
		mpbCurSttsDcdd.isOn = true;
	else
		mpbCurSttsDcdd.isOn = false;

	if(pkgOtpts & (((uint32_t)1) << IsEnabledBitPos))
		mpbCurSttsDcdd.isEnabled = true;
	else
		mpbCurSttsDcdd.isEnabled = false;

	// From here on the attribute flags are not present in every subclass!!
	if(pkgOtpts & (((uint32_t)1) << PilotOnBitPos))
		mpbCurSttsDcdd.pilotOn = true;
	else
		mpbCurSttsDcdd.pilotOn = false;

	if(pkgOtpts & (((uint32_t)1) << WrnngOnBitPos))
		mpbCurSttsDcdd.wrnngOn = true;
	else
		mpbCurSttsDcdd.wrnngOn = false;

	if(pkgOtpts & (((uint32_t)1) << IsVoidedBitPos))
		mpbCurSttsDcdd.isVoided = true;
	else
		mpbCurSttsDcdd.isVoided = false;

	if(pkgOtpts & (((uint32_t)1) << IsOnScndryBitPos))
		mpbCurSttsDcdd.isOnScndry = true;
	else
		mpbCurSttsDcdd.isOnScndry = false;

	mpbCurSttsDcdd.otptCurVal = (pkgOtpts & 0xffff0000) >> OtptCurValBitPos;

	return mpbCurSttsDcdd;
}
