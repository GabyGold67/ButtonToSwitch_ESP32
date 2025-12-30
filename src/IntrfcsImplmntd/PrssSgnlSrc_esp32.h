/**
  ******************************************************************************
  * @file	: PrssSgnlSrc_ESP32.h
  * @brief	: Header file for the ButtonToSwitch_ESP32 library's PressSignalSource interface implementation classes
  *
  * @details The classes included implement the PressSignalSource interface class,
  * allowing the ButtonToSwitch_ESP32 objects to accept different signals sources
  * to evaluate the _isPressed attribute flag.  
  * From the simplest project using the MCU GPIO pins as signal source, the
  * project development might need to implement different sources for the signal,
  * as MCU GPIO pins become used by other resources: (GPIO expanders might be used as
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
//FFDR For Future Development Reminder!!
//FTPO For Testing Purposes Only code!!

#ifndef _PRSSSGNLSRC_ESP32_
#define _PRSSSGNLSRC_ESP32_

#include <Arduino.h>
#include <stdint.h>

/*
   Strategy Pattern implementation of the updIsPressed() method of the DbncdMPBttn class
   Before this implementation the had a single source for detecting pushbuttons as being pressed:
   - The voltage level of an MCU GPIO pin configured as input, considered through the pushbutton characteristics and physical connection characteristics (NO/NC. Pulled-Up/Down)

   A strategy pattern implementation will be used to enable different sources to consider the pushbutton as being pressed, that would be (not limited to):
   - Public methods mpbPressed(), mpbReleased() that would update the _isPressed attribute.
   - GPIO expanders hardware signaling the state of the connected pin through the corresponding communications protocols and throug an adapter pattern, to be ultimately treated in an analog way to the original method.
*/

/**
 * @brief Strategy Pattern interface class
 * 
 * @details This interface defines the methods to be implemented by any concrete strategy
 * to provide the signal source for the DbncdMPBttn class objects to evaluate the _isPressed attribute.
 * 
 * @class PressSignalSource
 */
class PressSignalSource{   // Interface Strategy
public:
   PressSignalSource();
   virtual ~PressSignalSource();
   virtual bool updIsPressed() = 0;
};

/**
 * @brief Concrete Strategy class for MCU GPIO pin input as signal source
 * 
 * @details This class implements the PressSignalSource interface to provide the signal source for
 * the DbncdMPBttn class objects using an MCU GPIO pin as the input signal source.
 * 
 * @class McuInputPin
 */
class McuInputPin: public PressSignalSource{ // Concrete Strategy
protected:
   int8_t _mcuPin{};
   bool _pulledUp{};
   bool _typeNO{};

public:
   McuInputPin(const int8_t &mcuPin, const bool &pulledUp = true, const bool &typeNO = true);
   virtual ~McuInputPin();
   bool updIsPressed();
};

/**
 * @brief Concrete Strategy class for public method "virtual pressing" of a MPButton
 * 
 * @details This class implements the PressSignalSource interface to provide the signal source for
 * the DbncdMPBttn class objects using methods to generate the press and release events.
 * This strategy is useful as a generic path to generate the needed signals when no physical
 * connection is present:
 * - Wireless implementations.
 * - Wired interfaces connections (SPI, UART, etc)
 * 
 * @class McuInputPin
 */
class MethodInputPin: public PressSignalSource{
private:
   bool _isVrtlPressed{false};

public:
   MethodInputPin();
   virtual ~MethodInputPin();

   void vPress(const bool &newVal = true);
   void vRelease();
   bool updIsPressed();
};


#endif   // _PRSSSGNLSRC_ESP32_