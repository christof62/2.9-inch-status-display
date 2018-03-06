#include "button.h"

CButton::CButton (uint8_t pin, void (*stateChangeCB)(uint8_t, bool)){
  _state = false;
  _pin = pin;
  _lastTick = millis();
  pinMode(pin, INPUT_PULLUP);
  _stateChangeCB = stateChangeCB; 
}

CButton::~CButton(){

}

void CButton::debounce (void){
  bool currentState = !digitalRead(_pin);
  if (currentState == true){
    if (_state == false){
      if (_stateChangeCB != NULL){
        _stateChangeCB(_pin, true);
      } 
      _state = true;
    }
    _lastTick = millis();
  } else {
    if ((millis()-_lastTick) > 30){
      if (_state == true) {
        if (_stateChangeCB != NULL){
          _stateChangeCB(_pin, false);
        } 
        _state = false;
      }
    }
  }
}

void CButton::setStateChangeCB (void (*stateChangeCB)(uint8_t, bool)){
  _stateChangeCB = stateChangeCB;  
}
