#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <TimeLib.h>
#include <sys/time.h>
#include <Fsm.h>
#include <FreeRTOS.h>
#include "menu.h"

// The remote service.
static BLEUUID homeEnvServiceUUID("00000a00-0000-1000-8000-00805f9b34fb");
// The characteristics of the remote service.
static BLEUUID curTimeUUID("00002a2b-0000-1000-8000-00805f9b34fb");       // Provides current time according to GATT 
static BLEUUID partyModeUUID("0000d379-0000-1000-8000-00805f9b34fb");     // Party mode prolongs heating
static BLEUUID presenceUUID("0000d380-0000-1000-8000-00805f9b34fb");      // Home presence

static BLEAddress *pServerAddress;
static BLERemoteCharacteristic* pDateTimeCharacteristic;
static BLERemoteCharacteristic* pPartyModeCharacteristic;
static BLERemoteCharacteristic* pPresenceCharacteristic;
static BLEAdvertisedDevice bleDevice;
static TaskHandle_t tConnect = NULL;

// Party mode characteristic
#define PM_INVALID 99
RTC_DATA_ATTR uint8_t pmHour = PM_INVALID;
RTC_DATA_ATTR uint8_t pmMinute = 0;

/*! \fn void writePartyMode(uint8_t hour, uint8_t minute) 
 *  \brief Write the new time to end the party mode as BLE value 
 *  \param hour Hour when party mode should be ended
 *  \param minute Minute when party mode should be ended
 */
void writePartyMode(uint8_t hour, uint8_t minute){
  if (pPartyModeCharacteristic != NULL){
    char buffer[6];
    snprintf(buffer, 6, "%02d:%02d", hour, minute);
    pPartyModeCharacteristic->writeValue(buffer, true);
    pmHour = PM_INVALID;
    Serial.println ("PM-Value written");
  } else {
    pmHour = hour;
    pmMinute = minute;
  }
}

/*! \fn bool partyModeWritten() 
 *  \brief Status of BLE communication regarding successfully written party mode end time
 *  \return True if party mode ent time successfully written
 */
bool partyModeWritten(){
  return pmHour == PM_INVALID;
}

// Presence characteristic
void writePresence(char* presence){
  if (pPresenceCharacteristic != NULL){
    pPresenceCharacteristic->writeValue(presence, true);
    Serial.println ("Presence-Value written");
  } 
}


BLERemoteCharacteristic*  getCharacteristic(BLERemoteService* pService, BLEUUID uuid){
  BLERemoteCharacteristic* pCharacteristic = pService->getCharacteristic(curTimeUUID);
  if (pCharacteristic == nullptr) {
    Serial.print("Failed to find characteristic UUID: ");
    Serial.println(uuid.toString().c_str());
  }
  return pCharacteristic;
}
                                                              
bool connectToServer(BLEAddress pAddress) {
  Serial.print("Connecting to ");
  Serial.println(pAddress.toString().c_str());

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Client created ");

  // Connect to the remove BLE Server.
  if (pClient->connect(pAddress)){
    Serial.println(" - Connected to server: ");
  } else {
    Serial.println(" - Failed to connect to server");
  }

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(homeEnvServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.println(" - Failed to find service UUID: ");
    return false;
  }
  Serial.println(" - Found service");

  pDateTimeCharacteristic = getCharacteristic(pRemoteService, curTimeUUID);
  if (pDateTimeCharacteristic != nullptr){
    std::string value = pDateTimeCharacteristic->readValue();
    uint16_t yr = (uint8_t)value[0] + 256*(uint8_t)value[1];
    uint8_t mth = (uint8_t)value[2];
    uint8_t d = (uint8_t)value[3];
    uint8_t h = (uint8_t)value[4];
    uint8_t m = (uint8_t)value[5];
    uint8_t s = (uint8_t)value[6];
    setTime(h, m, s, d, mth, yr);
    // use ESP 32 RTC which continues during deep sleep
    struct timeval tv; 
    tv.tv_sec = now();
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);
    menuFsm.trigger(TIME_UPDATE);
  }

  pPartyModeCharacteristic = getCharacteristic(pRemoteService, partyModeUUID);
  if (pPartyModeCharacteristic != nullptr) {
    if (pmHour <= 24){ // value was set
      writePartyMode(pmHour, pmMinute);
    }
  }
  
  pPresenceCharacteristic = getCharacteristic(pRemoteService, presenceUUID);

  return true;
}

void connect(void * parameter){
  if (connectToServer(*pServerAddress)) {
    Serial.println("Connected to BLE Server.");
    menuFsm.trigger(SERVER_SUCCEED);
  } else {
    Serial.println("Failed to connect to the server.");
    menuFsm.trigger(SERVER_FAILED);
  }
  vTaskDelete( NULL );
}

/***
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
   // Called for each advertising BLE server.
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(homeEnvServiceUUID)) {
      Serial.println("Found device!"); 
      advertisedDevice.getScan()->stop();
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      xTaskCreate(connect, "connect", 4096, NULL, 0, &tConnect);
    } // Found server
  }   // onResult
};    // MyAdvertisedDeviceCallbacks


void BLEscan () {
  BLEDevice::init(""); 
  Serial.println("Enter scan"); 
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false);
  pBLEScan->start(5);
}



