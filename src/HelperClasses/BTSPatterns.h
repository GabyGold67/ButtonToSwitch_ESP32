/**
 * @file BTSPaiierns.h
 * @brief 
 */
#include <Arduino.h>

/*
   Strategy Pattern implementation of the updIsPressed() method of the DbncdMPBttn class
   Before this implementation the had a single source for detecting pushbuttons as being pressed:
   - The voltage level of an MCU GPIO pin configured as input, considered through the pushbutton characteristics and physical connection characteristics (NO/NC. Pulled-Up/Down)

   A strategy pattern implementation will be used to enable different sources to consider the pushbutton as being pressed, that would be (not limited to):
   - Public methods mpbPressed(), mpbReleased() that would update the _isPressed attribute.
   - GPIO expanders hardware signaling the state of the connected pin through the corresponding communications protocols and throug an adapter pattern, to be ultimately treated in an analog way to the original method.
*/

/*
Interface Strategy
*/
class PressSignalSource{   // Interface Strategy
public:
   PressSignalSource();
   virtual ~PressSignalSource();
   virtual bool updIsPressed() = 0;
};

/*
Concrete Strategy
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

/*
Concrete Strategy
*/
class MethodsSetters: public PressSignalSource{
private:
   bool _vIsPressed{false};

public:
   MethodsSetters();
   virtual ~MethodsSetters();
   void vPress();
   void vRelease();
   bool updIsPressed();
};
