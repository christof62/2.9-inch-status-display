#include <Arduino.h>
#include <TimeLib.h>
#include <time.h>
#include <sys/time.h>
#include <soc/rtc.h>
#include <esp_deep_sleep.h>
#include <Fsm.h>
#include "btcom.h"
#include "menu.h"
#include "button.h"
#include "configuration.h"
#include "hmi.h"


#define uS_TO_S_FACTOR 1000000    
RTC_DATA_ATTR int bootCount = 0;

uint32_t startTime;
uint32_t lastInteractionTime;
esp_sleep_wakeup_cause_t wakeupReason;
static CButton buttonL (BUTTON_L, &handleButtons);
static CButton buttonM (BUTTON_M, &handleButtons);
static CButton buttonR (BUTTON_R, &handleButtons);


void setup(){
  startTime = millis();
  lastInteractionTime = startTime;
  Serial.begin(115200);

  struct timeval tv;
  gettimeofday(&tv, NULL);
  setTime(tv.tv_sec);

  displayInit (bootCount == 0);
  menuInit();
  menuFsm.run_machine();  
  wakeupReason = esp_sleep_get_wakeup_cause();  
  if (wakeupReason == ESP_DEEP_SLEEP_WAKEUP_EXT0){
    menuFsm.trigger(BUTTON_WAKEUP);
    menuFsm.run_machine();
    BLEscan();
  } else if (bootCount == 0){
    menuFsm.trigger(FIRST_BOOT);
  }else {
    menuFsm.trigger(TIME_UPDATE);
  }
  ++bootCount;
  if (((bootCount % 30) == 0) || (year() < 2016)){
    BLEscan();
  } else if (wakeupReason != ESP_DEEP_SLEEP_WAKEUP_EXT0) {
    sleep();
  }    
}

void loop() {
  buttonL.debounce();
  buttonM.debounce();
  buttonR.debounce();  
  menuFsm.run_machine();
  vTaskDelay(5);
  if ((millis()-lastInteractionTime) > 20000){
    sleep();
  }
}

/*! \fn void sleep()
 *  \brief enter deep sleep until new minute starts or button pressed
 */
void sleep(){
    Serial.print("Going to sleep after ");
    Serial.println((uint32_t)(millis()-startTime));
    displayOff();
    gpio_pullup_en(GPIO_INPUT_IO_TRIGGER);    
    gpio_pulldown_dis(GPIO_INPUT_IO_TRIGGER);       
    esp_deep_sleep_enable_ext0_wakeup(GPIO_INPUT_IO_TRIGGER, 0); 
    uint8_t sleepTime = 60-second();
    esp_sleep_enable_timer_wakeup(sleepTime * uS_TO_S_FACTOR);
    esp_deep_sleep_start();  
}

/*! \fn void handleButtons(uint8_t pin, bool state)  
 *  \brief Handle button actions on pressed or released buttons
 *  \param pin   GPIO number of PIN connected to button
 *  \param state true if button pressed
 */
void handleButtons(uint8_t pin, bool state){
  if (state == 1){
    Serial.println (pin);
    if (pin == BUTTON_L){
      menuFsm.trigger(L_PRESSED);
    }
    if (pin == BUTTON_M){
      menuFsm.trigger(M_PRESSED);
    }
    if (pin == BUTTON_R){
      menuFsm.trigger(R_PRESSED);
    }
    lastInteractionTime = millis();      
  }
}

