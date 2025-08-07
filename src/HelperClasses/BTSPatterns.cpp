#include "BTSPatterns.h"

PressSignalSource::PressSignalSource()
{
}

PressSignalSource::~PressSignalSource()
{
}

//=========================================================================> Class methods delimiter

McuInputPin::McuInputPin(const int8_t &mcuPin, const bool &pulledUp, const bool &typeNO)
:_mcuPin{mcuPin}, _pulledUp{pulledUp}, _typeNO{typeNO}
{   
}

McuInputPin::~McuInputPin()
{   
}

bool McuInputPin::updIsPressed()
{

	/*To be 'pressed' the conditions are:
	1) For NO == true
		a)  _pulledUp == false ==> digitalRead == HIGH
		b)  _pulledUp == true ==> digitalRead == LOW
	2) For NO == false
		a)  _pulledUp == false ==> digitalRead == LOW
		b)  _pulledUp == true ==> digitalRead == HIGH
	*/
	bool result {false};
	bool tmpPinLvl {digitalRead(_mcuPin)};
    
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
	// _isPressed = result;
	// return _isPressed;

   return result;
}

//=========================================================================> Class methods delimiter

MethodsSetters::MethodsSetters()
{
}

MethodsSetters::~MethodsSetters()
{
}

void MethodsSetters::vPress()
{
	_vIsPressed = true;
	
	return;
}

void MethodsSetters::vRelease()
{
	_vIsPressed = false;

	return;
}

bool MethodsSetters::updIsPressed()
{
	
	return _vIsPressed;
}