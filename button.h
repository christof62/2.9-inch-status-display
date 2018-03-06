#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "Arduino.h"

class CButton
{
public:
  CButton (uint8_t pin, void (*stateChangeCB)(uint8_t, bool));
  void debounce(void);  
  void setStateChangeCB (void (*function)(uint8_t pin, bool state));
  ~CButton();

protected:

private:
  uint8_t _pin;
  uint32_t _lastTick;
  bool _state;
  void (*_stateChangeCB)(uint8_t, bool) = NULL;
};

#endif
