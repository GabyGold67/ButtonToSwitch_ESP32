/**
  ******************************************************************************
  * @file	: PrssSgnlSrc_ESP32.cpp
  * @brief	: Source file for the ButtonToSwitch_ESP32 library's PressSignalSource interface implementation classes
  *
  * @details The classes included implement the PressSignalSource interface class,
  * allowing the ButtonToSwitch_ESP32 objects to accept different signals sources
  * to evaluate the _isPressed attribute flag.  
  * From the simplest project using the MCU GPIO pins as signal source, the
  * project development might need to implement different sources for the signal,
  * as MCU GPIO pins become used by other resources (GPIO expanders might be use as
  * an alternative input pins source), as the the signal source
  * might be too far for a MCU GPIO pin signal to get to the right value due
  * to voltage drop or interfearence (a cabled or radio signal technology input
  * solution might become mandatory), and others.
  *
  * Repository: https://github.com/GabyGold67/ButtonToSwitch_ESP32
  * 
  * Framework: Arduino  
  * Platform: ESP32  
  * 
  * @author Gabriel D. Goldman  
  * mail <gdgoldman67@hotmail.com>  
  * Github <https://github.com/GabyGold67>  
  * 
  * @version v5.0.0
  * 
  * @date First release: 06/11/2023  
  *       Last update:   16/08/2025 17:30 (GMT+0200) DST  
  * 
  * @copyright Copyright (c) 2025  GPL-3.0 license  
  *******************************************************************************
  * @attention	This library was originally developed as part of the refactoring
  * process for an industrial machines security enforcement and productivity control
  * (hardware & firmware update). As such every class included complies **AT LEAST**
  * with the provision of the attributes and methods to make the hardware & firmware
  * replacement transparent to the controlled machines. Generic use attributes and
  * methods were added to extend the usability to other projects and application
  * environments, but no fitness nor completeness of those are given but for the
  * intended refactoring project, and for the author's projects requirements.  
  * 
  * @warning **Use of this library is under your own responsibility**
  * 
  * @warning The use of this library falls in the category described by The Alan 
  * Parsons Project (c) 1980 Games People play disclaimer:   
  * Games people play, you take it or you leave it  
  * Things that they say aren't alright  
  * If I promised you the moon and the stars, would you believe it?  
 *******************************************************************************
 */
#include <./IntrfcsImplmntd/PrssSgnlSrc_esp32.h>

 //=========================================================================> Class methods delimiter

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

//=========================================================================> Class methods delimiter

