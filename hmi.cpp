#include <TimeLib.h>
#include "hmi.h"
#include "btcom.h"
#include "configuration.h"

#include <GxEPD.h>
#include <GxGDEH029A1/GxGDEH029A1.cpp>      // 2.9" b/w


// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

#if defined(ESP8266)

//GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io(SPI, SS, D3, D4); // arbitrary selection of D3, D4 selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
GxEPD_Class display(io); // default selection of D4, D2
// my IoT connection, busy on MISO
//GxEPD_Class display(io, D4, D6);

#elif defined(ESP32)

// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io(SPI, SS, 17, 16); // arbitrary selection of 17,16
// GxGDEP015OC1(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
GxEPD_Class display(io, 16, 4); // arbitrary selection of (16), 4

#else

GxIO_Class io(SPI, SS, 8, 9); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = 9, uint8_t busy = 7);
GxEPD_Class display(io);

#endif


#define DISPLAY_WIDTH GxEPD_HEIGHT
#define DISPLAY_HEIGHT GxEPD_WIDTH
#define DISPLAY_ROTATION 3
 
uint8_t partyModeTime = 0;
uint8_t partyModeValid = false;
SemaphoreHandle_t xDisplaySemaphore;

void entryScreen(){
  char* message = "";
  if (xSemaphoreTake(xDisplaySemaphore, (TickType_t) 5) == pdTRUE ){
    clear(0, R3_Y);
    printSoftKey("", "", "  >>>");
    if (!partyModeWritten()){
      message = "Verbinde...";
    }
    printMain(message);
    printTime();
    display.updateWindow(0, 0, DISPLAY_WIDTH-1, R3_Y, true);
    xSemaphoreGive(xDisplaySemaphore);
  }
}

void showTime(){
  if (xSemaphoreTake(xDisplaySemaphore, (TickType_t) 5) == pdTRUE ){
    clear(0, R1_Y);
    printTime();
    display.updateWindow(0, 0, DISPLAY_WIDTH-1, R1_Y, true);
    xSemaphoreGive(xDisplaySemaphore);
  }  
}

void printPartyModeTime(){
  char buffer[6];
  snprintf(buffer, 6, "%02d:00", partyModeTime);
  printMain (buffer);
}

void partyMode(){
  if (xSemaphoreTake(xDisplaySemaphore, (TickType_t) 5) == pdTRUE ){
    clear(0, R3_Y);
    printHeadline("Heizung bis:");
    printPartyModeTime();
    printSoftKey("    -", "    +", "  >>>");
    display.updateWindow(0, 0, DISPLAY_WIDTH-1, R3_Y, true);
    xSemaphoreGive(xDisplaySemaphore); 
  }   
}

void incPartyModeTime(){
  if (partyModeTime <= 5){
    partyModeTime++;
  }
  partyModeValid = true;  
  if (xSemaphoreTake(xDisplaySemaphore, (TickType_t) 5) == pdTRUE ){
    clear(R1_Y, R2_Y);
    printPartyModeTime();
    display.updateWindow(0, R1_Y, DISPLAY_WIDTH-1, R2_Y, true);
    xSemaphoreGive(xDisplaySemaphore); 
  }    
}

void decPartyModeTime(){
  if (partyModeTime > 0){
    partyModeTime--;
  }
  partyModeValid = true;
  if (xSemaphoreTake(xDisplaySemaphore, (TickType_t) 5) == pdTRUE ){
    clear(R1_Y, R2_Y);
    printPartyModeTime();
    display.updateWindow(0, R1_Y, DISPLAY_WIDTH-1, R2_Y, true);
    xSemaphoreGive(xDisplaySemaphore); // Now free or "Give" the Serial Port for others.
  }    
}

void writeValues(){
  if (partyModeValid){
    writePartyMode (partyModeTime, 0);
  }
  partyModeValid = false;
}

void absent(){
  if (xSemaphoreTake(xDisplaySemaphore, (TickType_t) 5) == pdTRUE ){
    clear(0, R3_Y);
    printHeadline("Anwesenheit:");
    printSoftKey(" Abw.", " Anw.", "  >>>");
    display.updateWindow(0, 0, DISPLAY_WIDTH-1, R3_Y, true);
    xSemaphoreGive(xDisplaySemaphore); 
  }    
}

void setPresent(){
  writePresence("home");
}

void setAbsent(){
  writePresence("absent");
}


void clear(uint8_t y1, uint8_t y2){
  display.fillRect(0, y1, DISPLAY_WIDTH-1, y2-y1, GxEPD_WHITE);
}

void printHeadline(char * text){
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(0, R1_Y-7);
  display.print (text);
}

void printTime(){
  char buffer[11];
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(0, R1_Y-7);
  snprintf(buffer, 11, "%02d.%02d.%02d", day(), month(), year());
  display.print(buffer); 
  display.setCursor(DISPLAY_WIDTH/2 + 45, R1_Y-7);
  snprintf(buffer, 6, "%02d:%02d", hour(), minute());
  display.print(buffer);
  Serial.println("Time printed");
}


void printSoftKey(char* s1, char* s2, char* s3){
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(0, R3_Y-8);
  display.print(s1);
  display.setCursor(DISPLAY_WIDTH/3+1, R3_Y-8);
  display.print(s2);
  display.setCursor(DISPLAY_WIDTH*2/3+1, R3_Y-8);
  display.print(s3);    
  Serial.println("Soft key printed");
}

void printMain(char* text){
  display.setFont(&FreeSansBold24pt7b);
  display.setCursor(0, DISPLAY_HEIGHT-50);
  display.print(text);  
  Serial.println("Main printed");
}

void displayInit (boolean first){
  display.init();
  if (first) {
    Serial.println("Init display");
    display.fillRect(0, 0, DISPLAY_HEIGHT, DISPLAY_WIDTH, GxEPD_WHITE);
    display.update();
    
  } 
  display.setRotation(DISPLAY_ROTATION);
  display.setTextColor(GxEPD_BLACK);
  if (xDisplaySemaphore == nullptr)  
  {
    xDisplaySemaphore = xSemaphoreCreateMutex();  
    if (xDisplaySemaphore != nullptr)
      xSemaphoreGive(xDisplaySemaphore);  
  }
}

void displayOff (){
  display.powerDown();
}

